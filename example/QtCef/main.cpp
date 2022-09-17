#include <QApplication>
#include <QTimer>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_OSX
#include "include/wrapper/cef_library_loader.h"
#endif
#include "include/cef_command_line.h"

#include "widget.h"
#include "simple_app.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#ifdef Q_OS_OSX
    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInMain()) {
        return 1;
    }

    // Provide CEF with command-line arguments.
    CefMainArgs main_args(argc, argv);

    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::CreateCommandLine();
    command_line->InitFromArgv(argc, argv);
#endif

#ifdef Q_OS_WIN
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    // Provide CEF with command-line arguments.
    const HINSTANCE hInstance = static_cast<HINSTANCE>(::GetModuleHandle(nullptr));
    CefMainArgs main_args(hInstance);

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
    if (exit_code >= 0) {
        // The sub-process has completed so return here.
        return exit_code;
    }

    // Parse command-line arguments for use in this method.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());
#endif

    // Specify CEF global settings here.
    CefSettings settings;

    // When generating projects with CMake the CEF_USE_SANDBOX value will be
    // defined automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line
    // to disable use of the sandbox.
    settings.no_sandbox = true;
    //settings.log_severity = LOGSEVERITY_DISABLE;

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Initialize CEF for the browser process.
    CefInitialize(main_args, settings, app.get(), nullptr);

    QApplication a(argc, argv);

    // 使用Qt的消息循环代替了cef的CefRunMessageLoop，所以需要定时处理cef的消息循环
    // cef提供了异步消息循环接口CefDoMessageLoopWork
    QTimer cef_loop(&a);
    QObject::connect(&cef_loop, &QTimer::timeout, [=]() {
        CefDoMessageLoopWork();
    });
    cef_loop.start(33);

    Widget* w = new Widget;
    w->show();
    auto ret = a.exec();
    delete w;

    // 放开这里会由于浏览器没有完全清理导致出发dcheckassert，
    // 看下面链接说是CefDoMessageLoopWork的问题，后续再优化
    // https://magpcss.org/ceforum/viewtopic.php?f=6&t=17064
    //CefShutdown();

    return ret;

}
