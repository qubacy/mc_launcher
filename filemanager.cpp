#include "filemanager.h"
#include "launcher.h"

FileManager::FileManager(Launcher *launcher, ClientFilesConfig *clientFilesConfig, QObject *parent)
    : QObject(parent),
      m_launcher (launcher),
      m_clientFilesConfig (clientFilesConfig)
{
    
}

void FileManager::checkLocalFiles(bool isPremium)
{
    std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
    
    // checking local files...
    
    if (searchGameFiles() == 0) {
        emit blockingFileManipulationEnded(CommandResultType::CT_CHECK_FILES_NO_FILES);
        
        return;
    }
    
    if (isPremium) {
        // strict checking...
        
        QString minePath = m_clientFilesConfig->m_filesPart.m_minePath;
        
        foreach (const QString &libName, m_clientFilesConfig->m_filesPart.m_remoteFiles) {
            if (!m_clientFilesConfig->m_filesPart.m_localFiles.contains(minePath + libName)) {
                emit blockingFileManipulationEnded(CommandResultType::CT_CHECK_FILES_MISMATCH);
                
                return;
            }
        }
    }
    
    emit blockingFileManipulationEnded(CommandResultType::CT_CHECK_FILES_SUCCESS);
}

void FileManager::unzipDownloadedClient()
{
    std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
    
    if (FileManager::extract(m_clientFilesConfig->m_filesPart.m_minePath + '/' + sharedConsts::C_CLIENT_ZIP_FILENAME, m_clientFilesConfig->m_filesPart.m_minePath)) {
        emit blockingFileManipulationEnded(CommandResultType::CT_CLIENT_UNZIP_SUCCESS);
        
    } else {
        emit blockingFileManipulationEnded(CommandResultType::CT_CLIENT_UNZIP_ERROR);
    }
}

int FileManager::searchGameFiles()
{
    m_clientFilesConfig->m_filesPart.m_localFiles.clear();
    
    // searching for ALL .JAR files...
    
    QDir curDir {m_clientFilesConfig->m_filesPart.m_minePath};
    
    foreach (auto &entry, curDir.entryList(QDir::Filter::AllDirs)) {
        if (isLibsLocation(entry))
            recurseFilesSearch(m_clientFilesConfig->m_filesPart.m_minePath + '/' + entry);
    }
    
    qInfo() << "Scan is done!" << m_clientFilesConfig->m_filesPart.m_localFiles.length() << "entries are found!";

    return m_clientFilesConfig->m_filesPart.m_localFiles.length();
}

void FileManager::recurseFilesSearch(const QString &dirPath)
{
    QDir curDir {dirPath};
    
    foreach (const auto &entry, curDir.entryInfoList()) {   // QDir::NoDotAndDotDot has a bug!
        if (entry.isDir()) {
            
            if (entry.fileName() == "." || entry.fileName() == "..")
                continue;
            
                
            recurseFilesSearch(entry.filePath());
            
        } else {
            if (entry.suffix() == "jar")
                m_clientFilesConfig->m_filesPart.m_localFiles += entry.filePath();
        }
    }
}

bool FileManager::isLibsLocation(const QString &dirName)
{
    QStringList libsDirs {};
    
    libsDirs << "libraries"
             << "versions";
    
    if (libsDirs.contains(dirName))
        return true;
    
    return false;
}

bool FileManager::extract(const QString &filePath, const QString &extDirPath, const QString &singleFileName) {

    QZipReader zip_reader(filePath);
    
    if (zip_reader.exists()) {
        const int zipFilesCount   = zip_reader.count();
        const int statusChunkSize = zipFilesCount / 10;
        
        qDebug() << "Number of items in the zip archive =" << zipFilesCount;
        
        if (zip_reader.count() == 0)
            return false;
        
        int i {0};
        
        foreach (QZipReader::FileInfo info, zip_reader.fileInfoList()) {
            if(info.isFile)
                qDebug() << "File:" << info.filePath << info.size;
            else if (info.isDir)
                qDebug() << "Dir:" << info.filePath;
            else
                qDebug() << "SymLink:" << info.filePath;
            
            if ((++i) % statusChunkSize == 0)
                emit statusValueUpdate(static_cast<int>((i / statusChunkSize)) * 10);
        }
 
        zip_reader.extractAll(extDirPath);
        
        return true;
    }
    
    return false;
}

void FileManager::loadLocalSettings()
{
   QFile settingsFile(QDir::currentPath() + '/' + C_SETTINGS_FILENAME);
   
   if (!settingsFile.open(QFile::ReadOnly)) {
       emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL);
   
       return;
   }
   
   // getting username (20 bytes), isPremium (1 byte), version (till the end)...
    
   QString username = QString::fromUtf8(settingsFile.read(20)).remove(" ");
   
   if (settingsFile.error() == QFile::FileError::ReadError) {
       emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL);
       
       return;
   }
   
   bool isPremium = static_cast<bool>(settingsFile.read(1)[0]);
   
    if (settingsFile.error() == QFile::FileError::ReadError) {
       emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL);
       
       return;
   } 
    
    QString version = QString::fromUtf8(settingsFile.read(sharedConsts::C_MAX_VERSION_LENGTH)).remove(" ");
    
    if (settingsFile.error() == QFile::FileError::ReadError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL);
    
        return;
    }
    
    // mineDir loading...
    
    QString mineDir = QString::fromUtf8(settingsFile.readAll());
    
    {
        std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
        
        m_clientFilesConfig->m_filesPart.m_minePath = mineDir;
    }
    
    if (settingsFile.error() == QFile::FileError::ReadError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL);
    
        return;
    }
   
   emit localSettingsLoaded(username, isPremium, version);
}

void FileManager::changeDirectory(const QString newDir)
{
    std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
    
    m_clientFilesConfig->m_filesPart.m_minePath = newDir;
    
    emit blockingFileManipulationEnded(CommandResultType::CT_CLIENT_DIR_CHANGE_SUCCESS);
}

void FileManager::saveNewClientConfig(const QString username, bool isPremium, const QString version)
{
    QFile settingsFile(QDir::currentPath() + '/' + C_SETTINGS_FILENAME);
    
    if (!settingsFile.open(QFile::WriteOnly)) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL);
    
        return;
    }
    
    const int usernameRealLength = username.length();
    QString usernameToWrite = username;
    
    for (int i = usernameRealLength; i < Launcher::C_MAX_USERNAME_LENGTH; ++i)
        usernameToWrite += " ";
    
    settingsFile.write(usernameToWrite.toUtf8());
    
    if (settingsFile.error() == QFile::FileError::WriteError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL);
    
        return;
    }
    
    QByteArray isPremiumBytes;
    
    isPremiumBytes += static_cast<char>(isPremium);
    
    settingsFile.write(isPremiumBytes);
    
    if (settingsFile.error() == QFile::FileError::WriteError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL);
    
        return;
    }
    
    QString versionToWrite = version;
    
    for (int i = version.length(); i < sharedConsts::C_MAX_VERSION_LENGTH; ++i)
        versionToWrite += " ";
    
    settingsFile.write(versionToWrite.toUtf8());
    
    if (settingsFile.error() == QFile::FileError::WriteError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL);
    
        return;
    }
    
    // mineDir saving...
    
    QString mineDir;
    
    {
        std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
        
        mineDir = m_clientFilesConfig->m_filesPart.m_minePath;
    }
    
    settingsFile.write(mineDir.toUtf8());
    
    if (settingsFile.error() == QFile::FileError::WriteError) {
        emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL);
    
        return;
    }
    
    emit blockingFileManipulationEnded(CommandResultType::CT_LOCAL_SETTINGS_SAVING_SUCCESS);
}
