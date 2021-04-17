#include "settingsform.h"
#include "ui_settingsform.h"
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Enter password:"), QLineEdit::Password,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        if (text != "mysuperpass")
        {
            QMessageBox msgBox;
            msgBox.setText("Wrong password");
            msgBox.exec();

            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
    else
    {
        QTimer::singleShot(0, this, SLOT(close()));
    }



    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(addin_path);
    if (!dir.exists())
        dir.mkpath(addin_path);
    if (!dir.exists("addins"))
        dir.mkdir("addins");

    dir.cd("addins");
    addin_path = dir.absoluteFilePath("settings.ini");
    QSettings *settings = new QSettings(addin_path,QSettings::IniFormat);
    ui->settingsPath->setText(settings->value("settings/path").value<QString>());
    ui->userName->setText(settings->value("settings/userName").value<QString>());

}

SettingsForm::~SettingsForm()
{
    delete ui;
}

void SettingsForm::on_CancelButton_clicked()
{
    this->hide();
}

void SettingsForm::on_SaveButton_clicked()
{
    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(addin_path);
    if (!dir.exists())
        dir.mkpath(addin_path);
    if (!dir.exists("addins"))
        dir.mkdir("addins");

    dir.cd("addins");
    addin_path = dir.absoluteFilePath("settings.ini");
    QSettings *settings = new QSettings(addin_path,QSettings::IniFormat);
    settings->setValue("settings/path", ui->settingsPath->text());
    settings->setValue("settings/userName", ui->userName->text());
    settings->sync();

}

void SettingsForm::on_GetSettingsPath_clicked()
{

    ui->settingsPath->setText(QFileDialog::getOpenFileName(this, tr("Открыть файл с настройками"), "", tr("Файл настроек (*.ini)")));
}
