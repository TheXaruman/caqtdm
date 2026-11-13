#include "casound.h"
#include <fileFunctions.h>
#include <searchfile.h>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QUrl>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#else
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#endif

Q_LOGGING_CATEGORY(caSoundLog, "caqtdm.widgets.casound")

caSound::caSound(QWidget *parent) : QWidget{parent}
{
    m_audioBuffer = new QBuffer(this);
    m_format = new QAudioFormat();
    m_decoder = new QAudioDecoder(this);
    connect(m_decoder, &QAudioDecoder::bufferReady, this, &caSound::onBufferReady);
    connect(m_decoder, &QAudioDecoder::finished, this, &caSound::onDecodingFinished);
    connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this, &caSound::onDecoderError);
    connect(this, &caSound::fileChanged, this, &caSound::onFileChanged);
    thisVolume = 50;
}

caSound::~caSound()
{
    if (m_sink != nullptr) {
        m_sink->stop();
        delete m_sink;
    }
    delete m_format;
}

void caSound::setFile(const QString &file){
    if (file == thisFile) {
        return;
    }
    fileFunctions fileFunc;
    fileFunc.checkFileAndDownload(file);
    searchFile searcher(file);
    QString foundPath = searcher.findFile();
    if (!foundPath.isNull()) {
        qCDebug(caSoundLog) << "Found file: " << foundPath;
        m_resolvedFile = foundPath;
        thisFile = file;
    } else {
        qCDebug(caSoundLog) << "Didn't find file";
        m_resolvedFile.clear();
        qCDebug(caSoundLog) << "Cleared resolved file";
    }
    emit fileChanged(m_resolvedFile);
}

QString caSound::file() const{
    return thisFile;
}

void caSound::setVolume(int volume){
    thisVolume = qBound(0, volume, 100);
    if (m_sink != nullptr) {
        m_sink->setVolume(thisVolume / 100.0);
    }
}

int caSound::volume() const{
    return thisVolume;
}

bool caSound::isDecoding() const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return m_decoder->state() == QAudioDecoder::DecodingState;
#else
    return m_decoder->isDecoding();
#endif
}

void caSound::onFileChanged(const QString &file) {
    if (file.isEmpty())
        return;
    if (isDecoding()) {
        m_decoder->stop();
    }
    if (m_sink != nullptr) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }
    m_pcm.clear();
    *m_format = QAudioFormat();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_decoder->setSourceFilename(file);
#else
    m_decoder->setSource(QUrl::fromLocalFile(file));
#endif
    m_decoder->start();
}

void caSound::onBufferReady() {
    QAudioBuffer chunk = m_decoder->read();
    if (!chunk.isValid()) {
        return;
    }
    if (!m_format->isValid()) {
        *m_format = chunk.format();
        qCDebug(caSoundLog) << m_format->sampleRate();
        qCDebug(caSoundLog) << m_format->channelCount();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qCDebug(caSoundLog) << m_format->sampleSize() << m_format->sampleType();
#else
        qCDebug(caSoundLog) << m_format->sampleFormat();
#endif
    }
    m_pcm.append(chunk.constData<char>(), chunk.byteCount());
}

void caSound::onDecodingFinished(){
    if (m_audioBuffer->isOpen()) {
        m_audioBuffer->close();
    }
    if (!m_pcm.isEmpty()) {
        m_audioBuffer->setBuffer(&m_pcm);
    }
    m_audioBuffer->open(QIODevice::ReadOnly);
    if (m_playPending) {
        m_playPending = false;
        play();
    }
}

void caSound::onDecoderError() {
    m_playPending = false;
    qCWarning(caSoundLog) << "Decoding failed:" << m_decoder->errorString();
}

void caSound::play() {
    qCDebug(caSoundLog) << "Entered play";
    if (m_resolvedFile.isEmpty()) {
        qCDebug(caSoundLog) << "No File to play";
        qCDebug(caSoundLog) << "Returning without playing sound";
        return;
    }
    if (thisVolume == 0) {
        qCDebug(caSoundLog) << "Volume: " << thisVolume;
        qCDebug(caSoundLog) << "Sound is mute";
        qCDebug(caSoundLog) << "Returning without playing sound";
        return;
    }
    if (isDecoding() || m_pcm.isEmpty()) {
        qCDebug(caSoundLog) << "Decoding not finished, play deferred";
        m_playPending = true;
        return;
    }
    if (m_sink != nullptr) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    const QString deviceName = device.deviceName();
#else
    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    const QString deviceName = device.description();
#endif
    if (!device.isFormatSupported(*m_format)) {
        qCWarning(caSoundLog) << "Audio format not supported by output device" << deviceName;
        return;
    }
    m_sink = new caSoundSink(device, *m_format, this);
    m_audioBuffer->seek(0);
    qCDebug(caSoundLog) << "Start playing sound";
    m_sink->start(m_audioBuffer);
}
