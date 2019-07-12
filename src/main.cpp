#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  CApplication app(argc, argv);
  app.Initialize();
  CMainWindow w;
  w.show();

  return app.exec();
}
