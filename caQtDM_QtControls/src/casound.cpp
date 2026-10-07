#include "casound.h"
#include <fileFunctions.h>
#include <searchfile.h>
#include <QMediaPlayer>
#include <QAudioOutput>


Q_LOGGING_CATEGORY(caSoundLog, "caqtdm.widgets.casound")

caSound::caSound(QWidget *parent) : QWidget{parent}
{
    player = new QMediaPlayer;
    audioOutput = new QAudioOutput;
    player->setAudioOutput(audioOutput);
    this->thisvolume = 50;
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
        qCDebug(caSoundLog) << "caSound:" << "Found file: " << foundPath;
        resolvedFile = foundPath;
        this->thisfile = file;
    } else {
        qCDebug(caSoundLog) << "caSound:" << "Didn't find file";
        resolvedFile.clear();
        qCDebug(caSoundLog) << "caSound:" << "Cleared resolvedFile";
    }
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

void caSound::play() {
    qCDebug(caSoundLog) << "caSound:" << "Entered play";
    if (resolvedFile.isEmpty()) {
        qCDebug(caSoundLog) << "caSound:" << "No File to play";
        qCDebug(caSoundLog) << "caSound:" << "Returning without playing sound";
        return;
    }
    if (this->thisvolume == 0.0) {
        qCDebug(caSoundLog) << "caSound:" << "Volume: " << this->thisvolume;
        qCDebug(caSoundLog) << "caSound:" << "Sound is mute";
        qCDebug(caSoundLog) << "caSound:" << "Returning without playing sound";
        return;
    }
    if (player->audioOutput() != audioOutput) {
        player->setAudioOutput(audioOutput);
        qCDebug(caSoundLog) << "caSound:" << "Set audio output to: " << audioOutput;
    }
    player->setSource(QUrl::fromLocalFile(resolvedFile));
    if (audioOutput->volume() != (this->thisvolume / 100.0) && (this->thisvolume / 100.0) <= 1.0) {
        audioOutput->setVolume(this->thisvolume / 100.0);
        qCDebug(caSoundLog) << "caSound:" << "Set volume to: " << this->thisvolume / 100.0;
    }
    if (!player->source().isEmpty()) {
        qCDebug(caSoundLog) << "caSound:" << "Start playing sound";
        player->play();
    }
}
