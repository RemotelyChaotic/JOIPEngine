#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"

#if defined(Q_OS_WIN)
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

#include <QApplication>
#include <QtAVWidgets>
#include <QOpenGLContext>
#include <QSslSocket>
#include <QtWebEngine>
#include <QtWebView/QtWebView>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  // WebView or Webengine is not used anymore, but we'll keep this for now
  QtWebView::initialize();
  QtAV::Widgets::registerRenderers();

#ifndef NDEBU
  qDebug() << "SSL suport: " << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();
#endif

  CApplication app(argc, argv);

  app.Initialize();

  // bugfix on vertain machines
  QOpenGLContext context;
  context.create();

  CMainWindow w;
  w.Initialize();
  w.show();

#if defined(Q_OS_WIN)
  // Fixes problems with OpenGL based windows
  // https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  QWindowsWindowFunctions::setHasBorderInFullScreen(w.windowHandle(), true);
#endif

  return app.exec();
}
