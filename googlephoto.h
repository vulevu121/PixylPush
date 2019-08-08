#ifndef GOOGLEPHOTO_H
#define GOOGLEPHOTO_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QFile>
#include <QUrl>
#include "googleoauth2.h"

class GooglePhoto : public QObject
{
    Q_OBJECT
public:
    explicit GooglePhoto(QObject *parent = nullptr);

private:
    QNetworkAccessManager *manager = nullptr;
    GoogleOAuth2 auth;
    QString accessToken;
    QString uploadToken;
    QString uploadedPhotoURL;
    QString albumName;
    QString albumID;
    QString albumDescription;
    QString albumURL;
    QString pathToFile;
    QString fileName;
    QStringList uploadTokenList;
    bool Uploading = false;
    bool albumReady = false;


signals:
    void authenticated();
    void uploadTokenReceived(QString);
    void albumCreated();
    void albumShared(QString);
    void albumIdChanged();
    void mediaCreated(QString);
    void pathToFileChanged(QString);
    void showMessage(QString const &msg);

private slots:
    void SetAccessToken(QString token);
    void UploadPicData(QString);
    void UploadReply(QNetworkReply *reply);
    void CreateAlbum();
    void CreateAlbumReply(QNetworkReply * reply);
    void ShareAlbum();
    void ShareAlbumReply(QNetworkReply * reply);
    void CreateMediaInAlbum(QString);
    void CreateMediaReply(QNetworkReply *reply);
    void GetAlbums();
    void GetAlbumsReply(QNetworkReply * reply);
    void AppendUploadTokenList(QString);


public slots:
    /* If album already exists, this function will set the target album for all uploads */
    void SetAlbumDescription(QString note);
    void SetPathToFile(QString path);
    void UploadPhoto(QString pathToPic);
    void SetAlbumName(QString name);
    bool isUploading();
    bool isAlbumReady();
    void CreateMultipleMediaInAlbum();
    void SetTargetAlbumToUpload(QString id);

    /* Use for testing oauth2 only */
    void Reauthenticate();

    QString GetAlbumName();
    QString GetAlbumURL();
    QString GetUploadedPhotoURL();
    /* This requires a different scope. Need to authenticate again*/
    QString GetAlbumID();
};

#endif // GOOGLEPHOTO_H
