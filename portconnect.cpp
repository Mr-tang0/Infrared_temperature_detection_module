#include "portconnect.h"
#include "ui_portconnect.h"
#include "mainwindow.h"

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
}

portConnect::~portConnect()
{
    delete ui;
}

void portConnect::on_connect_clicked()
{
    QString portName = ui->comPortName->currentText();
    int buadRate = ui->baundrate->currentText().toInt();
    float emissvity = ui->emissrate->value();
    emit saveed(portName,buadRate,emissvity);
}

