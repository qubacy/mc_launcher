#include "launcher.h"
#include "./ui_launcher.h"

Launcher::Launcher(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Launcher),
      m_networkManager {},
      m_isPremium {false},
      m_controlsBlockLevel {0}
{
    ui->setupUi(this);
    
    setWindowIcon(QIcon(":/img/icon.png"));
    
    m_usernameLabel = new QLabel(tr("&Username"));
    QLabel *passwordLabel = new QLabel(tr("&Password"));
    
    m_usernameLineEdit = new QLineEdit();
    m_passwordLineEdit = new QLineEdit();
    m_premiumCheckBox = new QCheckBox(tr("Premium mode"));
    
    m_usernameLabel->setBuddy(m_usernameLineEdit);
    passwordLabel->setBuddy(m_passwordLineEdit);
    m_usernameLabel->setObjectName("customUserInfoLabel");
    passwordLabel->setObjectName("customUserInfoLabel");
    
    m_usernameLineEdit->setObjectName("customInput");
    m_passwordLineEdit->setObjectName("customInput");
    m_usernameLineEdit->setMaxLength(C_MAX_USERNAME_LENGTH);
    m_passwordLineEdit->setMaxLength(C_MAX_PASSWORD_LENGTH);
    m_passwordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);
    m_usernameLineEdit->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_passwordLineEdit->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_premiumCheckBox->setChecked(true);
    
    m_loginButton = new QPushButton(tr("Login"));
    m_changeDirectoryButton = new QPushButton(tr("Change directory..."));
    
    m_loginButton->setCursor(Qt::CursorShape::PointingHandCursor);
    m_changeDirectoryButton->setCursor(Qt::CursorShape::PointingHandCursor);
    m_loginButton->setObjectName("customButton");
    m_changeDirectoryButton->setObjectName("customButton");
    
    // animations implementations...

    // do we need them???
    
    m_usernameLabel->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_usernameLineEdit->setMaximumWidth(C_MAIN_FORM_WIDTH);
    passwordLabel->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_passwordLineEdit->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_loginButton->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_changeDirectoryButton->setMaximumWidth(C_MAIN_FORM_WIDTH);
    m_premiumCheckBox->setMaximumWidth(C_MAIN_FORM_WIDTH);
    
    m_statusChecker = new StatusChecker();
    
    QFormLayout *formLayout = new QFormLayout();
    
    formLayout->addWidget(m_usernameLabel);
    formLayout->addWidget(m_usernameLineEdit);
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(m_passwordLineEdit);
    formLayout->addWidget(m_premiumCheckBox);
    formLayout->addWidget(m_loginButton);
    formLayout->addWidget(m_changeDirectoryButton);
    
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    mainLayout->addLayout(formLayout, 1);
    mainLayout->addWidget(m_statusChecker);
    
    // Other modules creation:
    
    m_clientFilesConfig = new FileManager::ClientFilesConfig();
    
    m_networkManager = new NetworkManager(this);
    m_fileManager    = new FileManager(this, m_clientFilesConfig);
    
    connect(m_loginButton,           &QPushButton::clicked,    this, &Launcher::loginButtonSlot);
    connect(m_premiumCheckBox,       &QCheckBox::stateChanged, this, &Launcher::premiumCheckChange);
    connect(m_changeDirectoryButton, &QPushButton::clicked,    this, &Launcher::changeDirectoryButtonSlot);
    
    
    connect(this, &Launcher::changeDirectory,       m_fileManager, &FileManager::changeDirectory,       Qt::QueuedConnection);
    connect(this, &Launcher::saveNewClientConfig,   m_fileManager, &FileManager::saveNewClientConfig,   Qt::QueuedConnection);
    connect(this, &Launcher::loadLocalSettings,     m_fileManager, &FileManager::loadLocalSettings,     Qt::QueuedConnection);
    connect(this, &Launcher::checkLocalFiles,       m_fileManager, &FileManager::checkLocalFiles,       Qt::QueuedConnection);
    connect(this, &Launcher::unzipDownloadedClient, m_fileManager, &FileManager::unzipDownloadedClient, Qt::QueuedConnection);
    
    connect(m_fileManager, &FileManager::localSettingsLoaded,           this, &Launcher::setLoadedLocalSettings,    Qt::QueuedConnection);
    connect(m_fileManager, &FileManager::blockingFileManipulationEnded, this, &Launcher::blockingCommandProcessing, Qt::QueuedConnection);
    connect(m_fileManager, &FileManager::statusValueUpdate,             this, &Launcher::statusValueUpdate,         Qt::QueuedConnection);
    
    
    connect(this, &Launcher::getRemoteClientData, m_networkManager, &NetworkManager::getRemoteClientData, Qt::QueuedConnection);
    connect(this, &Launcher::loginPremiumUser,    m_networkManager, &NetworkManager::loginPremiumUser,    Qt::QueuedConnection);
    connect(this, &Launcher::downloadClient,      m_networkManager, &NetworkManager::downloadClient,      Qt::QueuedConnection);
    
    connect(m_networkManager, &NetworkManager::remoteClientDataLoaded,      this, &Launcher::remoteClientDataLoaded,    Qt::QueuedConnection);
    connect(m_networkManager, &NetworkManager::loginDataReceived,           this, &Launcher::loginDataReceived,         Qt::QueuedConnection);
    connect(m_networkManager, &NetworkManager::blockingNetworkRequestEnded, this, &Launcher::blockingCommandProcessing, Qt::QueuedConnection);
    connect(m_networkManager, &NetworkManager::statusValueUpdate,           this, &Launcher::statusValueUpdate,         Qt::QueuedConnection);
    
    // moving other modules to their own threads...
    
    QThread *networkThread = new QThread();
    QThread *fileThread = new QThread();
    
    m_networkManager->moveToThread(networkThread);
    m_fileManager->moveToThread(fileThread);
    
    networkThread->start();
    fileThread->start();
    
    m_premiumCheckBox->setChecked(m_isPremium);
    
    QFile stylesFile(":/qss/styles.qss");
    
    stylesFile.open(QFile::ReadOnly);
    setStyleSheet(stylesFile.readAll());
    
    m_backgroundImage = QPixmap(":/img/background.jpg");
}

Launcher::~Launcher()
{
    delete ui;
}

void Launcher::loginButtonSlot()
{
    QString username = m_usernameLineEdit->text();
    QString password = m_passwordLineEdit->text();
    
    emit saveNewClientConfig(username, m_isPremium, m_version);
    
    if (username.length() == 0) {
        QMessageBox::warning(this, tr("Error"), tr("You have to provide a correct data!"));
        
        return;
    }
    
    if (m_isPremium) {
        if (password.length() == 0) {
            QMessageBox::warning(this, tr("Error"), tr("You have to provide a correct data!"));
            
            return;
        }
        
        if (m_remoteVersion.length() == 0) {
            QMessageBox::warning(this, tr("Premium mode error"), tr("Premium mode isn't available! Can't check an actual version!"));
            
            return;
        }
        
        if (m_version != m_remoteVersion) {
            QMessageBox versionDialog(QMessageBox::Icon::Warning, tr("Versions mismatch"), tr("Your client version mismatches the remote one. Do you want to continue?"));
            
            versionDialog.addButton(tr("Get update"), QMessageBox::ButtonRole::ActionRole);
            versionDialog.addButton(tr("Cancel"), QMessageBox::ButtonRole::RejectRole);
            versionDialog.addButton(tr("Continue"), QMessageBox::ButtonRole::AcceptRole);
            
            int dialogCode = versionDialog.exec();
            
            if (dialogCode == 0) {
                increaseControlsBlockLevel();
                
                std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
                
                emit downloadClient(m_clientFilesConfig->m_filesPart.m_minePath);
                
                return;
                
            } else if (dialogCode == 1)
                return;
        }
        
        // strict file checking:
        
        emit checkLocalFiles(true);
        
        increaseControlsBlockLevel();
        
        emit loginPremiumUser(username, password);
        
        // is this correct??
        m_statusChecker->setLabelText(tr("Signing in..."));
        
        increaseControlsBlockLevel();
        
        // status bar changing...
        
    } else {
        // weak file checking:
        
        // is this correct??
        m_statusChecker->setLabelText(tr("Local files checking..."));
        
        emit checkLocalFiles();
        
        increaseControlsBlockLevel();
    }
}

void Launcher::launchGame()
{
    QString username = m_usernameLineEdit->text();
    
    QProcess *clientProcess = new QProcess();
    
    QStringList launchingArgs;
    
    std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
    
    QString minePath = m_clientFilesConfig->m_filesPart.m_minePath;
    
    launchingArgs << "-Xmn128M"
                  << "-Xmx1024M"
                  << "-Djava.library.path=" + minePath + "/versions/ForgeOptiFine" + C_MINECRAFT_VERSION + "/natives"

                  << "-cp"
                  << m_clientFilesConfig->m_filesPart.m_localFiles.join(";")
                  << "-Dminecraft.applet.TargetDirectory=" + minePath
                  << "net.minecraft.launchwrapper.Launch"
                  << "--username"
                  << username
                  << "--uuid"
                  << m_uuid
                  << "--accessToken"
                  << m_accessToken
                  //<< "--version"
                  //<< QString(C_MINECRAFT_VERSION)
                  << "--gameDir"
                  << minePath
                  << "--assetsDir"
                  << minePath + "/assets"
                  << "--assetIndex" + QString(C_MINECRAFT_VERSION)
                  << "--userProperties []"
                  << "--userType legacy"
                  << "--tweakClass"
                  << "net.minecraftforge.fml.common.launcher.FMLTweaker"
                  << QString() + "-versionType Forge"
                  << "--width"
                  << QString::number(C_MINECRAFT_INITIAL_WINDOW_WIDTH)
                  << "--height"
                  << QString::number(C_MINECRAFT_INITIAL_WINDOW_HEIGHT);
    
    clientProcess->setArguments(launchingArgs);
    clientProcess->setProgram(minePath + "/jre8_portable/Java/bin/java.exe");
    
    /*
    connect(clientProcess, &QProcess::errorOccurred, this, &Launcher::processError);
    connect(clientProcess, &QProcess::stateChanged,  this, &Launcher::processStateChanged);
    connect(clientProcess, &QProcess::readyReadStandardError, this, &Launcher::readyReadProcessError);
    connect(clientProcess, &QProcess::readyReadStandardOutput, this, &Launcher::readyReadProcessInfo);
    */
    
    QProcess::startDetached(clientProcess->program(), launchingArgs);
    
    close();
}

void Launcher::reduceControlsBlockLevel()
{
    if (m_controlsBlockLevel < 1)
        return;
    
    m_controlsBlockLevel -= 1;
    
    if (m_controlsBlockLevel == 0) {
        m_loginButton->setEnabled(true);
        m_changeDirectoryButton->setEnabled(true);
        m_premiumCheckBox->setEnabled(true);
        m_usernameLineEdit->setEnabled(true);
        
        if (m_isPremium)
            m_passwordLineEdit->setEnabled(true);
    }
}

void Launcher::increaseControlsBlockLevel(const quint8 value)
{
    m_controlsBlockLevel += value;
    
    if (m_loginButton->isEnabled()) {
        m_loginButton->setEnabled(false);
        m_changeDirectoryButton->setEnabled(false);
        m_premiumCheckBox->setEnabled(false);
        m_usernameLineEdit->setEnabled(false);
        m_passwordLineEdit->setEnabled(false);
    }
}

void Launcher::blockingCommandProcessing(CommandResultType commandResultType)
{
    QString statusLabelText {""};
    
    m_statusChecker->setValue(0);
    
    qInfo() << commandResultType;
    
    switch (commandResultType) {
        case CommandResultType::CT_CHECK_FILES_SUCCESS: {
            if (!m_isPremium)
                launchGame();
        
            break;
        }
        case CommandResultType::CT_CHECK_FILES_MISMATCH: {
            if (m_isPremium)
                QMessageBox::warning(this, tr("Files mismatch"), tr("Remote list of files and the local one aren't equal!"));
        
            break;
        }
        case CommandResultType::CT_CHECK_FILES_NO_FILES: {
            if (QMessageBox::question(this, tr("No files"), tr("Looks like you don't have any .jar files at all. Do you want to download a client package?")) == QMessageBox::Yes) {
                increaseControlsBlockLevel(2);
                
                std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
                
                statusLabelText = tr("Client downloading...");
                
                emit downloadClient(m_clientFilesConfig->m_filesPart.m_minePath);
            }
        
            break;
        }
        case CommandResultType::CT_PREMIUM_LOGIN_SUCCESS: {
            launchGame();
            
            break;
        }
        case CommandResultType::CT_PREMIUM_LOGIN_FAIL: {
            QMessageBox::warning(this, tr("Authentication error"), tr("Provided user data isn't correct!"));
            
            break;
        }
        case CommandResultType::CT_CONNECTION_ERROR: {
            QMessageBox::warning(this, tr("Connection error"), tr("Cannot process a network request!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_UNZIP_ERROR: {
            QMessageBox::warning(this, tr("Unzipping error"), tr("Unable to unzip the downloaded client package!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_UNZIP_SUCCESS: {
            // is it correct?
        
            m_version = m_remoteVersion;
        
            QMessageBox::information(this, tr("Successful unzipping"), tr("Your client package was successfully unzipped!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_FILE_WRITING_ERROR: {
            QMessageBox::warning(this, tr("File writing error"), tr("Unable to write a client package file!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_DOWNLOADING_SUCCESS: {
            QMessageBox::information(this, tr("Downloading success"), tr("Client package was successfully downloaded!"));
        
            statusLabelText = tr("Client unzipping...");
            
            increaseControlsBlockLevel();  
            
            emit unzipDownloadedClient();
            
            break;
        }
        case CommandResultType::CT_CLIENT_DOWNLOADING_ERROR: {
            QMessageBox::warning(this, tr("File downloading error"), tr("Unable to download a client package file!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_GET_REMOTE_DATA_ERROR: {
            QMessageBox::warning(this, tr("Remote data error"), tr("Unable to get remote client data!"));
            
            break;
        }
        case CommandResultType::CT_CLIENT_GET_REMOTE_DATA_SUCCESS: {
            statusLabelText = tr("Remote client data was gotten");
            m_statusChecker->setValue(100);
        
            break;
        }
        case CommandResultType::CT_LOCAL_SETTINGS_LOADING_FAIL: {
            // nothing to do?    
        
            break;
        }
        case CommandResultType::CT_LOCAL_SETTINGS_LOADING_SUCCESS: {
            premiumCheckChange(m_isPremium);
            
            break;
        }
        case CommandResultType::CT_LOCAL_SETTINGS_SAVING_FAIL: {
            // nothing to do?
            
            break;
        }
        case CommandResultType::CT_LOCAL_SETTINGS_SAVING_SUCCESS: {
            // nothing to do?
            
            break;
        }
        case CommandResultType::CT_CLIENT_DIR_CHANGE_SUCCESS: {
            // nothing to do?
        
            break;
        }
        
    }
    
    m_statusChecker->setLabelText(statusLabelText);
    
    reduceControlsBlockLevel();
}

void Launcher::initialSignals()
{
    emit loadLocalSettings();
    emit getRemoteClientData();
    
    m_statusChecker->setLabelText(tr("Initialization..."));
}

void Launcher::statusValueUpdate(const int newValue, bool isInc)
{
    if (isInc)
        m_statusChecker->setValue(m_statusChecker->value() + newValue);
    else
        m_statusChecker->setValue(newValue);
}

void Launcher::setLoadedLocalSettings(const QString &username, bool isPremium, const QString &version)
{
    m_usernameLineEdit->setText(username);
    
    m_premiumCheckBox->setChecked(isPremium);
    
    m_version = version;
    
    blockingCommandProcessing(CommandResultType::CT_LOCAL_SETTINGS_LOADING_SUCCESS);
}

void Launcher::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    painter.drawPixmap(rect(), m_backgroundImage);
        
    // painting form frame:
    
    const int formPadding = 10;
    
    const int formYStart = m_usernameLabel->pos().y() - formPadding;
    const int formXStart = m_loginButton->pos().x() - formPadding;
    const int formYEnd   = m_changeDirectoryButton->pos().y() + m_changeDirectoryButton->height() + formPadding;
    const int formXEnd   = m_loginButton->pos().x() + m_loginButton->width() + formPadding;
    
    painter.setPen(QColor(0, 0, 0, 0));
    painter.setBrush(QBrush(QColor(138, 163, 79, 150)));
    painter.drawRect(QRect(QPoint(formXStart + 5, formYStart - 5), QPoint(formXEnd + 5, formYEnd - 5)));
    painter.setBrush(QBrush(QColor(160, 190, 92, 200)));
    painter.drawRect(QRect(QPoint(formXStart, formYStart), QPoint(formXEnd, formYEnd)));
}

void Launcher::closeEvent(QCloseEvent *)
{
    m_fileManager->thread()->quit();
    m_networkManager->thread()->quit();
}

void Launcher::premiumCheckChange(const int state)
{
    if (state > Qt::CheckState::Unchecked) {
        m_isPremium = true;
        m_passwordLineEdit->setEnabled(true);
        
    } else {
        m_isPremium = false;
        m_passwordLineEdit->setEnabled(false);
    }
}

void Launcher::changeDirectoryButtonSlot()
{
    QString mineDir = QFileDialog::getExistingDirectory(this, tr("Choose minecraft directory"));

    if (mineDir.isEmpty())
        return;
    
    emit changeDirectory(mineDir);
}

void Launcher::loginDataReceived(const QString uuid, const QString accessToken)
{
    if (uuid.length() <= 0 || accessToken.length() <= 0) {
        blockingCommandProcessing(CommandResultType::CT_PREMIUM_LOGIN_FAIL);
        
        return;
    }
    
    m_uuid        = uuid;
    m_accessToken = accessToken;
    
    blockingCommandProcessing(CommandResultType::CT_PREMIUM_LOGIN_SUCCESS);
}

void Launcher::remoteClientDataLoaded(const QString version, const QList<QString> remoteLibsList)
{
    m_remoteVersion = version;
    
    std::unique_lock<std::mutex> locker(m_clientFilesConfig->m_mutex);
    
    m_clientFilesConfig->m_filesPart.m_remoteFiles = remoteLibsList;
    
    blockingCommandProcessing(CommandResultType::CT_CLIENT_GET_REMOTE_DATA_SUCCESS);
}

/*
void Launcher::processError(QProcess::ProcessError errorCode)
{
    qInfo() << "Error occured! Code:" << errorCode;
}

void Launcher::readyReadProcessError()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    
    qInfo() << "Read standard error of proc:" << proc->readAllStandardError();
}

void Launcher::readyReadProcessInfo()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    
    qInfo() << "Read standard error of proc:" << proc->readAllStandardOutput();
}

void Launcher::processStateChanged(QProcess::ProcessState state)
{
    qInfo() << "Process changed state to" << state;
}
*/
