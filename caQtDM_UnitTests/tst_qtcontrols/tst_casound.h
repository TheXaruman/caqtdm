#ifndef TST_CASOUND_H
#define TST_CASOUND_H

#include <QObject>
#include <QTest>
#include <QByteArray>

class TestCaSound : public QObject
{
    Q_OBJECT

public:
    QByteArray makeWav();

private slots:
    void loadsSoundFile();
    void playsSoundToOutputDevice();
};

#endif // TST_CASOUND_H
