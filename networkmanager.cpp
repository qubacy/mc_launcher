#include "networkmanager.h"

NetworkManager::NetworkManager(Launcher *launcher, QObject *parent)
    : QNetworkAccessManager(parent),
      m_launcher (launcher)
{
    
}

void NetworkManager::getRemoteClientData()
{
    QUrl endpointUrl(C_HOST_ENDPOINTS + QString("client/getConfig"));
    QNetworkRequest updateRequest(endpointUrl);

    QNetworkReply *curReply = get(updateRequest);
    
    QEventLoop eventLoop;
    
    connect(curReply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    
    eventLoop.exec();
    
    QString rawClientData = QString::fromUtf8(curReply->readAll());
    
    // json parsing...
    
    int versionPos = rawClientData.indexOf("\"version\":");
    int libsPos    = rawClientData.indexOf("\"libs\":\[");
    
    if (versionPos == -1 || libsPos == -1 || rawClientData.length() == 0) {
        emit blockingNetworkRequestEnded(CommandResultType::CT_CLIENT_GET_REMOTE_DATA_ERROR);
    
        return;
    }
    
    QString version;
    
    for (int i = versionPos + strlen("\"version\":\""); i < rawClientData.length(); ++i) {
        if (rawClientData.at(i) == '\"')
            break;
        
        version += rawClientData.at(i);
    }
    
    QString libsRow;
    
    for (int i = libsPos + strlen("\"libs\":["); i < rawClientData.length(); ++i) {
        if (rawClientData.at(i) == ']')
            break;
        
        libsRow += rawClientData.at(i);
    }
    
    emit remoteClientDataLoaded(version, libsRow.remove('\"').split(','));
}

void NetworkManager::loginPremiumUser(const QString username, const QString password)
{
    QUrl endpointUrl(C_HOST_ENDPOINTS + QString("auth/launcher"));
    QNetworkRequest postRequest(endpointUrl);
    
    postRequest.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    
    QString requestBody = "{\"method\":\"auth\",\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";

    QNetworkReply *curReply = post(postRequest, requestBody.toUtf8());
    
    QEventLoop eventLoop;
    
    connect(curReply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    
    eventLoop.exec();
    
    if (!curReply) {
        emit blockingNetworkRequestEnded(CommandResultType::CT_CONNECTION_ERROR);
        
        return;
    }
    
    QString replyString = QString::fromUtf8(curReply->readAll());
    
    if (!replyString.contains("OK")) {
        emit loginDataReceived("", "");
        
    } else {
        QStringList replyStringList = replyString.split(":");
        
        if (replyStringList.length() < 4) {
            emit loginDataReceived("", "");
            
            return;
        }
        
        emit loginDataReceived(replyStringList[2], replyStringList[3]);
    }
}

void NetworkManager::newDownloadingChunkProcessing()
{
    QNetworkReply *curReply = qobject_cast<QNetworkReply*>(sender());
    
    m_downloadingBuffer += curReply->readAll();
    
    emit statusValueUpdate(10);
}

void NetworkManager::downloadClient(const QString downloadingPath)
{
    QUrl endpointUrl(C_HOST_ENDPOINTS + QString("client/download"));
    QNetworkRequest downloadRequest(endpointUrl);
    
    QNetworkReply *curReply = get(downloadRequest);
    
    connect(curReply, &QNetworkReply::readyRead, this, &NetworkManager::newDownloadingChunkProcessing);

    QEventLoop eventLoop;
    
    connect(curReply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    
    eventLoop.exec();
    
    if (m_downloadingBuffer.length() <= 0) {
        emit blockingNetworkRequestEnded(CommandResultType::CT_CLIENT_DOWNLOADING_ERROR);
        
        return;
    }
    
    QFile clientZip(downloadingPath + '/' + sharedConsts::C_CLIENT_ZIP_FILENAME);
    
    if (clientZip.open(QFile::WriteOnly)) {
        clientZip.write(m_downloadingBuffer);
        
        emit blockingNetworkRequestEnded(CommandResultType::CT_CLIENT_DOWNLOADING_SUCCESS);
        
        clientZip.close();
       
    } else
        emit blockingNetworkRequestEnded(CommandResultType::CT_CLIENT_FILE_WRITING_ERROR);
    
    m_downloadingBuffer.clear();
}
