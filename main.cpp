#include <QApplication>
#include <QSystemTrayIcon>
#include <QDebug>
#include <QTime>
#include <QChar>



using namespace std;


#include "mainwindow.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(resources);

  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);


  MainWindow w;
  w.hide();

  return a.exec();
}
