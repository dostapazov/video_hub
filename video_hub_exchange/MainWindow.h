#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTcpSocket>
#include <QTimer>
#include <QTranslator>

namespace Ui {
class MainWindow;
}

const char startByte = 0x5A;    // первый байт ответа

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QByteArray getAnswer();

    // получить ответ на запрос
    virtual bool readAnswer();

protected:
    void changeEvent(QEvent *e);
    void readAnswerData(QByteArray answer);
    void readAnswerTime(QByteArray answer);

private slots:
    bool tcpConnect(QString host, ushort port);
    void send();
    void read();
    void sendOnChange(bool checked);
    void slotError          (QAbstractSocket::SocketError);
    void connect_established();
    void connect_lost       ();

    void on_buttonConnect_clicked();
    void on_buttonTime_clicked();
    void on_sbInrval_valueChanged(int arg1);

    void on_actionEN_triggered();
    void on_actionRU_triggered();

private:
    Ui::MainWindow *ui = nullptr;
    QTranslator qtLanguageTranslator;
    QTcpSocket tcpSocket;
    uint16_t tx_number; // номер пакета
    uint16_t rx_number; // номер пакета
    QTimer timer;
    QByteArray buffer;
};

#endif // MAINWINDOW_H
