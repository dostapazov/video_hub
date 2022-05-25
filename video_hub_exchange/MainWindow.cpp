#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "DataInfo.h"
#include "crc32.h"

#include <QHostAddress>
#include <QDateTime>

const int answerSizeHeader = 10;
const int answerSizeData = 20;
const int answerSizeTime = 22;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->comboBoxStart->addItem(tr("channel A (not activated)"), 0b00000001);
    ui->comboBoxStart->addItem(tr("channel A (activated)"), 0b10000001);
    ui->comboBoxStart->addItem(tr("channel B (not activated)"), 0b00000010);
    ui->comboBoxStart->addItem(tr("channel B (activated)"), 0b10000010);

    crc32_init();
    timer.setInterval(ui->sbInrval->value());
    connect_lost();
    connect(&timer, SIGNAL(timeout()), SLOT(send()));

    connect(&tcpSocket, &QTcpSocket::connected, this, &MainWindow::connect_established);
    connect(&tcpSocket, &QTcpSocket::disconnected, this, &MainWindow::connect_lost);

    ui->buttonConnect->clicked();
    //connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(slotError(QAbstractSocket::SocketError)));


    connect(ui->rb11, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb12, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb13, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb14, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));

    connect(ui->rb21, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb22, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb23, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb24, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));

    connect(ui->rb31, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb32, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb33, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb34, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));

    connect(ui->rb41, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb42, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb43, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));
    connect(ui->rb44, SIGNAL(toggled(bool)), SLOT(sendOnChange(bool)));

    // пытаемся читать данные через кванты времени
    QTimer* t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(read()));
    t->start(10);

    ui->actionEN->trigger();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
        {
            ui->retranslateUi(this);

            int index = ui->comboBoxStart->currentIndex();
            ui->comboBoxStart->clear();
            ui->comboBoxStart->addItem(tr("channel A (not activated)"), 0b00000001);
            ui->comboBoxStart->addItem(tr("channel A (activated)"), 0b10000001);
            ui->comboBoxStart->addItem(tr("channel B (not activated)"), 0b00000010);
            ui->comboBoxStart->addItem(tr("channel B (activated)"), 0b10000010);
            ui->comboBoxStart->setCurrentIndex(index);

            ui->labelConnect->setText(tr(ui->labelConnect->property("tr").toString().toStdString().c_str()));
        }
        break;
        default:
            break;
    }
}

bool MainWindow::tcpConnect(QString host, ushort port)
{
    timer.stop();

    if (tcpSocket.isOpen() && (tcpSocket.peerAddress().toString() != host || tcpSocket.peerPort() != port))
    {
        tcpSocket.close();
        qDebug() << "закрываем устаревшее соединение";
    }

    tcpSocket.connectToHost(host, port);
    bool bRes = tcpSocket.waitForConnected(100);
    if (bRes)
    {
        tcpSocket.setProperty("host", host);
        tcpSocket.setProperty("port", port);
        qDebug() << tr("The tcp connection is established. host : %1 port : %2").arg(host).arg(port);
    }
    else
    {
        tcpSocket.close();
        qDebug() << tr("The tcp connection is not established");
    }


    return bRes;
}

/*void MainWindow::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(tcpSocket.errorString())
                    );
    qDebug() << strError;
}*/

void MainWindow::on_buttonConnect_clicked()
{
    tcpConnect(ui->edHost->text(), ui->sbPort->text().toUShort());
}

void MainWindow::on_buttonTime_clicked()
{
    if (!tcpSocket.isOpen())
    {
        qDebug() << "Устройство не открыто";
        return;
    }

    tx_number++;

    // запрос
    TimeInfo data;
    //data.startByte = 0xA5;  // признак начала пакета
    data.startByte = ui->comboBoxStart->currentData().toUInt(); // Идентификатор ПДУ вместо признака начала пакета
    //data.sessionNumber = 0x1122;    // номер сессии
    data.packageNumber = tx_number; // номер пакета
    //data.versionNumber = 0; // номер версии протокола
    //data.packageType = 3;  // пакет синхронизации времени
    //data.dataSize = 0;  // размер поля данных

    QDateTime dt = QDateTime::currentDateTime();
    data.year = dt.date().year() % 100;
    data.month = dt.date().month();
    data.day = dt.date().day();
    data.hour = dt.time().hour();
    data.min = dt.time().minute();
    data.sec = dt.time().second();
    data.msec = dt.time().msec();

    QByteArray ba = data.toByteArray();

    uint32_t crc ; //

    // контрольная сумма
    crc = crc32(ba, 0, sizeof(data));
    //crc = crc32_byte(0, reinterpret_cast<const uint8_t*>(ba.constData()), 16);
    char d[4];
    memcpy(d, &crc, 4);
    QByteArray dataCrc(d, 4);
    ba.append(dataCrc);

    tcpSocket.write(ba);
    tcpSocket.flush();
//    if(tcpSocket.error()!= QAbstractSocket::UnknownSocketError )
//    {
//        tcpSocket.disconnectFromHost();
//        ui->teRequest->clear();
//    }

    QString res;
    res += QString(tr("Start byte: 0x%1\n")).arg(ba.mid(0, 1).toHex().toStdString().c_str());
    res += QString(tr("Session number: 0x%1%2\n")).arg(ba.mid(2, 1).toHex().toStdString().c_str()).arg(ba.mid(1, 1).toHex().toStdString().c_str());
    res += QString(tr("Package number: %1\n")).arg(data.packageNumber);
    res += QString(tr("Data version: %1\n")).arg(data.versionNumber);
    res += QString(tr("Package type: %1\n")).arg(data.packageType);
    res += QString(tr("Data size: %1\n")).arg(data.dataSize);
    res += QString(tr("Msec: 0x%1\n")).arg(ba.mid(10, 2).toHex().toStdString().c_str());
    res += QString(tr("Sec: 0x%1\n")).arg(ba.mid(12, 1).toHex().toStdString().c_str());
    res += QString(tr("Min: 0x%1\n")).arg(ba.mid(13, 1).toHex().toStdString().c_str());
    res += QString(tr("Hour: 0x%1\n")).arg(ba.mid(14, 1).toHex().toStdString().c_str());
    res += QString(tr("Day: 0x%1\n")).arg(ba.mid(15, 1).toHex().toStdString().c_str());
    res += QString(tr("Month: 0x%1\n")).arg(ba.mid(16, 1).toHex().toStdString().c_str());
    res += QString(tr("Year: 0x%1\n")).arg(ba.mid(17, 1).toHex().toStdString().c_str());
//    res += QString(tr("CRC-32: 0x%1\n")).arg(ba.mid(18, 4).toHex().toStdString().c_str());
    res += QString(tr("CRC-32: 0x%1\n")).arg(crc, 0, 16);
    ui->teRequest->setText(res);
}

void MainWindow::send()
{
    if (!tcpSocket.isOpen())
    {
        qDebug() << "Устройство не открыто";
        return;
    }

    tx_number++;

    // запрос
    DataInfo data;
    //data.startByte = 0xA5;  // признак начала пакета
    data.startByte = ui->comboBoxStart->currentData().toUInt(); // Идентификатор ПДУ вместо признака начала пакета
    //data.sessionNumber = 0x1122;    // номер сессии
    data.packageNumber = tx_number; // номер пакета
    //data.versionNumber = 0; // номер версии протокола
    //data.packageType = 2;  // тип пакета
    //data.dataSize = 0;  // размер поля данных

    //data.error = 0; // ошибки
    data.channel1 = 0; // канал 1
    if (ui->rb11->isChecked())
        data.channel1 = 1;
    if (ui->rb12->isChecked())
        data.channel1 = 2;
    if (ui->rb13->isChecked())
        data.channel1 = 4;
    if (ui->rb14->isChecked())
        data.channel1 = 8;

    data.channel2 = 0; // канал 2

    if (ui->rb21->isChecked())
        data.channel2 = 1;
    if (ui->rb22->isChecked())
        data.channel2 = 2;
    if (ui->rb23->isChecked())
        data.channel2 = 4;
    if (ui->rb24->isChecked())
        data.channel2 = 8;

    data.channel3 = 0; // канал 3
    if (ui->rb31->isChecked())
        data.channel3 = 1;
    if (ui->rb32->isChecked())
        data.channel3 = 2;
    if (ui->rb33->isChecked())
        data.channel3 = 4;
    if (ui->rb34->isChecked())
        data.channel3 = 8;

    data.channel4 = 0; // канал 4
    if (ui->rb41->isChecked())
        data.channel4 = 1;
    if (ui->rb42->isChecked())
        data.channel4 = 2;
    if (ui->rb43->isChecked())
        data.channel4 = 4;
    if (ui->rb44->isChecked())
        data.channel4 = 8;

    QByteArray ba = data.toByteArray();

    uint32_t crc ; //

    // контрольная сумма
    crc = crc32(ba, 0, sizeof(data));
    //crc = crc32_byte(0, reinterpret_cast<const uint8_t*>(ba.constData()), 16);
    char d[4];
    memcpy(d, &crc, 4);
    QByteArray dataCrc(d, 4);
    ba.append(dataCrc);

    tcpSocket.write(ba);
    tcpSocket.flush();
//    if(tcpSocket.error()!= QAbstractSocket::UnknownSocketError )
//    {
//        tcpSocket.disconnectFromHost();
//        ui->teRequest->clear();
//    }

    QString res;
    res += QString(tr("Start byte: 0x%1\n")).arg(ba.mid(0, 1).toHex().toStdString().c_str());
    res += QString(tr("Session number: 0x%1%2\n")).arg(ba.mid(2, 1).toHex().toStdString().c_str()).arg(ba.mid(1, 1).toHex().toStdString().c_str());
    res += QString(tr("Package number: %1\n")).arg(data.packageNumber);
    res += QString(tr("Data version: %1\n")).arg(data.versionNumber);
    res += QString(tr("Package type: %1\n")).arg(data.packageType);
    res += QString(tr("Data size: %1\n")).arg(data.dataSize);
    res += QString(tr("Errors: 0x%1\n")).arg(ba.mid(10, 2).toHex().toStdString().c_str());
    res += QString(tr("Output 1: 0x%1\n")).arg(ba.mid(12, 1).toHex().toStdString().c_str());
    res += QString(tr("Output 2: 0x%1\n")).arg(ba.mid(13, 1).toHex().toStdString().c_str());
    res += QString(tr("Output 3: 0x%1\n")).arg(ba.mid(14, 1).toHex().toStdString().c_str());
    res += QString(tr("Output 4: 0x%1\n")).arg(ba.mid(15, 1).toHex().toStdString().c_str());
//    res += QString(tr("CRC-32: 0x%1\n")).arg(ba.mid(16, 4).toHex().toStdString().c_str());
    res += QString(tr("CRC-32: 0x%1\n")).arg(crc, 0, 16);
    ui->teRequest->setText(res);
}

void MainWindow::sendOnChange(bool checked)
{
//    if (!checked)
//        return;
//    int time = timer.remainingTime();
//    if (time < 10)
//        return;
//    if ((timer.interval() - time) < 10)
//        QTimer::singleShot(10, this, SLOT(send()));
//    else
    Q_UNUSED(checked)
    send();
}

QByteArray MainWindow::getAnswer()
{
    // начало пакета
    while (buffer.size() && buffer.at(0) != 0x5A)
        buffer.remove(0, 1);

    // читаем данные из буфера, данные удаляются из буфера
    if (buffer.size() < answerSizeHeader)
        return QByteArray();

    uint16_t dataSize = 0;
    memcpy(&dataSize, buffer.data() + 8, 2);
    int size = answerSizeHeader + dataSize + 4;
    if (buffer.size() < size)
        return QByteArray();

    QByteArray res = buffer.left(size);
    buffer.remove(0, size);
    return res;
}

void MainWindow::read()
{
    if (!tcpSocket.isOpen())
        return;

    // если есть доступные для чтения байты, добавляем их в буфер
    qint64 bytes = tcpSocket.bytesAvailable();
    if (bytes > 0)
    {
        buffer += tcpSocket.readAll();
        if (buffer.size() > answerSizeHeader)
            readAnswer();
    }
}

bool MainWindow::readAnswer()
{
    QByteArray answer;
    do
    {
        answer = getAnswer();
        if (answer.isEmpty())
            break;
        if (answerSizeData == answer.size())
            readAnswerData(answer);
        if (answerSizeTime == answer.size())
            readAnswerTime(answer);
    }
    while (answer.size());

    return true;
}

void MainWindow::readAnswerData(QByteArray answer)
{
    if (answer.size() >= answerSizeData)
    {
        DataInfo data;
        data.fromByteArray(answer.mid(0, 16));

        uint32_t crc;
        memcpy(&crc, answer.mid(16, 4), 4);
        if (crc == crc32(answer, 0, sizeof(DataInfo)))
        {
            if (rx_number && ( (data.packageNumber - rx_number) != 1 ))
                qDebug() << tr("error packet number new %1  old %2").arg(data.packageNumber).arg(rx_number);

            rx_number = data.packageNumber;

            QString res;
            res += QString(tr("Start byte: 0x%1\n")).arg(answer.mid(0, 1).toHex().toStdString().c_str());
            res += QString(tr("Session number: 0x%1%2\n")).arg(answer.mid(2, 1).toHex().toStdString().c_str()).arg(answer.mid(1, 1).toHex().toStdString().c_str());
            res += QString(tr("Package number: %1\n")).arg(data.packageNumber);
            res += QString(tr("Data version: %1\n")).arg(data.versionNumber);
            res += QString(tr("Package type: %1\n")).arg(data.packageType);
            res += QString(tr("Data size: %1\n")).arg(data.dataSize);
            res += QString(tr("Errors: 0x%1%2\n")).arg(answer.mid(11, 1).toHex().toStdString().c_str()).arg(answer.mid(10, 1).toHex().toStdString().c_str());
            res += QString(tr("Output 1: 0x%1\n")).arg(answer.mid(12, 1).toHex().toStdString().c_str());
            res += QString(tr("Output 2: 0x%1\n")).arg(answer.mid(13, 1).toHex().toStdString().c_str());
            res += QString(tr("Output 3: 0x%1\n")).arg(answer.mid(14, 1).toHex().toStdString().c_str());
            res += QString(tr("Output 4: 0x%1\n")).arg(answer.mid(15, 1).toHex().toStdString().c_str());
            res += QString(tr("CRC-32: 0x%1\n")).arg(answer.mid(16, 4).toHex().toStdString().c_str());
            ui->teAnswer->setText(res);
        }
        else
            qDebug() << tr("The checksum doesn't match.");
    }
    else
        qDebug() << "Answer size dismiss.";
}

void MainWindow::readAnswerTime(QByteArray answer)
{
    if (answer.size() >= answerSizeTime)
    {
        TimeInfo data;
        data.fromByteArray(answer.mid(0, 18));

        uint32_t crc;
        memcpy(&crc, answer.mid(18, 4), 4);
        if (crc == crc32(answer, 0, sizeof(TimeInfo)))
        {
            if (rx_number && ( (data.packageNumber - rx_number) != 1 ))
                qDebug() << tr("error packet number new %1  old %2").arg(data.packageNumber).arg(rx_number);

            rx_number = data.packageNumber;

            QString res;
            res += QString(tr("Start byte: 0x%1\n")).arg(answer.mid(0, 1).toHex().toStdString().c_str());
            res += QString(tr("Session number: 0x%1%2\n")).arg(answer.mid(2, 1).toHex().toStdString().c_str()).arg(answer.mid(1, 1).toHex().toStdString().c_str());
            res += QString(tr("Package number: %1\n")).arg(data.packageNumber);
            res += QString(tr("Data version: %1\n")).arg(data.versionNumber);
            res += QString(tr("Package type: %1\n")).arg(data.packageType);
            res += QString(tr("Data size: %1\n")).arg(data.dataSize);
            res += QString(tr("Msec: 0x%1\n")).arg(answer.mid(10, 2).toHex().toStdString().c_str());
            res += QString(tr("Sec: 0x%1\n")).arg(answer.mid(12, 1).toHex().toStdString().c_str());
            res += QString(tr("Min: 0x%1\n")).arg(answer.mid(13, 1).toHex().toStdString().c_str());
            res += QString(tr("Hour: 0x%1\n")).arg(answer.mid(14, 1).toHex().toStdString().c_str());
            res += QString(tr("Day: 0x%1\n")).arg(answer.mid(15, 1).toHex().toStdString().c_str());
            res += QString(tr("Month: 0x%1\n")).arg(answer.mid(16, 1).toHex().toStdString().c_str());
            res += QString(tr("Year: 0x%1\n")).arg(answer.mid(17, 1).toHex().toStdString().c_str());
            res += QString(tr("CRC-32: 0x%1\n")).arg(crc, 0, 16);
            ui->teAnswer->setText(res);
        }
        else
            qDebug() << tr("The checksum doesn't match.");
    }
    else
        qDebug() << "Answer size dismiss.";
}

void MainWindow::on_sbInrval_valueChanged(int arg1)
{
    timer.stop();
    timer.setInterval(arg1);
    timer.start();
}

void MainWindow::slotError          (QAbstractSocket::SocketError)
{

}

void MainWindow::connect_established()
{
    timer.setInterval(ui->sbInrval->value());
    timer.start();
    this->tx_number = this->rx_number = 0;
    ui->labelConnect->setText(tr("Connected"));
    ui->labelConnect->setProperty("tr", "Connected");
    QPalette palette = ui->labelConnect->palette();
    palette.setColor(QPalette::WindowText, Qt::green );
    ui->labelConnect->setPalette(palette);
}

void MainWindow::connect_lost()
{
    timer.stop();
    if (tcpSocket.isOpen())
    {
        tcpSocket.disconnectFromHost();
        tcpSocket.close();
        ui->labelConnect->setText(tr("Not connected"));
        ui->labelConnect->setProperty("tr", "Not connected");
        QPalette palette = ui->labelConnect->palette();
        palette.setColor(QPalette::WindowText, Qt::red );
        ui->labelConnect->setPalette(palette);
    }
}

void MainWindow::on_actionEN_triggered()
{
    ui->actionRU->setChecked(false);
    ui->actionEN->setChecked(true);
    qtLanguageTranslator.load(QString("video_hub_exchange_") + QString("en_EN"), qApp->applicationDirPath());
    qApp->installTranslator(&qtLanguageTranslator);
}

void MainWindow::on_actionRU_triggered()
{
    ui->actionEN->setChecked(false);
    ui->actionRU->setChecked(true);
    qtLanguageTranslator.load(QString("video_hub_exchange_") + QString("ru_RU"), qApp->applicationDirPath());
    qApp->installTranslator(&qtLanguageTranslator);
}
