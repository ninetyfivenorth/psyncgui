#include "cryptopage.h"
#include "pcloudwindow.h"
#include "ui_pcloudwindow.h"
#include "pcloudapp.h"
#include "common.h"
#include "psynclib.h"
#include <QDateTime>
#include <QUrl>
#include <QDesktopServices>
#include <QPalette>

CryptoPage::CryptoPage(PCloudWindow *w, PCloudApp *a,QObject *parent) :
    QObject(parent)
{
    this->app = a;
    this->win = w;
    passStrenth = -1;
    requestCryptoKeyDialog = NULL;

    //welcome page
    win->ui->labelCryptoWlcmInfo1->setFont(app->bigger3pFont);
    win->ui->labelCryptoWlcmInfo2->setFont(app->bigger1pFont);
    win->ui->btnCryptoBuy->setStyleSheet("QPushButton {background-color: #83c41a; color: white; border-radius:3px; border:1x solid #83c41a;}");
    win->ui->btnCryptoBuy->setMaximumHeight(win->ui->btnCryptoTryTrial->height());

    //setuppage
    win->ui->labelCryptoSetup->setFont(app->bigger3pFont);
    win->ui->progressBarCryptoPass->setMinimum(0);
    win->ui->progressBarCryptoPass->setMaximum(6);
    win->ui->labelCryptoPassStrenth->setFont(app->smaller1pFont);

    //main page
    int maxbtnw = qMax(win->ui->btnCryptoOpenFldr->width(), qMax(win->ui->btnCryptoManageFldr->width(), win->ui->btnCryptoMainPagePay->width()));
    win->ui->btnCryptoOpenFldr->setMinimumWidth(maxbtnw);
    win->ui->btnCryptoManageFldr->setMinimumWidth(maxbtnw);
    win->ui->btnCryptoMainPagePay->setMinimumWidth(maxbtnw);

    win->ui->labelWhatisCrFldr->setFont(app->bigger1pFont);
    win->ui->tbtnMoreInfo->setStyleSheet("QToolButton{background-color:transparent; text-decoration: underline; color:#17BED0}"
                                         "QToolButton:hover{text-decoration: underline; background-color: transparent;}");

    QFont boldfont;
    boldfont.setBold(true);
    win->ui->labelCryptoMainPayInfo->setFont(boldfont);

#ifdef Q_OS_LINUX
    win->ui->pagedWidgetCrypto->setMinimumHeight(400);
#else
    win->ui->labelCryptoWelcmPic->setPixmap(QPixmap(":/crypto/images/crypto/cryptoWelcomePageXP.png"));
    win->ui->labelCryptoMainFldrPic->setPixmap(QPixmap(":/crypto/images/crypto/cryptoMainPageXP.png"));
    win->ui->progressBarCryptoPass->setMaximumHeight(10);
    win->ui->pagedWidgetCrypto->setMaximumHeight(350);
#endif
    connect(win->ui->pagedWidgetCrypto, SIGNAL(currentChanged(int)), this, SLOT(autoResize()));
    connect(win->ui->lineEditCryptoPass, SIGNAL(textChanged(QString)), this, SLOT(setProgressBar()));
    connect(win->ui->lineEditCryptoPass2, SIGNAL(textChanged(QString)), this, SLOT(checkPasswordsMatch()));
    connect(win->ui->btnCryptoTryTrial, SIGNAL(clicked()), this, SLOT(tryTrial()));
    connect(win->ui->btnCryptoBuy, SIGNAL(clicked()), this, SLOT(buyCrypto()));
    connect(win->ui->btnCryptoMoreInfo, SIGNAL(clicked()), this, SLOT(getMoreCryptoInfo()));
    connect(win->ui->btnCryptoCreateKey, SIGNAL(clicked()), this, SLOT(setupCrypto()));
    connect(win->ui->btnCryptoManageFldr, SIGNAL(clicked()), this, SLOT(manageCryptoFldr()));
    connect(win->ui->btnCryptoOpenFldr, SIGNAL(clicked()), this, SLOT(openCryptoFldr()));
    connect(win->ui->btnCryptoMainPagePay, SIGNAL(clicked()), this, SLOT(buyCrypto()));
    connect(win->ui->tbtnMoreInfo, SIGNAL(clicked()), this, SLOT(getMoreCryptoInfo()));

}

void CryptoPage::initCryptoPage() //called when user has just loggedin
{
    tryTrialClickedFlag = false;
    setCurrentPageIndex();
    if (this->pageIndex == 2 && app->settings->value("autostartcrypto").toBool() && !app->settings->value("showintrowin").toBool())
        QTimer::singleShot(3000, this, SLOT(requestCryptoKey()));
    this->autoResize();
}

void CryptoPage::showEventCrypto()
{    
    setCurrentPageIndex();
    //qDebug()<<this->pageIndex<<win->ui->pagedWidgetCrypto->height();
}

void CryptoPage::setCurrentPageIndex()
{
    qDebug() << " CryptoPage::setCurrentPageIndex" << psync_crypto_isexpired() <<psync_crypto_expires()<< psync_crypto_issetup()
             << psync_crypto_isstarted() <<psync_crypto_hassubscription();

    uint subscbtntTime = psync_crypto_expires();
    qDebug()<<QDateTime::fromTime_t(subscbtntTime);
    if (!tryTrialClickedFlag &&                                                                         //for case when entered pass but hasn't already setup and went to another flag
            (!subscbtntTime ||                                                                           // 1. trial
             (QDateTime::fromTime_t(subscbtntTime).addDays(30) < QDateTime::currentDateTime()) ||        // 2. in active subscription or 30 days read only mode
             (QDateTime::fromTime_t(subscbtntTime).addDays(30) > QDateTime::currentDateTime() &&         // 3. resetted crypto in read-only mode
              QDateTime::fromTime_t(subscbtntTime) < QDateTime::currentDateTime() && !psync_crypto_issetup())))

    {
        if(!subscbtntTime)
        {
            win->ui->btnCryptoTryTrial->setVisible(true);
            win->ui->label_trialdays->setVisible(true);
        }
        else
        {
            win->ui->btnCryptoTryTrial->setVisible(false);
            win->ui->label_trialdays->setVisible(false);
        }

        app->isCryptoExpired = true;
        this->pageIndex = 0;  //show welcome crypto page
    }
    else if (psync_crypto_issetup() == 0)  //show setup pass page
    {
        if(win->ui->lineEditCryptoPass->text().isEmpty())
        {
            win->ui->progressBarCryptoPass->setVisible(false);
            win->ui->labelCryptoPassStrenth->setText("");
            win->ui->lineEditCryptoPass->setFocus();
        }
        else
            win->ui->progressBarCryptoPass->setVisible(true);

        app->isCryptoExpired = true;
        this->pageIndex = 1; //show welcome crypto page
    }
    else //show main crypto page
    {
        if(psync_crypto_isstarted())
            setUnlockedFldrUI();
        else
            setLockedFldrUI();
        setTrialUI(psync_crypto_hassubscription(), subscbtntTime);

        app->isCryptoExpired = false;
        this->pageIndex = 2;
    }
    win->ui->pagedWidgetCrypto->setCurrentIndex(pageIndex);
}

void CryptoPage::autoResize()
{
    // auto resize
    for(int i = 0; i < win->ui->pagedWidgetCrypto->count(); i++)
    {
        if (i != this->pageIndex)
            win->ui->pagedWidgetCrypto->widget(i)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    }
    win->ui->pagedWidgetCrypto->widget(this->pageIndex)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    win->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    win->updateGeometry();
}

int CryptoPage::getCurrentCryptoPageIndex()
{
    return this->pageIndex;
}

void CryptoPage::setTrialUI(bool hasSubscriptoin, uint expTime)
{
    QDateTime expDtTime =  QDateTime::fromTime_t(expTime);
    int daysLeft = QDateTime::currentDateTime().daysTo(expDtTime);
    if(daysLeft>1)
        win->ui->labelCryptoMainPayInfo->setText(QString("pCloud Crypto expires in %1 days").arg(daysLeft));
    else if(!daysLeft)
        win->ui->labelCryptoMainPayInfo->setText(QString("pCloud Crypto expires today."));
    else
        win->ui->labelCryptoMainPayInfo->setText(QString("pCloud Crypto has expired."));

    if (!hasSubscriptoin) // trial
    {
        win->ui->btnCryptoMainPagePay->setText("  Buy Now  ");
        win->ui->labelCryptoPrice->setVisible(true);
    }
    else
    {
        win->ui->btnCryptoMainPagePay->setText(" Change Subscription ");
        win->ui->labelCryptoPrice->setVisible(false);
    }
}

// slots
void CryptoPage::setUnlockedFldrUI()
{
    win->ui->btnCryptoManageFldr->setText("  Lock  ");
    win->ui->btnCryptoManageFldr->setIcon(QIcon(":/crypto/images/crypto/crypto.png"));
    win->ui->label_cryptoStatus->setText("Crypto Folder is accessible");
    win->ui->labelCryptoFldrInfo->setText("Lock the Crypto Folder to keep your files protected.");
    win->ui->btnCryptoOpenFldr->setEnabled(true);
}

void CryptoPage::setLockedFldrUI()
{
    win->ui->btnCryptoManageFldr->setText("  Unlock  ");
    win->ui->btnCryptoManageFldr->setIcon(QIcon(":/crypto/images/crypto/crypto-unlck.png"));
    win->ui->label_cryptoStatus->setText("Crypto Folder is <b>not</b> accessible");
    win->ui->labelCryptoFldrInfo->setText("Unlock the Crypto Folder to decrypt and manage your files.");
    win->ui->btnCryptoOpenFldr->setEnabled(false);
}

void CryptoPage::clearSetupUI()
{
    if(!win->ui->lineEditCryptoPass->text().isEmpty())
        win->ui->lineEditCryptoPass->clear();
    win->ui->lineEditCryptoPass->setFocus();
    if(!win->ui->lineEditCryptoPass2->text().isEmpty())
        win->ui->lineEditCryptoPass2->clear();
    if(!win->ui->lineEditCryptoHint->text().isEmpty())
        win->ui->lineEditCryptoHint->clear();
    win->ui->progressBarCryptoPass->setVisible(false);
    win->ui->labelCryptoPassStrenth->clear();
    win->ui->label_passMatchPic->setPixmap(QPixmap(":/crypto/images/crypto/matchNo.png"));
}

void CryptoPage::setProgressBar()
{
    QString pass = win->ui->lineEditCryptoPass->text();
    if(!pass.isEmpty())
    {
        win->ui->progressBarCryptoPass->setVisible(true);
        passStrenth = psync_password_quality(pass.toUtf8());
        QPalette paletteLabel;
        switch(passStrenth)
        {
        case 0:
            win->ui->progressBarCryptoPass->setValue(2);
            win->ui->progressBarCryptoPass->setStyleSheet("QProgressBar:chunk{background-color:#FF4D4D; width: 15px; margin: 0.5px;}");
            paletteLabel.setColor(QPalette::WindowText, QColor("#FF4D4D"));
            win->ui->labelCryptoPassStrenth->setText("Weak");
            break;
        case 1:
            win->ui->progressBarCryptoPass->setValue(4);
            win->ui->progressBarCryptoPass->setStyleSheet("QProgressBar:chunk{background-color:#FF9326; width: 15px; margin: 0.5px;}");
            paletteLabel.setColor(QPalette::WindowText, QColor("#FF9326"));
            win->ui->labelCryptoPassStrenth->setText("Medium");
            break;
        case 2:
            win->ui->progressBarCryptoPass->setValue(6);
            win->ui->progressBarCryptoPass->setStyleSheet("QProgressBar:chunk{background-color:#83C100; width: 15px; margin: 0.5px;}");
            paletteLabel.setColor(QPalette::WindowText, QColor("#83C100"));
            win->ui->labelCryptoPassStrenth->setText("Strong");
            break;
        default:
            break;
        }
        win->ui->labelCryptoPassStrenth->setPalette(paletteLabel);
        if(!win->ui->lineEditCryptoPass2->text().isEmpty())
            checkPasswordsMatch();
    }
    else
    {
        win->ui->progressBarCryptoPass->setVisible(false);
        win->ui->labelCryptoPassStrenth->setText("");
    }
}

void CryptoPage::checkPasswordsMatch()
{
    if ((win->ui->lineEditCryptoPass->text() == win->ui->lineEditCryptoPass2->text()) && !win->ui->lineEditCryptoPass->text().isEmpty())
        win->ui->label_passMatchPic->setPixmap(QPixmap(":/crypto/images/crypto/matchYes.png"));
    else
        win->ui->label_passMatchPic->setPixmap(QPixmap(":/crypto/images/crypto/matchNo.png"));
}

void CryptoPage::tryTrial()
{
    if(!app->isVerified)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(win,"Your Account is not Verified", "This functionality is available for verified accounts only.\nDo you want to verify your account now?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
            win->verifyEmail();
        return;
    }

    this->pageIndex = 1;
    win->ui->progressBarCryptoPass->setVisible(false);
    win->ui->labelCryptoPassStrenth->setText("");
    win->ui->pagedWidgetCrypto->setCurrentIndex(this->pageIndex);
    tryTrialClickedFlag = true;
}

void CryptoPage::buyCrypto()
{    
    QUrl url(QString("https://www.pcloud.com/payment.html?product=crypto#authtoken=").append(psync_get_auth_string()));
    QDesktopServices::openUrl(url);
}

void CryptoPage::getMoreCryptoInfo()
{
    QUrl url("https://www.pcloud.com/crypto");
    QDesktopServices::openUrl(url);
}

void CryptoPage::setupCrypto()
{
    if(app->nointernetFlag)
    {
        QMessageBox::critical(win, "pCloud","No Internet Connection!");
        return;
    }

    if(win->ui->lineEditCryptoPass->text().isEmpty() || win->ui->lineEditCryptoPass2->text().isEmpty())
    {
        QMessageBox::critical(win, "pCloud","Please enter passphrase in both fields!");
        return;
    }

    if(win->ui->lineEditCryptoPass->text() != win->ui->lineEditCryptoPass2->text())
    {
        QMessageBox::critical(win, "Error","The two Passphrases should match!");
        return;
    }

    if(passStrenth < 1)
    {
        QMessageBox::critical(win, "Passphrase not strong enough","Your Passphrase is too weak. You need to provide a sufficiant Passphrase!");
        return;
    }

    if(win->ui->lineEditCryptoHint->text().contains(win->ui->lineEditCryptoPass->text()))
    {
        QMessageBox::critical(win,"Compromising Hint","Your Passphrase Hint must not contain parts of or the entire Passphrase!");
        return;
    }

    if(win->ui->lineEditCryptoHint->text().length() > 141)
    {
        QMessageBox::critical(win,"Hint Too Long", "The Passphrase Hint must not exceed 140 characters!");
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("pCloud Crypto");
    msgBox.setText("Did you memorize your Passphrase? Once forgotten it cannot be restored!");
    msgBox.setIcon(QMessageBox::Warning);
    QPushButton *okBtn = msgBox.addButton(trUtf8("I got it"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(okBtn);
    msgBox.setStandardButtons(QMessageBox::Cancel);
    if (msgBox.exec() != QMessageBox::AcceptRole)
        return;

    QDir localDir(QString(psync_fs_getmountpoint()).append(OSPathSeparator).append("Crypto Folder")); // temp hardcode according to the specification (pfs-13)
    if(localDir.exists())
    {
        QMessageBox::critical(win,"Folder with the name \"Crypto Folder\" already exists",
                              "pCloud Drive was unable to create the Crypto Folder because there is another folder with the same name. You need to rename it and try again.");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int resSetup = psync_crypto_setup(win->ui->lineEditCryptoPass->text().toUtf8(),win->ui->lineEditCryptoHint->text().toUtf8());
    if (resSetup == PSYNC_CRYPTO_SETUP_SUCCESS || resSetup == PSYNC_CRYPTO_SETUP_ALREADY_SETUP)
    {
        qDebug()<< "CRYPTO: setted up successfully or already setted up";

        int resCryptoStart = psync_crypto_start(win->ui->lineEditCryptoPass->text().toUtf8());
        if(!resCryptoStart || resCryptoStart == PSYNC_CRYPTO_START_ALREADY_STARTED)
        {
            qDebug()<< "CRYPTO: started successfully or already started";

            const char *err = NULL;
            int mkDirRes = psync_crypto_mkdir(0,"Crypto Folder", &err, NULL);
            if (!mkDirRes && err == NULL)
            {
                qDebug()<<"CRYPTO: crypto dir created successfully";
                this->showEventCrypto();
                if(tryTrialClickedFlag)
                    tryTrialClickedFlag = false;
            }
            else
            {
                QApplication::restoreOverrideCursor();
                this->showMkDirError(mkDirRes,err);
                return;
            }

            win->ui->lineEditCryptoHint->clear();
            win->ui->lineEditCryptoPass->clear();
            win->ui->lineEditCryptoPass2->clear();
            win->ui->label_passMatchPic->setPixmap(QPixmap(":/crypto/images/crypto/matchNo.png"));
            QApplication::restoreOverrideCursor();
        }
        else
        {
            QApplication::restoreOverrideCursor();
            showStartCryptoError(resCryptoStart);
            return;
        }
    }
    else
    {
        QApplication::restoreOverrideCursor();
        showSetupCryptoError(resSetup);
    }
}

void CryptoPage::manageCryptoFldr()
{
    if(psync_crypto_isstarted())
        lock();
    else
        unlock();
}

void CryptoPage::lock()
{
    int resCryptoManageFLdr = psync_crypto_stop();
    if (!resCryptoManageFLdr)
        this->setLockedFldrUI();
    else
        this->showStopCryptoError(resCryptoManageFLdr);
}

void CryptoPage::unlock()
{
    emit this->requestCryptoKey();
}

void CryptoPage::requestCryptoKey()
{       
    if(requestCryptoKeyDialog == NULL)
    {
        requestCryptoKeyDialog = new CryptoKeyDialog(this);
        app->setActiveWindow(requestCryptoKeyDialog);
    }
    else
    {
        requestCryptoKeyDialog->show();
        app->setActiveWindow(requestCryptoKeyDialog);
        return;
    }
    if (requestCryptoKeyDialog->exec() == QDialog::Accepted) // also starts the crypto if pass is ok
    {
        emit this->setUnlockedFldrUI();
        emit this->openCryptoFldr();
    }
    requestCryptoKeyDialog = NULL;
}

void CryptoPage::openCryptoFldr()
{
    psync_folderid_t cryptoFldrId = psync_crypto_folderid();
    if(cryptoFldrId != PSYNC_CRYPTO_INVALID_FOLDERID)
    {
        char *path = psync_fs_get_path_by_folderid(cryptoFldrId);
        qDebug()<<"CRYPTO: openCryptoFldr"<<path<<cryptoFldrId;

        if(path != NULL)
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            free(path);
        }
        else
            QMessageBox::critical(NULL,"Crypto Folder Not Found",
                                  "The Crypto Folder was either moved or deleted. You can either move it back, restore it from your Trash or re-create it by resetting the Passphrase from pCloud Drive's settings.");

    }
    else
        QMessageBox::critical(NULL,"Crypto Folder Not Found",
                              "The Crypto Folder was either moved or deleted. You can either move it back, restore it from your Trash or re-create it by resetting the Passphrase from pCloud Drive's settings.");
}


// show crypto errors
void CryptoPage::showSetupCryptoError(int setupRes)
{
    qDebug()<<"CRYPTO: setup error = "<<setupRes;
    switch(setupRes)
    {
    case PSYNC_CRYPTO_SETUP_NOT_SUPPORTED:
        QMessageBox::critical(win,"Crypto Error", "Not supported.");
        break;
    case PSYNC_CRYPTO_SETUP_KEYGEN_FAILED:
        QMessageBox::critical(win,"Crypto Error", "Keygen failed.");
        break;
    case PSYNC_CRYPTO_SETUP_CANT_CONNECT:
        QMessageBox::critical(win,"Crypto Error", "Unable to connect to server.");
        break;
    case PSYNC_CRYPTO_SETUP_NOT_LOGGED_IN:
        QMessageBox::critical(win,"Crypto Error", "Your are not logged in!");
        if(app->isLogedIn())
            emit app->logOut();
        break;
    case PSYNC_CRYPTO_SETUP_UNKNOWN_ERROR:
        QMessageBox::critical(win,"Crypto Error", "Unknown error.");
        break;
    default:
        break;
    }
}

void CryptoPage::showStartCryptoError(int startRes)
{
    qDebug()<<"CRYPTO: startres "<<startRes;
    switch(startRes)
    {
    case PSYNC_CRYPTO_START_NOT_SUPPORTED:
        QMessageBox::critical(win,"Crypto Error", "Not supported.");
        break;
    case PSYNC_CRYPTO_START_CANT_CONNECT:
        QMessageBox::critical(win,"Crypto Error", "Unable to connect to server.");
        break;
    case PSYNC_CRYPTO_START_NOT_LOGGED_IN:
        QMessageBox::critical(win,"Crypto Error", "Your are not logged in!");
        if(app->isLogedIn())
            emit app->logOut();
        break;
    case PSYNC_CRYPTO_START_NOT_SETUP:
        QMessageBox::critical(win,"Crypto Error", "The Crypto Folder is not setted up.");
        break;
    case PSYNC_CRYPTO_START_UNKNOWN_KEY_FORMAT:
        QMessageBox::critical(win,"Crypto Error", "Unknown Passphrase format!");
        break;
    case PSYNC_CRYPTO_START_BAD_PASSWORD:
        QMessageBox::critical(win,"Crypto Error", "Incorrect Passphrase!");
        break;
    case PSYNC_CRYPTO_START_KEYS_DONT_MATCH:
        QMessageBox::critical(win,"Crypto Error", "The two Passphrase should match.");
        break;
    case PSYNC_CRYPTO_START_UNKNOWN_ERROR:
        QMessageBox::critical(win,"Crypto Error", "Unknown error.");
        break;
    default:
        break;
    }
}

void CryptoPage::showMkDirError(int mkdirRes, const char* err)
{
    qDebug()<<"CRYPTO: mkdir "<<mkdirRes;
    switch(mkdirRes)
    {
    case PSYNC_CRYPTO_NOT_STARTED:
        QMessageBox::critical(win,"Crypto Error", "The Crypto service is not started!");
        break;
    case PSYNC_CRYPTO_RSA_ERROR:
        QMessageBox::critical(win,"Crypto Error", "Can't create a Crypto Folder. RSA error.");
        break;
    case PSYNC_CRYPTO_FOLDER_NOT_FOUND:
        break;
    case PSYNC_CRYPTO_FILE_NOT_FOUND:
        break;
    case PSYNC_CRYPTO_INVALID_KEY:
        QMessageBox::critical(win,"Crypto Error", "Can't create a Crypto Folder. Invalid Key!");
        break;
    case PSYNC_CRYPTO_CANT_CONNECT:
        QMessageBox::critical(win,"Crypto Error", "Can't create a Crypto Folder. Unable to connect to server.");
        break;
    case PSYNC_CRYPTO_FOLDER_NOT_ENCRYPTED:
        break;
    case PSYNC_CRYPTO_INTERNAL_ERROR:
        QMessageBox::critical(win,"Crypto Error", "Can't create a Crypto Folder. Internal error.");
        break;
    default:
        QMessageBox::critical(win,"Crypto Error", trUtf8(err));
        break;
    }
}

void CryptoPage::showStopCryptoError(int stopRes)
{
    qDebug()<<"CRYPTO: stop crypto "<<stopRes;
    switch(stopRes)
    {
    case PSYNC_CRYPTO_STOP_NOT_SUPPORTED:
        QMessageBox::critical(win,"Crypto Error", "Stop not supported!");
        break;
    case PSYNC_CRYPTO_STOP_NOT_STARTED:
        QMessageBox::critical(win,"Crypto Error", "The Crypto service is not started!");
        break;
    default:
        break;
    }
}
