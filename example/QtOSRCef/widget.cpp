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
    cef_wnd_info.SetAsChild((CefWindowHandle)this->winId(), win_rect);

    simple_handler_ = CefRefPtr<SimpleHandler>(new SimpleHandler());
    QString str_url = "http://www.baidu.com";
    CefBrowserSettings cef_browser_settings;
    CefBrowserHost::CreateBrowser(cef_wnd_info,
        simple_handler_,
        str_url.toStdString(),
        cef_browser_settings,
        nullptr,
        CefRequestContext::GetGlobalContext());
}

Widget::~Widget()
{
    simple_handler_->CloseAllBrowsers(true);
    delete ui;
}

