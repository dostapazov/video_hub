#include <mainwindow.h>
#include <applog.h>
#include <appconfig.h>
#include <QtCore>
#include <QSerialPortInfo>

void MainWindow::init_uart()
{
    char b[] = {1, 2, 3, CU_SIGNATURE_, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    rxBuf.append(b);
    hasPacket();

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
    uart->open(QSerialPort::ReadWrite);
    if (uart->isOpen())
        connect(uart, &QIODevice::readyRead, this, &MainWindow::onUARTread, Qt::ConnectionType::QueuedConnection);
    qDebug() << Q_FUNC_INFO << uart->isOpen() << uart->errorString() << uart->baudRate();
}

void MainWindow::deinitUART()
{

    if (uart && uart->isOpen())
    {
        uart->close();
        uart->deleteLater();
        uart = nullptr;
    }
}


void MainWindow::onUARTread()
{
    QByteArray b = uart->readAll();
    //appLog::write(6, QString("Recieved %1 byte(s)").arg(b.count()));
    rxBuf.append(b);
    parseReceive();
}


const PCK_Header_t* MainWindow::checkCompletePacket()
{
    if (rxBuf.size() <  EMPTY_PACKET_SIZE)
        return nullptr;
    const PCK_Header_t* hdr = reinterpret_cast<const PCK_Header_t*>(rxBuf.constData());

    if (rxBuf.size() < packetSize(hdr))
        return nullptr;
    return hdr;
}

const PCK_Header_t* MainWindow::hasPacket()
{
    int index = rxBuf.indexOf(CU_SIGNATURE_);
    if (index)
    {
        if (index)
            rxBuf.remove(0, index);
        else
            rxBuf.clear();
    }

    return  checkCompletePacket();
}

void MainWindow::removePacket()
{
    const PCK_Header_t* packet = checkCompletePacket();
    if (packet)
        rxBuf.remove(0, packetSize(packet));
}



void MainWindow::parseReceive()
{

    quint16 offset = 0;
    PCK_Header_t header;
    quint32 crc = 0;
    const PCK_Header_t* hdr;

    while ( (hdr = hasPacket()) )
    {


        if ((crc == pcrc) || 1)
        {
            if (header.devId == appConfig::value("DEV/ID").toUInt())
            {
                switch (header.pckType)
                {
                    case PCT_SHUTDOWN:
                    {
                        close();
                        break;
                    }
                    case PCT_CAM_SWITCH:
                    {
                        quint8 id = 0;
                        if (header.size == sizeof(id))
                        {
                            id = (quint8)rxBuf.at(offset);
                            if (id)
                                emit cam_switch(id - 1);
                        }

                        break;
                    }
                    case PCT_STATE:
                    {
                        if (header.size == 0)
                            uart->write(makePck(PCT_STATE, QByteArray((char*)&appState, sizeof(appState))));
                        break;
                    }
                    case PCT_DATETIME:
                    {
                        if (header.size == sizeof(PCK_DateTime_t))
                        {
                            PCK_DateTime_t* pl = (PCK_DateTime_t*)&rxBuf.data()[offset];
                            QDateTime dt;
                            dt.setDate(QDate(pl->year + 2000, pl->mounth, pl->day));
                            dt.setTime(QTime(pl->hour, pl->min, pl->sec));
                            setSystemDateTime(dt);
                        }
                        break;
                    }
                    default :
                        appLog::write(5, tr("UART unknownd packet type %1").arg(int(header.pckType)));
                        break;
                }
            }
            else
                appLog::write(5, tr("UART different devId [self %1] packet %2 type %3 size %4").arg(appConfig::value("DEV/ID").toUInt()).arg(int(header.devId)).arg(int(header.pckType)).arg(int(header.devId)).arg(int(header.size)));

        }
        removePacket();
    }
}

void MainWindow::setSystemDateTime(QDateTime dt)
{
    QString dateTimeString = QString("sudo date -s %1").arg(dt.toString("\"yyyy-MM-dd hh:mm\""));
    int res = system(dateTimeString.toStdString().c_str());
    Q_UNUSED(res)
    appLog::write(0, QString("Accepted new date/time: %1").arg(dt.toString()));
}
