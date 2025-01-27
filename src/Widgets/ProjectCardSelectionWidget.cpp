#include "ProjectCardSelectionWidget.h"
#include "Application.h"
#include "Constants.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Systems/ProjectDownloader.h"
#include "Systems/Script/ScriptDbWrappers.h"
#include "Widgets/HelpOverlay.h"
#include "ui_ProjectCardSelectionWidget.h"

#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUrl>
#include <cassert>

namespace  {
  const QString c_sCardHelpId =           "Player/Card";
  const QString c_sSearchBarHelpId =      "Player/SearchBar";
}

//----------------------------------------------------------------------------------------
//
CProjectCardSelectionWidget::CProjectCardSelectionWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CProjectCardSelectionWidget>()),
  m_flags(0),
  m_selectionColor(Qt::white),
  m_iSelectedProjectId(-1),
  m_bLoadedQml(false)
{
  qRegisterMetaType<EDownLoadStateFlags>();

  m_spUi->setupUi(this);
  Initialize();
}

CProjectCardSelectionWidget::~CProjectCardSelectionWidget()
{
  UnloadProjects();
  FinishUnloadPrivate();

  delete m_spUi->pQmlWidget;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::Initialize()
{
  m_bLoadedQml = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    wpHelpFactory->RegisterHelp(c_sCardHelpId, ":/resources/help/player/card_button_help.html");
    m_spUi->pQmlWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCardHelpId);
    wpHelpFactory->RegisterHelp(c_sSearchBarHelpId, ":/resources/help/editor/resources/filter_widget_help.html");
    m_spUi->pSearchWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSearchBarHelpId);
  }

  m_spUi->pQmlWidget->setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);

  InitQmlMain();

  connect(CHelpOverlay::Instance(), &CHelpOverlay::SignalOverlayOpened,
          this, &CProjectCardSelectionWidget::SlotOverlayOpened);
  connect(CHelpOverlay::Instance(), &CHelpOverlay::SignalOverlayClosed,
          this, &CProjectCardSelectionWidget::SlotOverlayClosed);

  auto wpDowloader = CApplication::Instance()->System<CProjectDownloader>();
  if (auto spDownloader = wpDowloader.lock())
  {
    connect(spDownloader.get(), &CProjectDownloader::SignalJobFinished,
            this, &CProjectCardSelectionWidget::SlotProjectDownloadFinished);
    connect(spDownloader.get(), &CProjectDownloader::SignalProgressChanged,
            this, &CProjectCardSelectionWidget::SlotProjectDownloadProgressChanged);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::LoadProjects(EDownLoadStateFlags flags)
{
  m_iSelectedProjectId = -1;

  if (!IsLoaded())
  {
    // load everything directly
    SlotLoadProjectsPrivate(flags);
  }
  else
  {
    // not unloaded yet, queue loading after everything is unloaded
    bool bOk = QMetaObject::invokeMethod(this, "SlotLoadProjectsPrivate", Qt::QueuedConnection,
                                         Q_ARG(EDownLoadStateFlags, flags));
    assert(bOk);
    Q_UNUSED(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::UnloadProjects()
{
  m_iSelectedProjectId = -1;

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "onUnLoad");

  disconnect(m_spUi->pQmlWidget->rootObject(), SIGNAL(selectedProjectIndex(int)),
             this, SLOT(SlotCardClicked(int)));

  bool bOk = QMetaObject::invokeMethod(this, "SlotUnloadFinished", Qt::QueuedConnection);
  assert(bOk);
  Q_UNUSED(bOk);
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::ShowWarning(const QString& sWarning)
{
  if (!m_bLoadedQml) { return; }

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  if (nullptr != pRootObject)
  {
    bool bOk = QMetaObject::invokeMethod(pRootObject, "showWarning", Q_ARG(QVariant, sWarning));
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SetSelectionColor(const QColor& color)
{
  if (m_selectionColor != color)
  {
    m_selectionColor = color;

    if (!m_bLoadedQml) { return; }

    QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
    if (nullptr != pRootObject)
    {
      pRootObject->setProperty("selectionColor", QVariant::fromValue(m_selectionColor));
    }
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CProjectCardSelectionWidget::SelectionColor()
{
  return m_selectionColor;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::on_pSearchWidget_SignalFilterChanged(const QString& sText)
{
  if (!m_bLoadedQml) { return; }

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  if (nullptr != pRootObject)
  {
    pRootObject->setProperty("filter", sText);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::on_pQmlWidget_statusChanged(QQuickWidget::Status status)
{
  if (QQuickWidget::Error == status)
  {
    QStringList errors;
    const auto widgetErrors = m_spUi->pQmlWidget->errors();
    for (const QQmlError &error : widgetErrors)
    {
      errors.append(error.toString());
    }
    QString sErrors = errors.join(QStringLiteral(", "));
    qWarning() << sErrors;
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError /*error*/,
                                                                const QString &message)
{
  qWarning() << message;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotCardClicked(int iProjId)
{
  m_iSelectedProjectId = iProjId;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotLoadProjectsPrivate(EDownLoadStateFlags flags)
{
  m_flags = flags;
  m_spUi->pQmlWidget->setSource(QUrl("qrc:/qml/resources/qml/JoipEngine/ProjectCardSelection.qml"));

  connect(m_spUi->pQmlWidget->rootObject(), SIGNAL(selectedProjectIndex(int)),
          this, SLOT(SlotCardClicked(int)));

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  pRootObject->setProperty("selectionColor", QVariant::fromValue(m_selectionColor));

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    std::set<qint32, std::less<qint32>> ids = spDbManager->ProjectIds();
    for (auto it = ids.begin(); ids.end() != it; ++it)
    {
      SlotProjectAdded(*it);
    }
  }

  m_bLoadedQml = true;

  if (auto spDbManager = m_wpDbManager.lock())
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalProjectAdded,
            this, &CProjectCardSelectionWidget::SlotProjectAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalProjectAdded,
            this, &CProjectCardSelectionWidget::SlotProjectRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotOverlayOpened()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, false);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, false);
  m_spUi->pQmlWidget->setStyleSheet(styleSheet());
  m_spUi->pQmlWidget->setClearColor(m_spUi->pQmlWidget->palette().window().color());
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotOverlayClosed()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  m_spUi->pQmlWidget->setClearColor(Qt::transparent);
  m_spUi->pQmlWidget->setStyleSheet("background-color: transparent;");
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotProjectAdded(qint32 iId)
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
    QJSEngine* pEngine = m_spUi->pQmlWidget->engine();

    tspProject spProject = spDbManager->FindProject(iId);
    if (nullptr != spProject)
    {
      bool bDisplay = true;
      if (-1 == m_iSelectedProjectId)
      {
        QReadLocker locker(&spProject->m_rwLock);
        m_iSelectedProjectId = spProject->m_iId;
      }

      {
        QReadLocker locker(&spProject->m_rwLock);
        bDisplay = m_flags & spProject->m_dlState;
      }

      if (bDisplay)
      {
        CProjectScriptWrapper* pValue = new CProjectScriptWrapper(pEngine, spProject);
        m_spUi->pQmlWidget->rootObject()->setProperty("currentlyAddedProject", QVariant::fromValue(pValue));
        QMetaObject::invokeMethod(pRootObject, "onAddProject");
        m_vpProjects.push_back(pValue);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotProjectDownloadFinished(qint32 iProjId)
{
  if (!IsLoaded()) { return; }

  if (!(m_flags & EDownLoadState::eFinished))
  {
    SlotProjectRemoved(iProjId);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotProjectDownloadProgressChanged(qint32 iProjId,
                                                                     qint32 iProgress)
{
  if (!IsLoaded()) { return; }

  // add project to view if not already present
  auto it = std::find_if(m_vpProjects.begin(), m_vpProjects.end(),
                         [iProjId](const QPointer<CProjectScriptWrapper>& pProj) {
    return pProj->getId() == iProjId;
  });
  if (m_vpProjects.end() == it)
  {
    SlotProjectAdded(iProjId);
  }

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "onUpdateProject",
                            Q_ARG(QVariant, QVariant(iProjId)),
                            Q_ARG(QVariant, QVariant(iProgress)));
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotProjectRemoved(qint32 iId)
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();

  m_spUi->pQmlWidget->rootObject()->setProperty("currentlyRemovedProject", iId);
  QMetaObject::invokeMethod(pRootObject, "onRemoveProject");

  for (auto it = m_vpProjects.begin(); m_vpProjects.end() != it; ++it)
  {
    if ((*it)->getId())
    {
      m_vpProjects.erase(it);
      return;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotResizeDone()
{
  if (!m_bLoadedQml) { return; }

  // set size properties manually setzen, since this isn't done automatically
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QSize newSize = m_spUi->pQmlWidget->size();
  if (nullptr != pRootObject)
  {
    pRootObject->setProperty("width", QVariant::fromValue(newSize.width()));
    pRootObject->setProperty("height",QVariant::fromValue(newSize.height()));
    QMetaObject::invokeMethod(pRootObject, "onResize");
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotUnloadFinished()
{
  FinishUnloadPrivate();
  emit SignalUnloadFinished();
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::resizeEvent(QResizeEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    SlotResizeDone();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::FinishUnloadPrivate()
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    disconnect(spDbManager.get(), &CDatabaseManager::SignalProjectAdded,
            this, &CProjectCardSelectionWidget::SlotProjectAdded);
    disconnect(spDbManager.get(), &CDatabaseManager::SignalProjectAdded,
            this, &CProjectCardSelectionWidget::SlotProjectRemoved);
  }

  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();
  m_spUi->pQmlWidget->setSource(QUrl());

  QQuickWidget::Status status = m_spUi->pQmlWidget->status();
  assert(status == QQuickWidget::Null);
  Q_UNUSED(status);

  for (CProjectScriptWrapper* pProject : qAsConst(m_vpProjects))
  {
    if (!pProject->isLoaded())
    {
      tspProject spProject = pProject->Data();
      CDatabaseManager::UnloadProject(spProject);
    }
    delete pProject;
  }
  m_vpProjects.clear();

  m_bLoadedQml = false;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::InitQmlMain()
{
  SlotOverlayClosed();

  QQmlEngine::setObjectOwnership(m_spUi->pQmlWidget->engine(), QQmlEngine::CppOwnership);

  // engine will always take owership of this object
  CDatabaseImageProvider* pProvider = new CDatabaseImageProvider(m_wpDbManager);
  m_spUi->pQmlWidget->engine()->addImageProvider("DataBaseImageProivider", pProvider);
}
