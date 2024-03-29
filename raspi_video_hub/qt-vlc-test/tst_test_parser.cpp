#include <QtTest>
#include "../qt-vlc/recvparser.hpp"

// add necessary includes here

quint32 crc32(const QByteArray&, int, int )
{

    return quint32(0xFEED0CBC);
}

class TestedRecvParser: public RecvParser
{

public:
    explicit TestedRecvParser(): RecvParser() {}
    using RecvParser::handleRecv;
};


class test_parser : public QObject
{
    Q_OBJECT

public:
    test_parser();
    ~test_parser();
private:
    TestedRecvParser cut;
    static constexpr quint8 DEV_ID = 3;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void EmptyIncomingDataShouldNoEmitSignals();
    void InvalidIncomingDataShouldNoEmitSignals();
    void IncompleteFrameMustStayInBuffer();
    void NotConsistentPrefixIncompleteFrameMustStayInBufferOnlyValidPart();
    void CamSwitchShouldEmitEvent();
    void CamSwitchShouldNotEmitEventWhenRepeatCamId();
    void CamSwitchShouldNotEmitEventWhenWrongDataSize();
    void AppStateShouldEmitEvent();
    void AppStateShouldNotEmitEventWhenWrongDataSize();
    void DateTimeShouldEmitEvent();
    void DateTimeShouldNotEmitEventWhenWrongDataSize();
    void ParserShouldNotEmitAnyEventWhenWrongDevId();
    void ParserShouldFilterGarbageData();
    void ReceivePer1byteShouldCorrectHandled();
};

test_parser::test_parser()
{

}

test_parser::~test_parser()
{

}

void test_parser::initTestCase()
{
    cut.setDevId(DEV_ID);
    cut.setPacketSignature(RP_SIGNATURE_);
}

void test_parser::cleanupTestCase()
{

}

void test_parser::EmptyIncomingDataShouldNoEmitSignals()
{

    cut.handleRecv(QByteArray());
    QCOMPARE(cut.bufferSize(), 0);
}

void test_parser::InvalidIncomingDataShouldNoEmitSignals()
{
    cut.handleRecv(QByteArray("abcdef"));
    QCOMPARE(cut.bufferSize(), 0);
}

void test_parser::IncompleteFrameMustStayInBuffer()
{
    QByteArray packet = makePck(1, DEV_ID, QByteArray("abcdef"));
    QByteArray half = packet.left(packet.size() / 2);
    cut.handleRecv(half);
    QCOMPARE(cut.bufferSize(), half.size());
}

void test_parser::NotConsistentPrefixIncompleteFrameMustStayInBufferOnlyValidPart()
{
    QByteArray packet = makePck(1, DEV_ID, QByteArray("abcdef"));
    QByteArray half = packet.left(packet.size() / 2);
    cut.handleRecv(QByteArray("abcdef") + half);
    QCOMPARE(cut.bufferSize(), half.size());
}

void test_parser::CamSwitchShouldEmitEvent()
{
    QSignalSpy spy(&cut, &RecvParser::camSwitch);
    char camId = 1;
    QByteArray packet = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray(&camId, 1));
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.takeAt(0).toUInt(), camId - 1 );
}

//TODO тест запрета повторного вызова смены камеры при одинаковом номере
void test_parser::CamSwitchShouldNotEmitEventWhenRepeatCamId()
{
    QSignalSpy spy(&cut, &RecvParser::camSwitch);
    char camId = 7;
    QByteArray packet = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray(&camId, 1));
    cut.handleRecv(packet);
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.takeAt(0).toUInt(), camId - 1 );
}


void test_parser::CamSwitchShouldNotEmitEventWhenWrongDataSize()
{
    QSignalSpy spy(&cut, &RecvParser::camSwitch);
    QByteArray packet = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray("\x01\x2"));
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 0);
}

void test_parser::ParserShouldNotEmitAnyEventWhenWrongDevId()
{
    QSignalSpy spy_cam(&cut, &RecvParser::camSwitch);
    QByteArray packet = makePck(PCT_CAM_SWITCH, DEV_ID + 1, QByteArray("\x01"));
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy_cam.count(), 0);
}

void test_parser::AppStateShouldEmitEvent()
{
    QSignalSpy spy(&cut, &RecvParser::appState);
    QByteArray packet = makePck(PCT_STATE, DEV_ID, QByteArray());
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);

}

void test_parser::AppStateShouldNotEmitEventWhenWrongDataSize()
{
    QSignalSpy spy(&cut, &RecvParser::appState);
    QByteArray packet = makePck(PCT_STATE, DEV_ID, QByteArray("\x01"));
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 0);

}

void fromQDateTime(const QDateTime& src, PCK_DateTime_t& dtm)
{
    dtm.day = src.date().day();
    dtm.mounth = src.date().month();
    dtm.year = src.date().year() - 2000;
    dtm.hour = src.time().hour();
    dtm.min = src.time().minute();
    dtm.sec = src.time().second();
}

void test_parser::DateTimeShouldEmitEvent()
{
    QSignalSpy spy(&cut, &RecvParser::setDateTime);
    // setyp date time
    QByteArray data(sizeof (PCK_DateTime_t), Qt::Uninitialized);
    PCK_DateTime_t* dtm = reinterpret_cast<PCK_DateTime_t*>(data.data());
    QDateTime now = QDateTime::currentDateTime();

    // remove msecs because PCK_DateTime_t have't it
    now.setTime(now.time().addMSecs(-now.time().msec()));

    fromQDateTime(now, *dtm);
    QByteArray packet = makePck(PCT_DATETIME, DEV_ID, data);

    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.count(), 1);
    QCOMPARE(args.takeFirst().toDateTime(), now);

}

void test_parser::ReceivePer1byteShouldCorrectHandled()
{
    QSignalSpy spy(&cut, &RecvParser::setDateTime);
    // setyp date time
    QByteArray data(sizeof (PCK_DateTime_t), Qt::Uninitialized);
    PCK_DateTime_t* dtm = reinterpret_cast<PCK_DateTime_t*>(data.data());
    QDateTime now = QDateTime::currentDateTime();

    // remove msecs because PCK_DateTime_t have't it
    now.setTime(now.time().addMSecs(-now.time().msec()));

    fromQDateTime(now, *dtm);
    QByteArray packet = makePck(PCT_DATETIME, DEV_ID, data);

    for (int i = 0; i < packet.size(); i++)
    {
        cut.handleRecv(QByteArray(packet.data() + i, 1));
    }
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.count(), 1);
    QCOMPARE(args.takeFirst().toDateTime(), now);

}


void test_parser::DateTimeShouldNotEmitEventWhenWrongDataSize()
{
    QSignalSpy spy(&cut, &RecvParser::setDateTime);
    QByteArray data(sizeof (PCK_DateTime_t) +1, Qt::Uninitialized);
    QByteArray packet = makePck(PCT_DATETIME, DEV_ID, data);
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 0);
}

void test_parser::ParserShouldFilterGarbageData()
{
    QSignalSpy spy(&cut, &RecvParser::camSwitch);
    QByteArray packet1 = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray("\x10"));
    QByteArray packet2 = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray("\x20"));
    QByteArray garbage1(3, RP_SIGNATURE_);
    QByteArray garbage2("abcdef");

    cut.handleRecv(garbage2 + packet1 + garbage1 + packet2);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 2);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.takeAt(0).toUInt(), 0x10 - 1);
    args = spy.takeFirst();
    QCOMPARE(args.takeAt(0).toUInt(), 0x20 - 1);

}



QTEST_APPLESS_MAIN(test_parser)

#include "tst_test_parser.moc"
