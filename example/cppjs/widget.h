#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "simple_handler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_ExecuteJavaScript_clicked();

private:
    Ui::Widget *ui;

    CefRefPtr<SimpleHandler> simple_handler_;
};
#endif // WIDGET_H
