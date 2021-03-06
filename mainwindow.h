#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);

public slots:
  void slotTimerAlarm();
  void slotSaveTimer();

private:
  QSystemTrayIcon* trayIcon;
  QMenu* trayIconMenu;

  QMenu* createMenu();
};
