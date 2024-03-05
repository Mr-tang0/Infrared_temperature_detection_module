#ifndef PORTCONNECT_H
#define PORTCONNECT_H

#include <QWidget>

namespace Ui {
class portConnect;
}

class portConnect : public QWidget
{
    Q_OBJECT

public:
    explicit portConnect(QWidget *parent = nullptr);
    ~portConnect();

private slots:
    void on_connect_clicked();

private:
    Ui::portConnect *ui;

signals:
    void saveed(QString portName, int buadrate, float emissvity);
};

#endif // PORTCONNECT_H
