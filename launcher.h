#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QProcess>
#include <QThread>
#include <QPropertyAnimation>

#include "statuschecker.h"

#include "CommandResultType.h"
#include "networkmanager.h"
#include "filemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Launcher; }
QT_END_NAMESPACE

class Launcher : public QWidget
{
    Q_OBJECT
    
    constexpr static const quint16 C_MAIN_FORM_WIDTH = 150;
    
    constexpr static const quint8 C_MAX_PASSWORD_LENGTH = 20;
    
    constexpr static const char* C_MINECRAFT_VERSION = "1.12.2";
    
    constexpr static const quint16 C_MINECRAFT_INITIAL_WINDOW_WIDTH  = 800;
    constexpr static const quint16 C_MINECRAFT_INITIAL_WINDOW_HEIGHT = 600;
    
public:
    constexpr static const quint8 C_MAX_USERNAME_LENGTH = 20;
    
    Launcher(QWidget *parent = nullptr);
    ~Launcher();

signals:
    void changeDirectory(const QString newPath);
    void saveNewClientConfig(const QString username, bool isPremium, const QString version);
    void loadLocalSettings();                                                    // local client info loading...
    void getRemoteClientData();                                              // request an actual info about a client.
    void loginPremiumUser(const QString username, const QString password);
    void checkLocalFiles(bool isPremium = false);                            // client files checking...
    void downloadClient(const QString &downloadingPath);
    void unzipDownloadedClient();
    
public slots:
    void loginButtonSlot(); // checking for using a premium mod...
    void changeDirectoryButtonSlot();
    
    void loginDataReceived(const QString uuid, const QString accessToken);
    void remoteClientDataLoaded(const QString version, const QList<QString> remoteLibsList);
    void premiumCheckChange(const int);
    void launchGame();
    
    void blockingCommandProcessing(CommandResultType commandResultType);
    
    void initialSignals();
    void statusValueUpdate(const int, bool isInc = false);
    
    /*
    void processError(QProcess::ProcessError);
    void readyReadProcessError();
    void readyReadProcessInfo();
    void processStateChanged(QProcess::ProcessState);
    */
    
    void setLoadedLocalSettings(const QString &username, bool isPremium, const QString &version);
    
private:
    void paintEvent(QPaintEvent *) override;
    
    void reduceControlsBlockLevel();
    void increaseControlsBlockLevel(const quint8 = 1);
    
    void closeEvent(QCloseEvent *) override;
    
    Ui::Launcher *ui;
    
    quint8 m_controlsBlockLevel;
    
    QString                         m_uuid;
    QString                         m_accessToken;
    bool                            m_isPremium;
    QString                         m_version;
    QString                         m_remoteVersion;
    FileManager::ClientFilesConfig *m_clientFilesConfig;
    
    QLabel        *m_usernameLabel;
    QLineEdit     *m_usernameLineEdit;
    QLineEdit     *m_passwordLineEdit;
    QCheckBox     *m_premiumCheckBox;
    QPushButton   *m_loginButton;
    QPushButton   *m_changeDirectoryButton;
    StatusChecker *m_statusChecker;
    
    QPixmap m_backgroundImage;
    
    NetworkManager *m_networkManager;
    FileManager    *m_fileManager;
};
#endif // LAUNCHER_H
