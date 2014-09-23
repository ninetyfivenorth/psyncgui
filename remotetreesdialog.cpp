#include "remotetreesdialog.h"
#include "ui_remotetreesdialog.h"
#include "pcloudwindow.h"

RemoteTreesDialog::RemoteTreesDialog(PCloudWindow* &w,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteTreesDialog)
{
    ui->setupUi(this);
    this->win = w;
    ui->widget_fldrName->setVisible(false); /// for acceptshare - to edin fldr name
    root = ui->treeRemoteFldrs->currentItem();
    this->init();
    connect(ui->btnAccept, SIGNAL(clicked()), this,SLOT(setSelectedFolder()));
    connect(ui->btnReject, SIGNAL(clicked()),this,SLOT(hide()));
    connect(ui->btnNewFolder, SIGNAL(clicked()), this, SLOT(newRemoteFldr()));
    this->setWindowIcon(QIcon(WINDOW_ICON));
    this->setWindowTitle("pCloud");

}

void RemoteTreesDialog::init()
{
    if(ui->treeRemoteFldrs->topLevelItemCount())
        ui->treeRemoteFldrs->clear();
    win->initRemoteTree(ui->treeRemoteFldrs);
}

RemoteTreesDialog::~RemoteTreesDialog()
{
    delete ui;
}

void RemoteTreesDialog::showEvent(QShowEvent *event)
{
    ui->treeRemoteFldrs->setCurrentItem(this->root);
    event->accept();
}

void RemoteTreesDialog::newRemoteFldr()
{
    this->win->newRemoteFldr(ui->treeRemoteFldrs);
}

void RemoteTreesDialog::setSelectedFolder()
{    
    if(!ui->treeRemoteFldrs->currentItem())
    {
        QMessageBox::warning(this,"pCloud",trUtf8("No remote folder is selected. Please click on a folder to select"));
        return;
    }
    else
    {
        fldrid = ui->treeRemoteFldrs->currentItem()->data(1,Qt::UserRole).toULongLong();
        fldrPath = ui->treeRemoteFldrs->currentItem()->data(0,Qt::UserRole).toString();
        this->accept();
    }
}

quint64 RemoteTreesDialog::getFldrid()
{
    return this->fldrid;
}
QString RemoteTreesDialog::getFldrPath()
{
    return this->fldrPath;
}