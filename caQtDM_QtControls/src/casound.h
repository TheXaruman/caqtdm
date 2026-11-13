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

    Q_PROPERTY(QString file READ file WRITE setFile)
    Q_PROPERTY(int volume READ volume WRITE setVolume)

public:
    void setFile(const QString &file);
    QString file() const;

    void setVolume(int volume);
    int volume() const;

    caSound(QWidget *parent);
    ~caSound();

public slots:
    void play();

private slots:
    void onFileChanged(const QString &file);
    void onDecodingFinished();
    void onDecoderError();
    void onBufferReady();

signals:
    void fileChanged(const QString &file);

private:
    bool isDecoding() const;

    QString thisFile;
    QString m_resolvedFile;
    QBuffer *m_audioBuffer;
    QByteArray m_pcm;
    int thisVolume;
    QAudioDecoder *m_decoder;
    QAudioFormat *m_format;
    caSoundSink *m_sink = nullptr;
    bool m_playPending = false;
};
#endif
