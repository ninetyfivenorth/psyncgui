#include "pcloudwindow.h"
#include "ui_pcloudwindow.h"
#include "pcloudapp.h"
#include "changepassdialog.h"
#include "ui_changepassdialog.h"
#include "common.h"
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDebug>

PCloudWindow::PCloudWindow(PCloudApp *a,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PCloudWindow)
{
    app=a;
    ui->setupUi(this);
    this->verifyClicked = false;
    vrsnDwnldThread = NULL;

    // the window consists of QListWidget(for icon-buttons) and
    //QStackedWidget which loads different page according to selected item in the listWidget (and hides other pages)
    ui->listButtonsWidget->setViewMode(QListView::IconMode);
    ui->listButtonsWidget->setFlow(QListWidget::LeftToRight); //orientation
    ui->listButtonsWidget->setSpacing(12);
    ui->listButtonsWidget->setStyleSheet("background-color:transparent");
    ui->listButtonsWidget->setMovement(QListView::Static); //not to move items with the mouse
    ui->listButtonsWidget->setMinimumWidth(450);  //++ for crypto
#ifdef Q_OS_WIN
    ui->listButtonsWidget->setMaximumHeight(80);
    ui->listButtonsWidget->setMinimumHeight(79);
#else
    ui->listButtonsWidget->setMaximumHeight(85);
    ui->listButtonsWidget->setMinimumHeight(84); // precakva mi layouta
#endif

    ui->listButtonsWidget->installEventFilter(this);
#ifndef Q_OS_WIN
    if (ui->listButtonsWidget->palette().highlightedText().color().value() == 255) //if selection is white
    {
        QIcon accnticon;
        accnticon.addPixmap(QPixmap(":/128x128/images/128x128/user.png"), QIcon::Normal);
        accnticon.addPixmap(QPixmap(":/128x128/images/128x128/user-w.png"), QIcon::Selected);
        new QListWidgetItem(accnticon,trUtf8("Account "), ui->listButtonsWidget); //index 0

        QIcon syncicon;
        syncicon.addPixmap(QPixmap(":/128x128/images/128x128/sync.png"), QIcon::Normal);
        syncicon.addPixmap(QPixmap(":/128x128/images/128x128/sync-w.png"), QIcon::Selected);
        new QListWidgetItem(syncicon,trUtf8("  Sync  "), ui->listButtonsWidget); //index 1

        QIcon sharesicon;
        sharesicon.addPixmap(QPixmap(":/128x128/images/128x128/shares.png"), QIcon::Normal);
        sharesicon.addPixmap(QPixmap(":/128x128/images/128x128/shares-w.png"), QIcon::Selected);
        new QListWidgetItem(sharesicon,trUtf8(" Shares "), ui->listButtonsWidget); //index 2

        QIcon cryptoicon;
        cryptoicon.addPixmap(QPixmap(":/128x128/images/128x128/crypto.png"), QIcon::Normal);
        cryptoicon.addPixmap(QPixmap(":/128x128/images/128x128/crypto-w.png"), QIcon::Selected);
        new QListWidgetItem(cryptoicon,trUtf8(" Crypto "), ui->listButtonsWidget); //index 3

        QIcon settingsicon;
        settingsicon.addPixmap(QPixmap(":/128x128/images/128x128/settings.png"), QIcon::Normal);
        settingsicon.addPixmap(QPixmap(":/128x128/images/128x128/settings-w.png"), QIcon::Selected);
        new QListWidgetItem(settingsicon,trUtf8("Settings"), ui->listButtonsWidget); //index 4

        QIcon helpicon;
        helpicon.addPixmap(QPixmap(":/128x128/images/128x128/help.png"), QIcon::Normal);
        helpicon.addPixmap(QPixmap(":/128x128/images/128x128/help-w.png"), QIcon::Selected);
        new QListWidgetItem(helpicon,trUtf8("  Help  "), ui->listButtonsWidget); //index 5

        QIcon abouticon;
        abouticon.addPixmap(QPixmap(":/128x128/images/128x128/info.png"), QIcon::Normal);
        abouticon.addPixmap(QPixmap(":/128x128/images/128x128/info-w.png"), QIcon::Selected);
        new QListWidgetItem(abouticon,trUtf8(" About  "), ui->listButtonsWidget); //index 6

    }
    else
    {
        new QListWidgetItem(QIcon(":/128x128/images/128x128/user.png"),trUtf8("Account"),ui->listButtonsWidget); //index 0
        new QListWidgetItem(QIcon(":/128x128/images/128x128/sync.png"),trUtf8("Sync"),ui->listButtonsWidget); //Sync Page index 1
        new QListWidgetItem(QIcon(":/128x128/images/128x128/shares.png"),trUtf8("Shares"),ui->listButtonsWidget); //index 2
        new QListWidgetItem(QIcon(":/128x128/images/128x128/crypto.png"),trUtf8("Crypto"),ui->listButtonsWidget); //index 3
        new QListWidgetItem(QIcon(":/128x128/images/128x128/settings.png"),trUtf8("Settings"),ui->listButtonsWidget); //index 4
        new QListWidgetItem(QIcon(":/128x128/images/128x128//help.png"),trUtf8("Help"),ui->listButtonsWidget); //index 4
        new QListWidgetItem(QIcon(":/128x128/images/128x128/info.png"),trUtf8("About"),ui->listButtonsWidget); //index 6
    }

#else
    new QListWidgetItem(QIcon(":/128x128/images/128x128/user.png"),trUtf8("Account"),ui->listButtonsWidget); //index 0
    new QListWidgetItem(QIcon(":/128x128/images/128x128/sync.png"),trUtf8("Sync"),ui->listButtonsWidget); //Sync Page index 1
    new QListWidgetItem(QIcon(":/128x128/images/128x128/shares.png"),trUtf8("Shares"),ui->listButtonsWidget); //index 2
    new QListWidgetItem(QIcon(":/128x128/images/128x128/crypto.png"),trUtf8("Crypto"),ui->listButtonsWidget); //index 3
    new QListWidgetItem(QIcon(":/128x128/images/128x128/settings.png"),trUtf8("Settings"),ui->listButtonsWidget); //index 4
    new QListWidgetItem(QIcon(":/128x128/images/128x128//help.png"),trUtf8("Help"),ui->listButtonsWidget); //index 5
    new QListWidgetItem(QIcon(":/128x128/images/128x128/info.png"),trUtf8("About"),ui->listButtonsWidget); //index 6
#endif

    fillAccountLoggedPage();
    fillAboutPage();
    ui->label_user->setFont(app->bigger1pFont);
    ui->label_plan->setFont(app->bigger1pFont);
    ui->label_versionVal->setText(QString("Installed Version: ") + APP_VERSION);
    this->setFrameProps(ui->frame_accnt);
    this->setFrameProps(ui->frame_help);
    this->setFrameProps(ui->frame);
    this->setFrameProps(ui->frameCryptoWelcome);
    this->setFrameProps(ui->frameCryptoKey);
    this->setFrameProps(ui->frameCryptoMain);

    cryptoPage = new CryptoPage(this, app);
    settngsPage = new SettingsPage(this, app); //needs cryptopage because of expired cryptocheck
    syncPage = new SyncPage(this, app);
    sharesPage = new SharesPage(this, app);

    // indexes of Items in listWidget and their coresponding pages in StackWidget are the same
    connect(ui->listButtonsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    connect(ui->btnVerify, SIGNAL(clicked()), this, SLOT(verifyEmail()));
    connect(ui->btnShareFolder, SIGNAL(clicked()), app, SLOT(addNewShare()));
    connect(this, SIGNAL(refreshPageSgnl(int,int)), this, SLOT(refreshPageSlot(int,int)));

    //for resize
    for(int i = 0; i < ui->pagesWidget->count(); i++)
        ui->pagesWidget->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    setWindowIcon(QIcon(WINDOW_ICON));
    setWindowTitle("pCloud Drive");
    this->setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    connect(ui->btnOnlineHelp, SIGNAL(clicked()), this, SLOT(openOnlineHelp()));
    connect(ui->btnOnlineTutorial, SIGNAL(clicked()), this, SLOT(openOnlineTutorial()));
    connect(ui->btnFeedback, SIGNAL(clicked()), this, SLOT(sendFeedback()));
    connect(ui->btnChangePass, SIGNAL(clicked()), this, SLOT(changePass()));
    connect(ui->btnForgotpass, SIGNAL(clicked()), this, SLOT(forgotPass()));
    connect(ui->btnGetMoreSpace, SIGNAL(clicked()), this, SLOT(upgradePlan()));
    connect(ui->btnMyPcloud, SIGNAL(clicked()), this, SLOT(openMyPcloud()));
    connect(ui->comboBox_versionReminder, SIGNAL(currentIndexChanged(int)), app, SLOT(setTimerInterval(int)));
    connect(ui->btnUpdtVersn, SIGNAL(clicked()), this, SLOT(updateVersion()));
    connect(ui->btnLgout, SIGNAL(clicked()), app, SLOT(logOut()));
    connect(ui->btnUnlink, SIGNAL(clicked()), this, SLOT(unlinkSync()));

    this->setMinimumHeight(560);
#ifdef Q_OS_WIN
    //this->setMinimumWidth(724);
    for (int i = 0; i < ui->pagesWidget->count(); i++)
    {
        ui->pagesWidget->widget(i)->setMinimumWidth(724);
    }
#endif

    updateGeometry();
}

void PCloudWindow::setFrameProps(QFrame *frame)
{
    QPalette Pal(palette());
    QColor frameColor(244,244,243);
    Pal.setColor(QPalette::Background,frameColor);
    //Pal.setColor(QPalette::Background,Qt::white); //ui->tabSyncList->palette().background().color());
    frame->setAutoFillBackground(true);
    frame->setPalette(Pal);
    frame->setFrameShadow(QFrame::Plain);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setMidLineWidth(3);
}

PCloudWindow::~PCloudWindow()
{    
    if(vrsnDwnldThread)
    {
        if(vrsnDwnldThread->isRunning())
            vrsnDwnldThread->terminate();
        vrsnDwnldThread->wait();
        delete vrsnDwnldThread;
    }
    delete ui;
}

void PCloudWindow::closeEvent(QCloseEvent *event) //not to close the app after window close
{
    hide();
    event->ignore();
}

bool PCloudWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->objectName() != "listButtonsWidget")
        return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int keyid = keyEvent->key();

        if(app->isLogedIn())
        {
            switch(keyid)
            {
            case Qt::Key_Home:
                this->setCurrntIndxPclWin(ACCNT_LOGGED_PAGE_NUM);
                event->ignore();
                return true;
            case Qt::Key_A:
                if(ui->listButtonsWidget->currentRow() == ACCNT_LOGGED_PAGE_NUM)
                    this->setCurrntIndxPclWin(ABOUT_PAGE_NUM);
                else
                    this->setCurrntIndxPclWin(ACCNT_LOGGED_PAGE_NUM);
                event->ignore();
                return true;
            case Qt::Key_D:
            case Qt::Key_Backspace:
                event->ignore();
                return true;
            default:
                return QObject::eventFilter(obj, event);
            }
        }
        else
        {
            switch(keyid)
            {
            case Qt::Key_Home:
                this->setCurrntIndxPclWin(HELP_PAGE_NUM);
                event->ignore();
                return true;
            case Qt::Key_A:
                this->setCurrntIndxPclWin(ABOUT_PAGE_NUM);
                event->ignore();
                return true;
            case Qt::Key_C:
            case Qt::Key_D:
            case Qt::Key_S:
            case Qt::Key_Backspace:
                event->ignore();
                return true;
            default:
                return QObject::eventFilter(obj, event);
            }
        }
    }
    else
        return QObject::eventFilter(obj, event);
}

void PCloudWindow::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    int currentIndex = ui->listButtonsWidget->row(current);

    // auto resize
    for(int i = 0; i < ui->pagesWidget->count(); i++)
    {
        if ( i != currentIndex)
            //ui->pagesWidget->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored); // resizes the form horizontally also
            ui->pagesWidget->widget(i)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored); //hoz vert
    }
    //ui->pagesWidget->widget(currentIndex)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); // resizes the form horizontally also
    ui->pagesWidget->widget(currentIndex)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred); //+

    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); // resizes the form horizontally also
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    updateGeometry();
    ui->pagesWidget->setCurrentIndex(currentIndex); // sets page
    refreshPage(currentIndex);
}
void PCloudWindow::showEvent(QShowEvent *)
{
    refreshPage(ui->listButtonsWidget->currentRow());
    // this->update();
}

void PCloudWindow::refreshPage(int currentIndex)
{
    switch(currentIndex)
    {
    case ACCNT_LOGGED_PAGE_NUM:
        if(verifyClicked)                 // Account page, case when user just has clicked Verify Email
            checkVerify();
        checkIsBusinessAccount();
        break;
    case SHARES_PAGE_NUM:
        sharesPage->loadAll();
        break;
    case CRYPTO_PAGE_NUM:
        cryptoPage->showEventCrypto(); // check cases for active subscription, pass setted up
        break;
    case SETTINGS_PAGE_NUM:
        settngsPage->showEvent();
        break;
    case ABOUT_PAGE_NUM:                    //About page, when have a new version
        if(app->new_version())
            fillAboutPage();
        break;
    default:
        break;
    }
}

void PCloudWindow::setCurrntIndxPclWin(int index)
{
    ui->listButtonsWidget->setCurrentRow(index);
    ui->pagesWidget->setCurrentIndex(index);
}

void PCloudWindow::setOnlineItems(bool online) // change pcloud window menu when is loggedin and loggedout
{
    if(online)
    {
        ui->listButtonsWidget->setRowHidden(ACCNT_LOGGED_PAGE_NUM, false); //Account - logged
        ui->listButtonsWidget->setRowHidden(SYNC_PAGE_NUM, false); //Sync
        ui->listButtonsWidget->setRowHidden(SHARES_PAGE_NUM, false); //Shares
        ui->listButtonsWidget->setRowHidden(CRYPTO_PAGE_NUM, false); //Crypto
        ui->listButtonsWidget->setRowHidden(SETTINGS_PAGE_NUM, false);       //setttings
    }
    else
    {
        ui->listButtonsWidget->setRowHidden(ACCNT_LOGGED_PAGE_NUM, true);
        ui->listButtonsWidget->setRowHidden(SYNC_PAGE_NUM,true);
        ui->listButtonsWidget->setRowHidden(SHARES_PAGE_NUM,true);
        ui->listButtonsWidget->setRowHidden(CRYPTO_PAGE_NUM, true);
        ui->listButtonsWidget->setRowHidden(SETTINGS_PAGE_NUM, true);
    }
    ui->label_upld->setVisible(online);
    ui->label_upldpic->setVisible(online);
    ui->label_dwnld->setVisible(online);
    ui->label_dwldpic->setVisible(online);
}

void PCloudWindow::initOnlinePages()
{
    this->fillAccountLoggedPage();
    this->cryptoPage->initCryptoPage();
    this->syncPage->load();
    this->syncPage->loadSettings();
    this->settngsPage->initPublic();
}

/*void PCloudWindow::setOnlinePages()
{    
    fillAccountLoggedPage();
}
*/

void PCloudWindow::fillAboutPage()
{        
    if(!app->new_version())
    {
        ui->label_versionInfo->setVisible(true);
        ui->widget_newVersion->setVisible(false);
    }
    else
    {
        ui->label_versionInfo->setVisible(false);
        ui->label_newVersion->setText(QString("New in version: "  + app->newVersion.versionstr + "\n"+ app->newVersion.notes));
        ui->comboBox_versionReminder->setCurrentIndex(app->settings->value("vrsnNotifyInvervalIndx").toInt());
    }
}

void PCloudWindow::fillAccountLoggedPage()
{    
    ui->label_email->setText(app->username);
    ui->progressBar_storage->setMinimum(0);
    ui->progressBar_storage->setMaximum(100);
    ui->progressBar_storage->setValue(100-app->freeSpacePercentage);
    ui->btnVerify->setVisible(!app->isVerified);
    ui->label_space->setText(app->usedSpaceStr + " (" +  QString::number(100 - app->freeSpacePercentage) + "%)");
    ui->label_planVal->setText(app->planStr);
    checkIsBusinessAccount();
}

void PCloudWindow::refreshUserinfo()
{
    ui->label_email->setText(app->username);
    ui->btnVerify->setVisible(!app->isVerified);
    ui->label_space->setText(app->usedSpaceStr + " (" +  QString::number(100 - app->freeSpacePercentage) + "%)");
    ui->label_planVal->setText(app->planStr);
    ui->progressBar_storage->setValue(100-app->freeSpacePercentage);
    if(ui->pagesWidget->currentIndex() == CRYPTO_PAGE_NUM)
        cryptoPage->showEventCrypto(); //check if crypto subsribtion changed
}

int PCloudWindow::getCurrentPage()
{
    return this->ui->listButtonsWidget->currentRow();
}

SyncPage* PCloudWindow::get_sync_page()
{
    return this->syncPage;
}

CryptoPage* PCloudWindow::getCryptoPage()
{
    return this->cryptoPage;
}

void PCloudWindow::refreshPagePulbic(int pageindex, int param)
{
    emit this->refreshPageSgnl(pageindex, param);
}

void PCloudWindow::setPageCurrentTab(int pageindex, int param)
{
    this->refreshPage(pageindex);
    if(pageindex == SETTINGS_PAGE_NUM)
        ui->tabWidgetSttngs->setCurrentIndex(param);
}

void PCloudWindow::changePass()
{
    ChangePassDialog *dialog = new ChangePassDialog();
    if (dialog->exec() == QDialog::Accepted)
    {
        char *err = NULL;
        int res =  psync_change_password(dialog->ui->line_pass->text().toUtf8(), dialog->ui->line_newpass->text().toUtf8(), &err);
        if (!res)
        {
            psync_set_pass(dialog->ui->line_newpass->text().toUtf8(),0);
            QMessageBox::information(this,trUtf8("New password"), trUtf8("Password successfully changed!"));
        }
        else
        {
            if (res == -1 )
                QMessageBox::information(this,trUtf8("New password"), trUtf8("No internet connection"));
            else
                QMessageBox::information(this,trUtf8("New password"), trUtf8(err));
        }
        free(err);
    }

}
void PCloudWindow::forgotPass()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    char *err = NULL;
    int res = psync_lost_password(app->username.toUtf8(),&err);
    if (!res) // returns 0 on success
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, trUtf8("Reset password"), trUtf8("An email with the password reset instructions was sent to your email."));
    }
    else
    {
        if (res == -1)
        {
            QMessageBox::information(this, trUtf8("Reset password"), trUtf8("No internet connection"));
            QApplication::restoreOverrideCursor();
            return;
        }
        else
        {
            QMessageBox::information(this, trUtf8("Reset password"), trUtf8(err));
            QApplication::restoreOverrideCursor();
            return;
        }
    }
    QApplication::restoreOverrideCursor();
    free(err);
}

void PCloudWindow::unlinkSync()
{
    QMessageBox::StandardButton reply;
    //reply = QMessageBox::warning(this,trUtf8("Unlink"), trUtf8("If you unlink your account from this computer any data about your synced folders will be lost. Do you still want to unlink?"),
    reply = QMessageBox::warning(this,trUtf8("Unlink"),
                                 trUtf8("By unlinking you will lose all pCloud Drive settings for the current account on this computer. Are you sure?"),
                                 QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        app->unlink();
}

void PCloudWindow::verifyEmail()
{
    char *err = NULL;
    int res = psync_verify_email(&err);
    if (!res)
    {
        QMessageBox::information(this, "Verify e-mail", "E-mail verification sent to: "+app->username);
        verifyClicked = true;
    }
    else
    {
        if(res == -1)
            QMessageBox::information(this,trUtf8("Verify e-mail"), trUtf8("No internet connection"));
        else
            QMessageBox::information(this,trUtf8("Verify e-mail"), trUtf8(err));
    }
    free(err);
}

void PCloudWindow::checkVerify() // has the user verified after had clicked "Verify Now"
{
    bool verified = psync_get_bool_value("emailverified");
    if (verified)
    {
        ui->btnVerify->setVisible(false);
        verifyClicked = false;
    }
}

void PCloudWindow::checkIsBusinessAccount()
{
    if(psync_get_bool_value("business"))
    {
        QFont boldfont;
        boldfont.setBold(true);
        boldfont.setPointSize(app->fontPointSize + 1);
        ui->label_company->setFont(boldfont);
        ui->label_company->setVisible(true);
        ui->label_companyVal->setVisible(true);
        QString companyName = psync_get_string_value("company");
        int maxCmpnyNameLength;
#ifdef Q_OS_WIN
        maxCmpnyNameLength = 50;
#else
        maxCmpnyNameLength = 30;
#endif
        if (companyName.length() > maxCmpnyNameLength)
        {
            ui->label_companyVal->setToolTip(companyName);
            companyName = companyName.left(maxCmpnyNameLength - 3).append("...");
        }
        ui->label_companyVal->setText(companyName);
    }
    else if(ui->label_company->isVisible())
    {
        ui->label_company->setVisible(false);
        ui->label_companyVal->setVisible(false);
    }

}

void PCloudWindow::updateVersion()
{
    app->stopTimer();
    QMessageBox::information(this,"pCloud Drive",trUtf8( "The new version of pCloud starts downloading and prepearing to install.\n Please wait."));
    if(!vrsnDwnldThread)
        vrsnDwnldThread = new VersionDwnldThread(app->OSStr);
    vrsnDwnldThread->start();
}

void PCloudWindow::refreshPageSlot(int pageindex, int param)
{
    switch(pageindex)
    {
    //case DRIVE_PAGE_NUM:
    //  this->fillDrivePage();
    case SHARES_PAGE_NUM:  // sharespage
        this->sharesPage->refreshTab(param);
    case SETTINGS_PAGE_NUM:
        this->settngsPage->initPublic();
    default:
        break;
    }
}

void PCloudWindow::openMyPcloud()
{
    QUrl url(QString("https://my.pcloud.com/#page=filemanager&authtoken=").append(psync_get_auth_string()));
    QDesktopServices::openUrl(url);
}

void PCloudWindow::upgradePlan()
{
    QUrl url(QString("https://my.pcloud.com/#page=plans&authtoken=").append(psync_get_auth_string()));
    QDesktopServices::openUrl(url);
}

void PCloudWindow::sendFeedback()
{
    QUrl url("https://my.pcloud.com/#page=contact"); //to check
    QDesktopServices::openUrl(url);
}

void PCloudWindow::openOnlineTutorial(){

    QUrl url ("https://my.pcloud.com/#page=contact&tutorial=show"); //to check
    QDesktopServices::openUrl(url);
}

void PCloudWindow::openOnlineHelp()
{
    //QUrl url ("https://my.pcloud.com/#page=faq"); //to check
    QUrl url("https://my.pcloud.com/#page=faqsync");
    QDesktopServices::openUrl(url);
}

void PCloudWindow::contactUs(){
    QUrl url ("https://my.pcloud.com/#page=contact");
    QDesktopServices::openUrl(url);
}
