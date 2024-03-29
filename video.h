/*
Copyright (c) 2015 Clement Roblot

This file is part of localtube.

Localtube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Localtube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with localtube.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VIDEO_H
#define VIDEO_H

#include <QObject>
#include <QRegExp>
#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStorageInfo>


extern QString *pathToFiles;


typedef enum
{
    videoDoneDownloaded,
    videoDownloading,
    videoNotDownloaded,
    videoError

}VideoStatus;




class Video : public QObject
{
    Q_OBJECT
public:
    explicit Video(QString title, QString link, QSettings *settings, QObject *parent = 0);
    explicit Video(QString id, QSettings *settings, QObject *parent = 0);
    ~Video();
    QString getTitle()const{return title;}
    QString getLink()const{return link;}
    QString getCode()const{return code;}
    QDateTime getReleaseDate()const{return releaseDate;}
    bool isVideoInitialised(){return haveBeenInitialised;}
    bool isVideoInitialising(){return isBeingInitialised;}

    void setInitialising(bool status){isBeingInitialised = status;}

    bool download();

    bool operator<(const Video &i1) const;
    static bool lessThan(const Video *v1, const Video *v2);
    VideoStatus getStatus();


signals:
    void videoDownloaded(Video *);
    void videoDownloadStarted(Video *);
    void videoStatusChanged();

public slots:
    void doneDownloading();
    void stopDownload();
    void reset();
    void setAsDownloaded();
    void decodeVideoInfo(QJsonObject reply);

private:

    QString extractCode(QString link);

    QString title;
    QString link;
    QString code;
    QProcess *proc;
    QSettings *settings;
    QDateTime releaseDate;
    VideoStatus status;

    QNetworkAccessManager manager;
    bool haveBeenInitialised;
    bool isBeingInitialised;


};

#endif // VIDEO_H
