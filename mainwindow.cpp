#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGlobal>
#include <QDataStream>
#include <QDebug>
#include <QtCore/qendian.h>
#include <qmath.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_font = QFont("Arial", 10);
    ui->label_temperature->setFont(m_font);

    connect(portset,&portConnect::saveed,this,[=](QString portName,int baundrate,float emissvity){
        if(!myPort->isOpen())
        {
            connectPort(portName,baundrate,true);
            portset->hide();
        }
        if(myPort->isOpen())
        {

            setEmisvity(emissvity);
            timer->start(500);
            portset->hide();
        }

    });
    connect(timer,&QTimer::timeout,this,[=](){
        getTemperature();
    });
    connect(myPort,&QSerialPort::readyRead,this, [=](){
        reseiveMessage = reseiveMessage+myPort->readLine();
        if(reseiveMessage.length()>11)
        {
            QString message = reseiveMessage.mid(0,9).toHex();
            qDebug()<<message;
            if(message.left(4)!="0103")
            {
                reseiveMessage.clear();
            }
            else
            {
                int temp = decode(reseiveMessage.mid(3,4).toHex());
                ui->label_temperature ->setText(QString::number(temp));

                history.append(temp);
                int max = history.first();
                foreach (int number, history) {
                    max = qMax(max,number);
                }
                ui->label_maxTempreture->setText(QString::number(max)+" ℃");
                reseiveMessage.remove(0,9);
            }


        }
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectPort(QString portName,int baundrate,bool open)
{
    myPort->setPortName(portName);
    myPort->setBaudRate(baundrate);
    myPort->setDataBits(QSerialPort::Data8);
    myPort->setParity(QSerialPort::EvenParity);

    if(open)
    {
        bool openFlag= myPort->open(QIODevice::ReadWrite);
        if(openFlag)
        {
            ui->port->setText(portName + ": " + QString::number(baundrate));
        }
    }
    else {
        myPort->close();

    }

}

void MainWindow::getTemperature()
{
    if(myPort->isOpen())
    {
        QString getTemp = "01 03 04 00 00 02";
        QByteArray builtData = buildData(getTemp);
        myPort->write(builtData);

    }
}

QByteArray MainWindow::buildData(QString data)
{
    QByteArray builtData;

    bool ok = false;
    for (const auto &chunk : data.split(" "))
    {
        uchar byteValue = uchar(chunk.toUShort(&ok, 16));
        if (!ok)
        {
            builtData.clear();
            continue;
        }
        else
        {
            builtData.append(char(byteValue));
        }
    }
    builtData = builtData+CRC16(builtData);

    return builtData;
}

QByteArray MainWindow::CRC16(QByteArray data)
{
    //x16+x15+x2+1
    static const uint16_t crc16ModbusPoly = 0xa001;
    quint16 crc = 0xFFFF;

    for (int i = 0; i < data.size(); ++i)
    {
        char byte = data[i];

        for (int j = 0; j < 8; ++j)
        {
            bool bit = byte & 0x01;
            byte >>= 1;
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= crc16ModbusPoly;
            } else {
                crc >>= 1;
            }
            if (bit) {
                crc ^= crc16ModbusPoly;
            }
        }
    }
    QString temp =QString::number(crc,16);
    temp =temp.right(2)+" "+temp.left(temp.length()-2);

    QByteArray dataChange;

    bool ok;
    for (const auto &chunk : temp.split(" "))
    {
        uchar byteValue = uchar(chunk.toUShort(&ok, 16));
        if (!ok)
        {
            // 转换失败后的操作，待定
            qDebug() << "weigh转换失败,数据存在错误" ;
            return "";
        }
        else
        {
            dataChange.append(char(byteValue));
        }
    }

    return dataChange;//校验值，高位在后
}


void MainWindow::setEmisvity(float emissvity)
{
    QString emissvityStr = encode(emissvity);
    QString getTemp = "01 10 04 02 00 02 04 "+emissvityStr.mid(4,2)+" "+emissvityStr.mid(6,2)+" "+emissvityStr.mid(0,2)+" "+emissvityStr.mid(2,2);
    QByteArray builtData =buildData(getTemp);
    qDebug()<<builtData.toHex();

    if(myPort->isOpen())
    {
        myPort->write(builtData);
    }
}

void MainWindow::on_actionemissvity_triggered()
{
    portset->show();
}

float MainWindow::decode(QString data)
{
    QString str=data.mid(4,4)+data.mid(0,4);
    int hex = str.toUInt(0, 16);
    float value = *(float*)&hex;
    qDebug()<<value;
    return value;
}
QString MainWindow::encode(float data)
{
    float f = data;
    uint f_uint = *(uint*)&f;
    QString f_hex = QString("%1").arg(f_uint, 4, 16, QLatin1Char('0'));	// 4是生成字符串的最小长度，可以改为8
    qDebug()<<f_hex;
    return f_hex;
}

void MainWindow::resize()
{
    QFont m_font;
    int newFontSize = std::min(this->size().width(), this->size().height()); // 这里假设窗口每宽高100像素增大1个字体大小单位
    qDebug()<<newFontSize;

    ui->label_temperature->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/2));
    ui->label_maxTempreture->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_2->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_3->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_4->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/10));
    ui->port->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));

}

void MainWindow::on_clearButton_clicked()
{
    history.clear();
}

