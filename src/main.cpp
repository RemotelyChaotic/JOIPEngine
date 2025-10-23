#include "Application.h"
#include "Settings.h"
#include "MainWindow.h"
#include "MainWindowFactory.h"

#include "Systems/NotificationSender.h"

#include "Systems/Debug/JoipMessageHandler.h"

#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/PhysFs/PhysFsQtAVIntegration.h"

#include "Systems/Script/ScriptCacheFileEngine.h"

#include <JsonLuaWrapper.h>
#include <LuaSandboxWrapper.h>

#include <QApplication>
#include <QImageReader>
#include <QtAVWidgets>
#include <QtAV_Global.h>
#include <QOpenGLContext>
#if defined(QT_QML_DEBUG)
#include <QQmlDebuggingEnabler>
#endif
#include <QSslSocket>
#include <QStandardPaths>
//#include <QtWebEngine>
//#include <QtWebView/QtWebView>

void ClearOldQmlCache()
{
#if defined(Q_OS_WIN)
  // clear old cache folder
  QStringList vsOldLocs = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
  for (const QString& s : vsOldLocs)
  {
    QDir d(s);
    if (QFileInfo::exists(s))
    {
      d.cdUp();
      if (d.dirName() == CSettings::c_sApplicationName)
      {
        d.removeRecursively();
      }
      else
      {
        QDir d2(s);
        d2.removeRecursively();
      }
    }
  }
#endif
}

//----------------------------------------------------------------------------------------
//
void SetQmlCache()
{
#if defined(Q_OS_WIN)
  {
    QFileInfo settingsPath(settings::GetSettingsPath());
    QString sFolder = settingsPath.absolutePath() + "/cache";
    QDir().mkpath(sFolder);
    qputenv("QML_DISK_CACHE_PATH", sFolder.toUtf8());
    qDebug() << "QML cache:" << sFolder.toUtf8();
  }
#else
  QStringList vsOldLocs = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
  qputenv("QML_DISK_CACHE_PATH", vsOldLocs[0].toUtf8());
  qDebug() << "QML cache:" << vsOldLocs[0].toUtf8();
#endif
}

//----------------------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  // WebView or Webengine is not used anymore, but we'll keep this for now
  //QtWebView::initialize();
  QtAV::Widgets::registerRenderers();

  // Too many debug messages in debug, if needed set to debug temporarilly
#ifndef NDEBUG
  QtAV::setLogLevel(QtAV::LogLevel::LogCritical);
#else
  QtAV::setLogLevel(QtAV::LogLevel::LogCritical);
#endif

  CPhysFsFileEngine::init(argv[0]);
  CPhysFsFileEngineHandler engine;
  CScriptCacheFileEngineHandler scriptFileEngine;
  QtAV::RegisterPhysFsFileHandler();

  CJoipMessageHandler::InstallMsgHandler();

  qDebug() << "SSL suport:" << QSslSocket::supportsSsl() <<
              "- SLL Build version:" << QSslSocket::sslLibraryBuildVersionString() <<
              "- SLL Library version:" << QSslSocket::sslLibraryVersionString();
  qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();
  qDebug() << "Supported archive formats:" << CPhysFsFileEngineHandler::SupportedFileTypes();

  SetQmlCache();

  CApplication app(argc, argv);

  ClearOldQmlCache();

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
