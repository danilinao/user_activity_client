#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <Windows.h>
#include <iostream>
#include "mainwindow.h"
#include "settingsform.h"

#pragma comment(lib, "user32.lib")

HHOOK hHook = NULL;
HHOOK hMouseHook = NULL;
QFile *saveFile;
QTextStream *stream;
QTimer *timer;
QTimer *saveTimer;
bool writeMouseEvent = false;
QString saveFileName;
QTime timeInterval;
QDir saveDir;

void UpdateKeyState(BYTE *keystate, int keycode)
{
    keystate[keycode] = GetKeyState(keycode);
}

LRESULT CALLBACK MyLowLevelKeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    KBDLLHOOKSTRUCT cKey = *((KBDLLHOOKSTRUCT*)lParam);

    wchar_t buffer[5];

    //get the keyboard state
    BYTE keyboard_state[256];
    GetKeyboardState(keyboard_state);
    UpdateKeyState(keyboard_state, VK_SHIFT);
    UpdateKeyState(keyboard_state, VK_CAPITAL);
    UpdateKeyState(keyboard_state, VK_CONTROL);
    UpdateKeyState(keyboard_state, VK_MENU);

    HKL keyboard_layout = GetKeyboardLayout(0);

    char lpszName[0x100] = {0};

    DWORD dwMsg = 1;
    dwMsg += cKey.scanCode << 16;
    dwMsg += cKey.flags << 24;

    int i = GetKeyNameText(dwMsg, (LPTSTR)lpszName,255);

    //Try to convert the key info
    int result = ToUnicodeEx(cKey.vkCode, cKey.scanCode, keyboard_state, buffer,4,0, keyboard_layout);
    buffer[4] = L'\0';

    //Print the output
    *stream  << "key: " << QString::fromUtf16((ushort*)lpszName)<< Qt::endl;

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}


LRESULT CALLBACK mouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
    if (pMouseStruct != NULL){
        bool ev = false;
            QString str = "";
            if(wParam == WM_LBUTTONDOWN)
            {
                str = "Left cick;";
                ev = true;
            }
            else if(wParam == WM_RBUTTONDOWN)
            {
                str = "Right cick;";
                ev = true;
            } else if(wParam == WM_MBUTTONDOWN)
            {
                str = "Middle cick;";
                ev = true;
            } else if(wParam == WM_MOUSEWHEEL)
            {
                str = "Whell rotate;";
                ev = true;
            }
            if(writeMouseEvent && !ev || ev)
            {


                *stream << str << pMouseStruct->pt.x << ";" << pMouseStruct->pt.y << Qt::endl;
                writeMouseEvent = false;
            }
    }
    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

DWORD WINAPI MyMouseLogger(LPVOID lpParm)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    hMouseHook = SetWindowsHookEx( WH_MOUSE_LL, mouseProc, hInstance, NULL );

    MSG message;
    while (GetMessage(&message,NULL,0,0)) {
        TranslateMessage( &message );
        DispatchMessage( &message );
    }

    UnhookWindowsHookEx(hMouseHook);
    return 0;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , trayIcon(new QSystemTrayIcon(this))
{

    auto menu = this->createMenu();
    this->trayIcon->setContextMenu(menu);

    auto appIcon = QIcon(":/icons/heart.png");
    this->trayIcon->setIcon(appIcon);
    this->setWindowIcon(appIcon);

    this->trayIcon->show();

    timer = new QTimer();
    saveFileName = "logfile";
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(500);

    QString addin_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(addin_path);
    if (!dir.exists())
        dir.mkpath(addin_path);
    if (!dir.exists("addins"))
        dir.mkdir("addins");

    dir.cd("addins");
    addin_path = dir.absoluteFilePath("settings.ini");
    QSettings *settings = new QSettings(addin_path,QSettings::IniFormat);
    QString settingsPath = settings->value("settings/path").value<QString>();
    QString userName = settings->value("settings/userName").value<QString>();

    if(!settingsPath.isEmpty())
    {

        QSettings *settings = new QSettings(settingsPath,QSettings::IniFormat);
        QString savePath = settings->value("settings/savePath").value<QString>();
        timeInterval = settings->value("settings/timeInterval").value<QTime>();


        if(!userName.isEmpty() & !savePath.isEmpty() & !(QTime(0, 0, 0).secsTo(timeInterval) == 0))
        {
            saveTimer = new QTimer();

            connect(saveTimer, SIGNAL(timeout()), this, SLOT(slotSaveTimer()));
            int sec = QTime(0, 0, 0).secsTo(timeInterval);
            saveTimer->start(sec*1000 );

            saveDir = QDir(savePath);
            if (!saveDir.exists())
                saveDir.mkpath(savePath);
            if (!saveDir.exists(userName))
                saveDir.mkdir(userName);

            saveDir.cd(userName);
            saveFile = new QFile(saveDir.absoluteFilePath(saveFileName));

            if(saveFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            {
                stream = new QTextStream(saveFile);
                hHook = SetWindowsHookEx(WH_KEYBOARD_LL, MyLowLevelKeyBoardProc, NULL, 0);
                if(hHook == NULL)
                {
                    *stream << "Hook failed" << Qt::endl;
                }

                hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, NULL, 0);
                if(hMouseHook == NULL)
                {
                    *stream << "Hook mouse failed" << Qt::endl;
                }

            }

        }
        else
        {

            QMessageBox msgBox;
            msgBox.setText("Имя текущего пользователя не назначено!");
            msgBox.exec();

        }

    }
    else
    {

        QMessageBox msgBox;
        msgBox.setText("Файл настроек не назначен!");
        msgBox.exec();

    }


}

void MainWindow::slotTimerAlarm()
{

    writeMouseEvent = true;
    timer->start(1000);

}

void MainWindow::slotSaveTimer()
{
    saveFileName = saveFileName + "_1";
    int sec = QTime(0, 0, 0).secsTo(timeInterval);
    saveTimer->start(sec*1000);
    saveFile->close();

    saveFile = new QFile(saveDir.absoluteFilePath(saveFileName));
    if(saveFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        stream = new QTextStream(saveFile);
    }
}

QMenu* MainWindow::createMenu()
{

    auto quitAction = new QAction("&Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto settingsAction = new QAction("&Settings", this);
    connect(settingsAction, &QAction::triggered, qApp,  [](){SettingsForm *testWindow = new SettingsForm(); testWindow->show();});

    auto menu = new QMenu(this);
    menu->addAction(quitAction);
    menu->addAction(settingsAction);

    return menu;
}


