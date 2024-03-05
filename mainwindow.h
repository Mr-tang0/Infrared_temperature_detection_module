#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QtSerialPort/QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include<QDebug>
#include <QTimer>
#include "portconnect.h"
#include <QEvent>

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

    void connectPort(QString portName,int baundrate,bool open);

    void getTemperature();

    void setEmisvity(float emissvity);
    QByteArray buildData(QString data);
    QByteArray CRC16(QByteArray data);

    QByteArray reseiveMessage;

private slots:
    void on_actionemissvity_triggered();

    void on_clearButton_clicked();

private:
    Ui::MainWindow *ui;
    float decode(QString data);
    QString encode(float data);
    QSerialPort *myPort = new QSerialPort;
    QTimer *timer = new QTimer;
    portConnect *portset = new portConnect;

    QList<int> history;
    QFont m_font;

protected:
    void resizeEvent(QResizeEvent *event) override {
        // 当Widget大小发生改变时，这个函数会被调用
        QWidget::resizeEvent(event);

        resize();
        // 在这里执行你自己的处理逻辑
    }
    void resize();

};
#endif // MAINWINDOW_H
