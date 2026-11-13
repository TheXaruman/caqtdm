#include "tst_casound.h"

#include <casound.h>

#include <QDataStream>
#include <QTemporaryFile>

// makes a wav file
QByteArray TestCaSound::makeWav() {
    int sampleRate = 8000;
    int channels = 1;
    int bits = 16;
    QByteArray data(8000, '\0');

    QByteArray wav;
    QDataStream out(&wav, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    out.writeRawData("RIFF", 4);
    out << quint32(data.size() + 36);
    out.writeRawData("WAVE", 4);
    out.writeRawData("fmt ", 4);
    out << quint32(16);
    out << quint16(1);
    out << quint16(channels);
    out << quint32(sampleRate);
    out << quint32(sampleRate * channels * bits / 8);
    out << quint16(channels * bits / 8);
    out << quint16(bits);
    out.writeRawData("data", 4);
    out << quint32(data.size());
    out.writeRawData(data.constData(), data.size());

    return wav;
}

void TestCaSound::loadsSoundFile() {
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    tmp.write(makeWav());
    tmp.close();

    caSound sound(nullptr);
    sound.setFile(tmp.fileName());

    QCOMPARE(sound.file(), tmp.fileName());
}

void TestCaSound::playsSoundToOutputDevice() {
    QSKIP("Spy output not implemented yet");
}