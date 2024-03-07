#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "portconnect.h"
#include<QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("红外温度检测模块");

    connect(port,&portConnect::saved,this,[=](QString portName, int baundrate){
        ui->port->setText(portName+": "+QString::number(baundrate));
    });
    connect(uiFreshTimer,&QTimer::timeout,this,[=](){
        ui->port->setText(port->portName+": "+QString::number(port->baundrate));
        tempretrue = port->currentDecodeData;

        if(tempretrue>650 && tempretrue<2700)
        {
            ui->label_temperature->setText(QString::number(tempretrue));
            // setEmissvityCount++;
            // qDebug()<<setEmissvityCount;
            // if(setEmissvityCount>20)
            // {
            //     autoChangeEissvity();
            //     setEmissvityCount = 0;
            // }

            historyMax = qMax(historyMax,tempretrue);
        }
        else
        {
            ui->label_temperature->setText("Nan");
        }
        ui->label_maxTempreture->setText(QString::number(historyMax)+" ℃");
    });
    uiFreshTimer->start(250);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resize()
{

    int newFontSize = std::min(this->size().width(), this->size().height()); // 这里假设窗口每宽高100像素增大1个字体大小单位
    qDebug()<<newFontSize;

    ui->label_temperature->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/2));
    ui->label_maxTempreture->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_2->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_3->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->label_4->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/10));
    ui->port->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/20));
    ui->radioButton->setStyleSheet(QStringLiteral("font: %1px \"黑体\"").arg(newFontSize/30));

}

void MainWindow::on_clearButton_clicked()
{
    historyMax = 0;
}


void MainWindow::on_actionemissvity_triggered()
{
    port->show();
}


void MainWindow::autoChangeEissvity()//自动修改发射率
{
    if(tempretrue<700)return;
    else
    {
        float tempEissvity = 0;
        tempEissvity = tempretrue/9000.0;
        tempEissvity = QString::number(tempEissvity,'f',2).toFloat();
        qDebug()<<"tempEissvity"<<tempretrue<<tempEissvity;
        port->setEmisvity(tempEissvity);
    }
}



void MainWindow::on_radioButton_toggled(bool checked)
{
    port->openLight(checked);
}

