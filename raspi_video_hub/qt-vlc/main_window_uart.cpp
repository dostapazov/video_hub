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
        appLog::write(0, QString("Error open UART %1 : %2")
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
    appLog::write(0, QString("Wrong packet :") + QString(crc ? "error CRC" : ""));
    appLog::write(0, packet.toHex().toUpper());
}

void MainWindow::setSystemDateTime(QDateTime dt)
{
    QString dateTimeString = QString("sudo date -s %1").arg(dt.toString("\'yyyy-MM-dd hh:mm\'"));
    int res = system(dateTimeString.toStdString().c_str());
    Q_UNUSED(res)
    appLog::write(0, QString("Accepted new date/time: %1").arg(dt.toString()));
}


void MainWindow::reqShutDown()
{
    qDebug() << "shutdown request";
}

void MainWindow::reqUpdateExecuteble()
{
    qDebug() << "update executable";
}

void MainWindow::reqAppState()
{
    uart->write(makePck(PCT_STATE, devId, QByteArray((char*)&appState, sizeof(appState))));
}

//void MainWindow::onUARTread()
//{
//    QByteArray b = uart->readAll();
//    //appLog::write(6, QString("Recieved %1 byte(s)").arg(b.count()));
//    rxBuf.append(b);
//    parseReceive();
//}

//void MainWindow::parseReceive()
//{
//    constexpr int EMPTY_PACKET_SIZE = (sizeof(PCK_Header_t) + sizeof(quint32));
//    quint16 offset = 0;
//    PCK_Header_t header;
//    quint32 crc = 0;
//    bool hasPck = (rxBuf.count() > EMPTY_PACKET_SIZE);

//    while (hasPck)
//    {
//        while ((CU_SIGNATURE_ != rxBuf[offset]) && (offset < rxBuf.count()))
//            offset++;

//        rxBuf.remove(0, offset);

//        hasPck = false;

//        if (rxBuf.count() >= EMPTY_PACKET_SIZE)
//        {
//            memcpy(&header, rxBuf.constData(), sizeof(header));
//            offset = sizeof(header);

//            if (rxBuf.count() >= (offset + EMPTY_PACKET_SIZE))
//            {
//                quint32 pcrc = 0;
//                crc = crc32(rxBuf, 0, header.size + offset);
//                memcpy(&pcrc, rxBuf.constData() + offset + header.size, sizeof(pcrc));

//                if ((crc == pcrc) || 1)
//                {
//                    if (header.devId == devId)
//                    {
//                        switch (header.pckType)
//                        {
//                            case PCT_SHUTDOWN:
//                            {
//                                close();
//                                break;
//                            }
//                            case PCT_CAM_SWITCH:
//                            {
//                                quint8 id = 0;
//                                if (header.size == sizeof(id))
//                                {
//                                    id = (quint8)rxBuf.at(offset);
//                                    if (id)
//                                        emit cam_switch(id - 1);
//                                }

//                                break;
//                            }
//                            case PCT_STATE:
//                            {
//                                if (header.size == 0)
//                                    reqAppState();
//                                break;
//                            }
//                            case PCT_DATETIME:
//                            {
//                                if (header.size == sizeof(PCK_DateTime_t))
//                                {
//                                    PCK_DateTime_t* pl = (PCK_DateTime_t*)&rxBuf.data()[offset];
//                                    QDateTime dt;
//                                    dt.setDate(QDate(pl->year + 2000, pl->mounth, pl->day));
//                                    dt.setTime(QTime(pl->hour, pl->min, pl->sec));
//                                    setSystemDateTime(dt);
//                                }
//                                break;
//                            }
//                            default :
//                                appLog::write(5, tr("UART unknownd packet type %1").arg(int(header.pckType)));
//                                break;
//                        }
//                    }
//                    //else
//                    // appLog::write(5, tr("UART different devId [self %1] packet %2 type %3 size %4").arg(appConfig::value("DEV/ID").toUInt()).arg(int(header.devId)).arg(int(header.pckType)).arg(int(header.devId)).arg(int(header.size)));

//                    offset += header.size + sizeof(crc);
//                }
//                else
//                {
//                    offset = 1;
//                    appLog::write(5, "UART error crc");
//                }

//                rxBuf.remove(0, offset);
//                hasPck = (rxBuf.count() > EMPTY_PACKET_SIZE);
//            }
//        }
//    }
//}



