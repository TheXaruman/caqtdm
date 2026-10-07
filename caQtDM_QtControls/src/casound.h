#ifndef CASOUND_H
#define CASOUND_H

#include <QWidget>
#include <qtcontrols_global.h>


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

public slots:
    void play();

private:
    QString thisfile;
    QString resolvedFile;
    double thisvolume;
    class QMediaPlayer *player;
    class QAudioOutput *audioOutput;
};
#endif
