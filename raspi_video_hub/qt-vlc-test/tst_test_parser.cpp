#include <QtTest>
#include "../qt-vlc/recvparser.hpp"

// add necessary includes here

quint32 crc32(QByteArray data, quint64 start, quint64 len )
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


QTEST_APPLESS_MAIN(test_parser)

#include "tst_test_parser.moc"
