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
    audioBuffer = new QBuffer(this);
    m_format = new QAudioFormat();
    m_decoder = new QAudioDecoder(this);
    connect(m_decoder, &QAudioDecoder::bufferReady, this, &caSound::createBuffer);
    connect(m_decoder, &QAudioDecoder::finished, this, &caSound::onFinishedBuffer);
    connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this, &caSound::onDecoderError);
    connect(this, &caSound::fileChanged, this, &caSound::onFileChange);
    this->thisvolume = 50;
}

caSound::~caSound()
{
    if (audio != nullptr) {
        audio->stop();
        delete audio;
    }
    delete m_format;
}

void caSound::setfile(const QString &file){
    if (file == this->thisfile) {
        return;
    }
    fileFunctions fileFunc;
    fileFunc.checkFileAndDownload(file);
    searchFile searcher(file);
    QString foundPath = searcher.findFile();
    if (!foundPath.isNull()) {
        qCDebug(caSoundLog) << "Found file: " << foundPath;
        resolvedFile = foundPath;
        this->thisfile = file;
    } else {
        qCDebug(caSoundLog) << "Didn't find file";
        resolvedFile.clear();
        qCDebug(caSoundLog) << "Cleared resolvedFile";
    }
    emit fileChanged(resolvedFile);
}

QString caSound::file() const{
    return this->thisfile;
}

void caSound::setvolume(const int &volume){
    this->thisvolume = volume;
}

int caSound::volume() const{
    return this->thisvolume;
}

bool caSound::isDecoding() const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return m_decoder->state() == QAudioDecoder::DecodingState;
#else
    return m_decoder->isDecoding();
#endif
}

void caSound::onFileChange(const QString &file) {
    if (file.isEmpty())
        return;
    if (isDecoding()) {
        m_decoder->stop();
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

void caSound::createBuffer() {
    QAudioBuffer received_byte = m_decoder->read();
    if (!received_byte.isValid()) {
        return;
    }
    if (!m_format->isValid()) {
        *m_format = received_byte.format();
        qCDebug(caSoundLog) << m_format->sampleRate();
        qCDebug(caSoundLog) << m_format->channelCount();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qCDebug(caSoundLog) << m_format->sampleSize() << m_format->sampleType();
#else
        qCDebug(caSoundLog) << m_format->sampleFormat();
#endif
    }
    m_pcm.append(received_byte.constData<char>(), received_byte.byteCount());
}

void caSound::onFinishedBuffer(){
    if (audioBuffer->isOpen()) {
        audioBuffer->close();
    }
    if (!m_pcm.isEmpty()) {
        audioBuffer->setBuffer(&m_pcm);
    }
    audioBuffer->open(QIODevice::ReadOnly);
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
    if (resolvedFile.isEmpty()) {
        qCDebug(caSoundLog) << "No File to play";
        qCDebug(caSoundLog) << "Returning without playing sound";
        return;
    }
    if (this->thisvolume == 0.0) {
        qCDebug(caSoundLog) << "Volume: " << this->thisvolume;
        qCDebug(caSoundLog) << "Sound is mute";
        qCDebug(caSoundLog) << "Returning without playing sound";
        return;
    }
    if (isDecoding() || m_pcm.isEmpty()) {
        qCDebug(caSoundLog) << "Decoding not finished, play deferred";
        m_playPending = true;
        return;
    }
    if (audio != nullptr) {
        audio->stop();
        delete audio;
        audio = nullptr;
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
    audio = new caSoundSink(device, *m_format, this);
    if (this->thisvolume <= 100.0) {
        audio->setVolume(this->thisvolume / 100.0);
    }
    audioBuffer->seek(0);
    qCDebug(caSoundLog) << "Start playing sound";
    audio->start(audioBuffer);
}
