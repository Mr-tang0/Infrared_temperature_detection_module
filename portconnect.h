#ifndef PORTCONNECT_H
#define PORTCONNECT_H

#include <QWidget>

#include<QtSerialPort/QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include<QDebug>
#include <QTimer>
#include <QEvent>

namespace Ui {
class portConnect;
}

class portConnect : public QWidget
{
    Q_OBJECT

public:
    explicit portConnect(QWidget *parent = nullptr);
    ~portConnect();

    bool connectPort(QString portName,int baundrate,bool open);

    void getTemperature();
    void getEmissvity();
    void setEmisvity(float emissvity);
    void openLight(bool open);
    void autoChangeEissvity();

    QSerialPort *myPort = new QSerialPort;

    float currentDecodeData = 0;
    QByteArray reseiveMessage ="";
    QTimer *myTimer = new QTimer;
    QTimer *myLightTimer = new QTimer;
    QTimer *tempTimer = new QTimer;
    int temp= 0;

private slots:
    void on_connect_clicked();
    void on_save_clicked();

private:
    Ui::portConnect *ui;
    void delay(int delayTime);
    QByteArray buildData(QString data);
    QByteArray CRC16(QByteArray data);

    float decode(QString data);
    QString encode(float data);

signals:

    void saved(QString portName, int baundrate);
};

#endif // PORTCONNECT_H
