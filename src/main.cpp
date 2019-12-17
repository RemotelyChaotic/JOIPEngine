#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"
#include <QApplication>
#include <QOpenGLContext>
#include <QtWebEngine>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
  CApplication app(argc, argv);

  QtWebEngine::initialize();
  app.Initialize();

  // bugfix on vertain machines
  QOpenGLContext context;
  context.create();

  CMainWindow w;
  w.Initialize();
  w.show();

  return app.exec();
}
