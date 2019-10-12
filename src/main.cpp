#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"
#include <QApplication>
#include <QtWebEngine>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  CApplication app(argc, argv);

  QtWebEngine::initialize();
  app.Initialize();

  CMainWindow w;
  w.Initialize();
  w.show();

  return app.exec();
}
