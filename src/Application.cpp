#include "Application.h"
#include "Enums.h"
#include "Style.h"
#include "UISoundEmitter.h"
#include "Backend/DatabaseManager.h"
#include "Backend/ScriptRunner.h"
#include "Backend/ThreadedSystem.h"

#include <QDebug>
#include <QFontDatabase>
#include <cassert>

CApplication::CApplication(int& argc, char *argv[]) :
  QApplication(argc, argv),
  m_spSystemsMap(),
  m_spSoundEmitter(std::make_unique<CUISoundEmitter>()),
  m_spSettings(nullptr),
  m_bStyleDirty(true),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{

}

//----------------------------------------------------------------------------------------
//
CApplication* CApplication::Instance()
{
  CApplication* pApp = dynamic_cast<CApplication*>(QCoreApplication::instance());
  assert(nullptr != pApp);
  return pApp;
}

//----------------------------------------------------------------------------------------
//
void CApplication::Initialize()
{
  // sound
  installEventFilter(m_spSoundEmitter.get());

  // fonts
  qint32 iFont = QFontDatabase::addApplicationFont("://resources/fonts/Equestria.otf");
  Q_ASSERT(-1 != iFont);
  Q_UNUSED(iFont)

  QStringList fonts;
  QFontDatabase fontDb;
  for(int i=0; i< fontDb.families().size(); i++)
  {
    fonts << fontDb.families().at(i);
  }
  //qDebug() << QString(tr("Supported font families: %1")).arg(fonts.join(", "));

  // settings
  m_spSettings = std::make_shared<CSettings>();
  connect(m_spSettings.get(), &CSettings::FontChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);
  connect(m_spSettings.get(), &CSettings::StyleChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);

  // sound emitter
  m_spSoundEmitter->Initialize();

  // create subsystems
  m_spSystemsMap.insert({ECoreSystems::eDatabaseManager, std::make_shared<CThreadedSystem>()});
  m_spSystemsMap.insert({ECoreSystems::eScriptRunner, std::make_shared<CThreadedSystem>()});

  // init subsystems
  m_spSystemsMap[ECoreSystems::eDatabaseManager]->RegisterObject<CDatabaseManager>();
  m_spSystemsMap[ECoreSystems::eScriptRunner]->RegisterObject<CScriptRunner>();

  // style
  LoadStyle();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
template<>
std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>()
{
  return std::static_pointer_cast<CDatabaseManager>(m_spSystemsMap[ECoreSystems::eDatabaseManager]->Get());
}

template<>
std::weak_ptr<CScriptRunner> CApplication::System<CScriptRunner>()
{
  return std::static_pointer_cast<CScriptRunner>(m_spSystemsMap[ECoreSystems::eScriptRunner]->Get());
}

//----------------------------------------------------------------------------------------
//
void CApplication::MarkStyleDirty()
{
  m_bStyleDirty = true;
  QTimer::singleShot(500, this, &CApplication::LoadStyle);
}

//----------------------------------------------------------------------------------------
//
void CApplication::LoadStyle()
{
  if (m_bStyleDirty)
  {
    m_bStyleDirty = false;
    joip_style::SetStyle(this);
    emit StyleLoaded();
  }
}

