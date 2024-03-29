#include "portconnect.h"
#include "ui_portconnect.h"

portConnect::portConnect(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::portConnect)
{

    ui->setupUi(this);
    connect(this,&portConnect::thisShow,this,[=](){
        on_connect_clicked();
        delay(100);
        on_connect_clicked();

    });

    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    foreach(const QSerialPortInfo &serialPortInfo, serialPorts)
    {
        ui->comPortName->addItem(serialPortInfo.portName());
    }
    on_connect_clicked();
    delay(500);
    myTimer->start(500);


    connect(ui->comboBox,&QComboBox::currentTextChanged,[=](){
        int text = ui->comboBox->currentIndex();
        qDebug()<<text;
        switch (text) {
        case 0:
            rate=0.15;
            break;
        case 1:
            rate=0.45;
             break;
        case 2:
            rate=0.2;
            break;
        case 3:
            rate=0.3;
            break;
        case 4:
            rate=0.15;
            break;
        case 5:
            rate=0.2;
            break;
        case 6:
            rate=0.75;
            break;
        case 7:
            rate=0.15;
            break;
        case 8:
            rate=0.2;
            break;

        default:
            rate=0.45;
            break;
        }

    });

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
        if(temp<4) rate=currentDecodeData;
        else
        {
            tempTimer->stop();
            temp=0;
            qDebug()<<temp;
        }
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
    {{}
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
    qDebug()<<emissvity;

    QString emissvityStr = encode(emissvity);
    QString getTemp = "01 10 04 02 00 02 04 "+emissvityStr.mid(4,2)+" "+emissvityStr.mid(6,2)+" "+emissvityStr.mid(0,2)+" "+emissvityStr.mid(2,2);
    QByteArray builtData =buildData(getTemp);
    qDebug()<<builtData.toHex();

    if(myPort->isOpen())
    {
        myPort->write(builtData);
    }
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
    myTimer->stop();
    myLightTimer->stop();
    currentDecodeData = 0;
    reseiveMessage.clear();

    portName = ui->comPortName->currentText();
    baundrate = ui->baundrate->currentText().toInt();

    if(connectPort(portName,baundrate,true))
    {
        ui->connect->setText("断开");
        delay(50);
        getEmissvity();
        tempTimer->start(50);
        delay(120);

        openLight(true);
        myTimer->start(500);
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

    portName = ui->comPortName->currentText();
    baundrate = ui->baundrate->currentText().toInt();
    qDebug()<<rate;
    float emissvity = rate;

    setEmisvity(emissvity);

    delay(50);

    this->hide();
    myTimer->start(500);

    emit saved(portName, baundrate);

}

