#include "casound.h"

caSound::caSound(QWidget *parent) : QWidget{parent}
{

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
        this->thisfile = file;
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
