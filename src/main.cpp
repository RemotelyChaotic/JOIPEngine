#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"
#include "MainWindowFactory.h"
#include "Systems/NotificationSender.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/PhysFs/PhysFsQtAVIntegration.h"

#include <QApplication>
#include <QImageReader>
#include <QtAVWidgets>
#include <QtAV\QtAV_Global.h>
#include <QOpenGLContext>
#if defined(QT_QML_DEBUG)
#include <QQmlDebuggingEnabler>
#endif
#include <QSslSocket>
//#include <QtWebEngine>
//#include <QtWebView/QtWebView>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  // WebView or Webengine is not used anymore, but we'll keep this for now
  //QtWebView::initialize();
  QtAV::Widgets::registerRenderers();

  // Too many debug messages in debug, if needed set to debug temporarilly
#ifndef NDEBUG
  QtAV::setLogLevel(QtAV::LogLevel::LogWarning);
#else
  QtAV::setLogLevel(QtAV::LogLevel::LogCritical);
#endif

  CPhysFsFileEngine::init(argv[0]);
  CPhysFsFileEngineHandler engine;
  QtAV::RegisterPhysFsFileHandler();

  qDebug() << "SSL suport:" << QSslSocket::supportsSsl() <<
              "- SLL Build version:" << QSslSocket::sslLibraryBuildVersionString() <<
              "- SLL Library version:" << QSslSocket::sslLibraryVersionString();
  qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();
  qDebug() << "Supported archive formats:" << CPhysFsFileEngineHandler::SupportedFileTypes();

  CApplication app(argc, argv);

#if defined(QT_QML_DEBUG)
  QQmlDebuggingEnabler enabler;
#endif

  app.Initialize();

  // bugfix on certain machines
  QOpenGLContext context;
  context.create();

  std::unique_ptr<CMainWindowBase> spW = CMainWindowFactory::Instance().CreateMainWindow();
  spW->Initialize();
  spW->show();
  spW->OnShow();

  Notifier()->SetMainWindow(dynamic_cast<CMainWindow*>(spW.get()));

  qint32 iRetVal = app.exec();

  // uninitialize logic
  CPhysFsFileEngine::deInit();
  return iRetVal;
}
