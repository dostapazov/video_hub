#include <QtTest>
#include "../qt-vlc/recvparser.hpp"

// add necessary includes here

quint32 crc32(QByteArray, quint64, quint64  )
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
    static constexpr quint8 DEV_ID = 7;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void EmptyIncomingDataShouldNoEmitSignals();
    void InvalidIncomingDataShouldNoEmitSignals();
    void IncompleteFrameMustStayInBuffer();
    void NotConsistentPrefixIncompleteFrameMustStayInBufferOnlyValidPart();
    void CamSwitchShouldEmitEvent();
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
    QByteArray packet = makePck(PCT_CAM_SWITCH, DEV_ID, QByteArray("\x01"));
    cut.handleRecv(packet);
    QCOMPARE(cut.bufferSize(), 0);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.takeAt(0).toUInt(), 1);
}

QTEST_APPLESS_MAIN(test_parser)

#include "tst_test_parser.moc"
