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

private slots:
    void initTestCase();
    void cleanupTestCase();
    void EmptyIncomingDataShouldNoEmitSignals();
    void InvalidIncomingDataShouldNoEmitSignals();

};

test_parser::test_parser()
{


}

test_parser::~test_parser()
{

}

void test_parser::initTestCase()
{
    cut.setDevId(7);
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

QTEST_APPLESS_MAIN(test_parser)

#include "tst_test_parser.moc"
