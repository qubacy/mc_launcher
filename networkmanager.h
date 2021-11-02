#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFile>

#include "CommandResultType.h"
#include "sharedconsts.h"

class Launcher;

class NetworkManager : public QNetworkAccessManager     
{
    Q_OBJECT
    
    constexpr static const char* C_HOST_ENDPOINTS  = "http://127.0.0.1/";
    
public:
    explicit NetworkManager(Launcher *launcher, QObject *parent = nullptr);
    
signals:
    void remoteClientDataLoaded(const QString version, const QList<QString> remoteLibsList);
    void loginDataReceived(const QString uuid, const QString accessToken);
    
    void blockingNetworkRequestEnded(CommandResultType commandResultType);
    
    void statusValueUpdate(const int, bool isInc = false);
    
public slots:
    void getRemoteClientData();
    void loginPremiumUser(const QString username, const QString password);
    void downloadClient(const QString downloadingPath);
    
private slots:
    void newDownloadingChunkProcessing();
    
private:
    QByteArray m_downloadingBuffer;
    
    Launcher *m_launcher;
};

//#include "launcher.h"

#endif // NETWORKMANAGER_H
