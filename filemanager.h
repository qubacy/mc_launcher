#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QDir>
#include <QtGui/private/qzipreader_p.h>
#include <QFile>
#include <mutex>

#include "CommandResultType.h"
#include "sharedconsts.h"

class Launcher;

class FileManager : public QObject
{
    Q_OBJECT
    
    constexpr static const char* C_SETTINGS_FILENAME   = "settings.dat";
    
public:
    struct ClientFilesConfig {
        struct FilesPart {
            QList<QString> m_localFiles;
            QList<QString> m_remoteFiles;
            QString        m_minePath;
            
            FilesPart() 
                : m_localFiles {},
                  m_remoteFiles {},
                  m_minePath {QDir::currentPath()}
            { }
            
        } m_filesPart;
        
        std::mutex m_mutex;
        
    };
    
    explicit FileManager(Launcher *launcher, ClientFilesConfig *clientFilesConfig, QObject *parent = nullptr);

signals:
    void localSettingsLoaded(const QString username, bool isPremium, const QString version);
    
    void blockingFileManipulationEnded(CommandResultType commandResultType);
    
    void statusValueUpdate(const int, bool isInc = false);
    
public slots:
    void loadLocalSettings();
    void changeDirectory(const QString newDir);
    void saveNewClientConfig(const QString username, bool isPremium, const QString version);
    void checkLocalFiles(bool isPremium = false);
    void unzipDownloadedClient();
    
private:
    bool extract(const QString &, const QString &, const QString & = "");
    int  searchGameFiles();
    void recurseFilesSearch(const QString &);
    bool isLibsLocation(const QString &);

    ClientFilesConfig *m_clientFilesConfig;
    
    Launcher *m_launcher;
};

//#include "launcher.h"

#endif // FILEMANAGER_H
