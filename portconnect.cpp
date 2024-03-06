#include "portconnect.h"
#include "ui_portconnect.h"

portConnect::portConnect(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::portConnect)
{

    ui->setupUi(this);
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    foreach(const QSerialPortInfo &serialPortInfo, serialPorts)
    {
        ui->comPortName->addItem(serialPortInfo.portName());
    }

    connect(myPort,&QSerialPort::readyRead,this, [=]()
    {
        reseiveMessage = reseiveMessage+myPort->readLine();
        if(reseiveMessage.length()>11)
        {
            QString message = reseiveMessage.mid(0,9).toHex();


            if(message.left(4)!="0103")
            {
                reseiveMessage.clear();
            }
            else
            {
                float temp = decode(reseiveMessage.mid(3,4).toHex());
                currentDecodeData = temp;
                reseiveMessage.remove(0,9);

            }



        }
    });

    connect(myTimer,&QTimer::timeout,this, [=]()
    {
        getTemperature();
    });
    connect(myLightTimer,&QTimer::timeout,this, [=]()
    {
        openLight(true);
    });
    connect(tempTimer,&QTimer::timeout,this, [=]()
    {
        temp++;
        if(temp<4) ui->emissrate->setValue(currentDecodeData);
        else tempTimer->stop();
    });


}

portConnect::~portConnect()
{
    delete ui;
}
void portConnect::delay(int delayTime)
{
    QEventLoop loop;
    QTimer::singleShot(delayTime,&loop,SLOT(quit()));
    loop.exec();
}

bool portConnect::connectPort(QString portName,int baundrate,bool open)//连接
{
    myPort->setPortName(portName);
    myPort->setBaudRate(baundrate);
    myPort->setDataBits(QSerialPort::Data8);
    myPort->setParity(QSerialPort::EvenParity);

    if(open)
    {
        return myPort->open(QIODevice::ReadWrite);
    }
    else {
        myPort->close();
        return false;
    }

}

void portConnect::getTemperature()//获取温度
{
    if(myPort->isOpen())
    {
        QString getTemp = "01 03 04 00 00 02";
        QByteArray builtData = buildData(getTemp);
        myPort->write(builtData);

    }
}

void portConnect::getEmissvity()//获取发射率
{
    if(myPort->isOpen())
    {
        QString getEmiss = "01 03 04 02 00 02";
        QByteArray builtData = buildData(getEmiss);
        myPort->write(builtData);

    }
}

void portConnect::openLight(bool open)//开关激光
{
    if(myPort->isOpen())
    {
        if(open)
        {
            QString openL = "01 10 04 38 00 01 02 00 01";
            QByteArray builtData = buildData(openL);
            myPort->write(builtData);
        }
        else
        {
            QString openL = "01 10 04 38 00 01 02 00 00";
            QByteArray builtData = buildData(openL);
            myPort->write(builtData);
        }

    }
}

void portConnect::setEmisvity(float emissvity)//写发射率
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

void portConnect::autoChangeEissvity()//自动修改发射率
{

}

QByteArray portConnect::buildData(QString data)
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

QByteArray portConnect::CRC16(QByteArray data)
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

float portConnect::decode(QString data)
{
    QString str=data.mid(4,4)+data.mid(0,4);
    int hex = str.toUInt(0, 16);
    float value = *(float*)&hex;

    return value;
}

QString portConnect::encode(float data)
{
    float f = data;
    uint f_uint = *(uint*)&f;
    QString f_hex = QString("%1").arg(f_uint, 4, 16, QLatin1Char('0'));	// 4是生成字符串的最小长度，可以改为8
    qDebug()<<f_hex;
    return f_hex;
}

void portConnect::on_connect_clicked()
{
    QString portName = ui->comPortName->currentText();
    int baundrate = ui->baundrate->currentText().toInt();

    if(connectPort(portName,baundrate,true))
    {
        ui->connect->setText("断开");
        delay(50);
        getEmissvity();
        tempTimer->start(100);
        delay(50);
        // ui->emissrate->setValue(currentDecodeData);
        openLight(true);
        myLightTimer->start(60000);
        delay(50);

    }
    else
    {
        ui->connect->setText("连接");
        myPort->close();
    }

}


void portConnect::on_save_clicked()
{
    myTimer->stop();
    delay(50);
    reseiveMessage.clear();

    QString portName = ui->comPortName->currentText();
    int baundrate = ui->baundrate->currentText().toInt();
    float emissvity = ui->emissrate->value();

    setEmisvity(emissvity);

    delay(50);

    myTimer->start(100);

    this->hide();
    emit saved(portName, baundrate);

}

