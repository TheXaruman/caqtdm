#ifndef CASOUND_H
#define CASOUND_H
#include <QWidget>
#include <QBuffer>
#include <qtcontrols_global.h>

class QAudioDecoder;
class QAudioFormat;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QAudioOutput;
typedef QAudioOutput caSoundSink;
#else
class QAudioSink;
typedef QAudioSink caSoundSink;
#endif

class QTCON_EXPORT caSound : public QWidget {
    Q_OBJECT

    Q_PROPERTY(QString file READ file WRITE setfile)
    Q_PROPERTY(int volume READ volume WRITE setvolume)

public:
    void setfile(const QString &file);
    QString file() const;

    void setvolume(const int &volume);
    int volume() const;

    caSound(QWidget *parent);
    ~caSound();

public slots:
    void play();

private slots:
    void onFileChange(const QString &file);
    void onFinishedBuffer();
    void onDecoderError();
    void createBuffer();

signals:
    void fileChanged(const QString &file);

private:
    bool isDecoding() const;

    QString thisfile;
    QString resolvedFile;
    QBuffer *audioBuffer;
    QByteArray m_pcm;
    int thisvolume;
    QAudioDecoder *m_decoder;
    QAudioFormat *m_format;
    caSoundSink *audio = nullptr;
    bool m_playPending = false;
};
#endif
