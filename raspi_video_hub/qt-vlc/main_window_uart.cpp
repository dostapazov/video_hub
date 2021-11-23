#include <mainwindow.h>
#include <applog.h>
#include <appconfig.h>
#include <QtCore>
#include <QSerialPortInfo>


void MainWindow::init_uart()
{
    QString  uartDevName = appConfig::get_uart_device();
    uint32_t uartBaud    = appConfig::get_uart_speed ();
    if (uartDevName.isEmpty() || !uartBaud)
        return;

    QList<QSerialPortInfo> info = QSerialPortInfo::availablePorts();
    bool exists = false;
    QSerialPortInfo u;
    foreach (QSerialPortInfo i, info)
        if (i.portName() == uartDevName)
        {
            exists = true;
            u = i;
        }

    if (!exists)
        return;
    uart = new QSerialPort(this);
    uart->setPort(u);
    uart->setBaudRate(uartBaud);
    uart->setDataBits(QSerialPort::Data8);
    uart->setStopBits(QSerialPort::OneStop);
    if (!uart->open(QSerialPort::ReadWrite))
    {
        appLog::write(LOG_LEVEL_UART, QString("Error open UART %1 : %2")
                      .arg(u.portName())
                      .arg(uart->errorString()));
    }

    //connect(uart, &QIODevice::readyRead, this, &MainWindow::onUARTread, Qt::ConnectionType::QueuedConnection);
    initRecvParser(uart);
}

void MainWindow::initRecvParser(QIODevice* io)
{
    recvParser.setDevId(devId);
    recvParser.setIoDevice(io);
    connect(&recvParser, &RecvParser::camSwitch, this, &MainWindow::onCamSwitch);
    connect(&recvParser, &RecvParser::appState, this, &MainWindow::reqAppState);
    connect(&recvParser, &RecvParser::setDateTime, this, &MainWindow::setSystemDateTime);
    connect(&recvParser, &RecvParser::errorPacket, this, &MainWindow::errorPacket);
}


void MainWindow::deinitUART()
{
    recvParser.setIoDevice(nullptr);
    recvParser.disconnect();
    if (uart && uart->isOpen())
    {
        uart->close();
        uart->deleteLater();
        uart = nullptr;
    }
}

void MainWindow::errorPacket(QByteArray packet, bool crc)
{
    appLog::write(LOG_LEVEL_UART_ERROR, QString("Wrong packet :") + QString(crc ? "error CRC" : ""));
    appLog::write(LOG_LEVEL_UART_ERROR, packet.toHex().toUpper());
}

void MainWindow::setSystemDateTime(QDateTime dt)
{
    QString dateTimeString = QString("sudo date -s \"%1\"").arg(dt.toString("yyyy-MM-dd hh:mm"));
    int res = system(dateTimeString.toStdString().c_str());
    Q_UNUSED(res)
    appLog::write(LOG_LEVEL_PARSER, QString("Accepted new date/time: %1 : result = %2").arg(dt.toString()).arg(res));
    //appLog::write(LOG_LEVEL_PARSER, QString("command %1").arg(dateTimeString));
}


void MainWindow::reqAppState()
{
    appLog::write(LOG_LEVEL_PARSER, "Respond AppState");
    uart->write(makePck(PCT_STATE, devId, QByteArray((char*)&appState, sizeof(appState))));
}

