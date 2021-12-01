// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app_render.h"

#include <string>
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

SimpleAppRender::SimpleAppRender()
{
}

void SimpleAppRender::OnWebKitInitialized()
{
    CEF_REQUIRE_RENDERER_THREAD();
    // 扩展类似于窗口绑定，除了它们被加载到每个框架的上下文中并且一旦加载就不能修改。
    // 加载扩展时 DOM 不存在，在扩展加载期间尝试访问 DOM 将导致崩溃。
    // 扩展使用 CefRegisterExtension() 函数注册，该函数应该从 CefRenderProcessHandler::OnWebKitInitialized() 方法调用
    // 表示的字符串extensionCode可以是任何有效的 JS 代码。然后框架中的 JS 可以与扩展代码进行交互。

    // Define the extension contents.
      std::string extensionCode =
        "var ext;"
        "if (!ext)"
        "  ext = {};"
        "(function() {"
        "  ext.cppval = 'cpp register ext';"
        "})();";

      // Register the extension.
      CefRegisterExtension("v8/ext", extensionCode, nullptr);
}

void SimpleAppRender::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefV8Context> context) {
    CEF_REQUIRE_RENDERER_THREAD();
    // 渲染进程
    // 窗口绑定允许客户端应用程序将值附加到frame的window对象
    // 每次重新加载框架时都会重新加载窗口绑定，从而为客户端应用程序提供必要时更改绑定的机会。例如，不同的框架可以通过修改绑定到该框架的窗口对象的值来访问客户端应用程序中的不同功能
    // 获取context的window object.
    CefRefPtr<CefV8Value> object = context->GetGlobal();

    // 创建一个V8 string
    CefRefPtr<CefV8Value> str = CefV8Value::CreateString("cpp Value");

    // 将V8 string注册到window object上
    object->SetValue("cppval", str, V8_PROPERTY_ATTRIBUTE_NONE);

}
