#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "portconnect.h"
#include<QDebug>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    portConnect *port = new portConnect;

    void autoChangeEissvity();
    QTimer *uiFreshTimer = new QTimer;
    int historyMax = 0;
    int tempretrue;
    int setEmissvityCount = 0;

private slots:
    void on_clearButton_clicked();

    void on_actionemissvity_triggered();


    void on_radioButton_toggled(bool checked);

private:
    Ui::MainWindow *ui;


protected:
    void resizeEvent(QResizeEvent *event) override {

        QWidget::resizeEvent(event);

        resize();
        // 在这里执行你自己的处理逻辑
    }
    void resize();

};
#endif // MAINWINDOW_H
