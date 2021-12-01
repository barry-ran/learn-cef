#include <qapplication.h>

#include "widget.h"
#include "./ui_widget.h"

#include "include/cef_request_context.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 以下是将 SimpleHandler 与窗体进行关联的代码
    QRect rect = this->geometry();
    CefRect win_rect(rect.left(), rect.top(), rect.right() - rect.left(), rect.bottom() - rect.top());
    CefWindowInfo cef_wnd_info;
    cef_wnd_info.SetAsChild((CefWindowHandle)ui->webWidget->winId(), win_rect);

    QString local_html = "file://" + QApplication::applicationDirPath() + "/test.html";
    simple_handler_ = CefRefPtr<SimpleHandler>(new SimpleHandler(false));
    CefBrowserSettings cef_browser_settings;
    CefBrowserHost::CreateBrowser(cef_wnd_info,
        simple_handler_,
        local_html.toStdString().c_str(),
        cef_browser_settings,
        nullptr,
        CefRequestContext::GetGlobalContext());

}

Widget::~Widget()
{
    simple_handler_->CloseAllBrowsers(true);
    delete ui;
}


void Widget::on_ExecuteJavaScript_clicked()
{
    simple_handler_->ExecuteJavaScript("alert('Hello, I come from cpp')");
}

