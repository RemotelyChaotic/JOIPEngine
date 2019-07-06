#include "CApplication.h"
#include "CSettings.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>

void InitialiseQmlEngine(CApplication *pApp, QQmlApplicationEngine* pEngine);

//----------------------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  CApplication app(argc, argv);
  app.Initialize();

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty())
  { return -1; }

  InitialiseQmlEngine(&app, &engine);

  return app.exec();
}

//----------------------------------------------------------------------------------------
//
void InitialiseQmlEngine(CApplication* pApp, QQmlApplicationEngine* pEngine)
{
  if (nullptr == pApp || nullptr == pEngine) { return; }

  pEngine->rootContext()->setContextProperty("Settings", pApp->Settings().get());
}
