#include "SceneCardSelectionWidget.h"
#include "Application.h"
#include "ui_SceneCardSelectionWidget.h"

#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/HelpFactory.h"
#include "Systems/Script/ScriptDbWrappers.h"

#include "Widgets/HelpOverlay.h"

#include <QQuickItem>
#include <QQmlEngine>

CSceneCardSelectionWidget::CSceneCardSelectionWidget(QWidget* pParent) :
    QWidget(pParent),
    m_spUi(std::make_unique<Ui::CSceneCardSelectionWidget>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CSceneCardSelectionWidget::~CSceneCardSelectionWidget()
{
  UnloadScenes();
  FinishUnloadPrivate();

  delete m_spUi->pQmlWidget;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::Initialize()
{
  m_bLoadedQml = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    // TODO:
  }

  m_spUi->pQmlWidget->setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);

  InitQmlMain();

  connect(CHelpOverlay::Instance(), &CHelpOverlay::SignalOverlayOpened,
          this, &CSceneCardSelectionWidget::SlotOverlayOpened);
  connect(CHelpOverlay::Instance(), &CHelpOverlay::SignalOverlayClosed,
          this, &CSceneCardSelectionWidget::SlotOverlayClosed);
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::LoadScenes(qint32 iId, const QStringList& vsScenes)
{
  m_sSelectedScene = QString();

  if (!IsLoaded())
  {
    // load everything directly
    SlotLoadScenesPrivate(iId, vsScenes);
  }
  else
  {
    // not unloaded yet, queue loading after everything is unloaded
    bool bOk = QMetaObject::invokeMethod(this, "SlotLoadScenesPrivate", Qt::QueuedConnection,
                                         Q_ARG(qint32, iId), Q_ARG(QStringList, vsScenes));
    assert(bOk);
    Q_UNUSED(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::UnloadScenes()
{
  m_sSelectedScene = QString();

  bool bOk = false;
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  if (nullptr != pRootObject)
  {
    bOk = QMetaObject::invokeMethod(pRootObject, "onUnLoad");
    assert(bOk);
    Q_UNUSED(bOk);

    pRootObject = m_spUi->pQmlWidget->rootObject();
    if (nullptr != pRootObject)
    {
      disconnect(pRootObject, SIGNAL(selectedScene(QString)),
                 this, SLOT(SlotCardClicked(QString)));
    }
  }

  bOk = QMetaObject::invokeMethod(this, "SlotUnloadFinished", Qt::QueuedConnection);
  assert(bOk);
  Q_UNUSED(bOk);
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SetSelectionColor(const QColor& color)
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
const QColor& CSceneCardSelectionWidget::SelectionColor()
{
  return m_selectionColor;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::on_pSearchWidget_SignalFilterChanged(const QString& sText)
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
void CSceneCardSelectionWidget::on_pQmlWidget_statusChanged(QQuickWidget::Status status)
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
void CSceneCardSelectionWidget::on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message)
{
  qWarning() << message;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotCardClicked(QString sScene)
{
  m_sSelectedScene = sScene;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotLoadScenesPrivate(qint32 iId, const QStringList& vsScenes)
{
  // TODO;
  m_spUi->pQmlWidget->setSource(QUrl("qrc:/qml/resources/qml/JoipEngine/SceneCardSelection.qml"));

  bool bConnect =
      connect(m_spUi->pQmlWidget->rootObject(), SIGNAL(selectedScene(QString)),
              this, SLOT(SlotCardClicked(QString)));
  assert(bConnect); Q_UNUSED(bConnect)

  if (auto spDbManager = m_wpDbManager.lock())
  {
    QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
    pRootObject->setProperty("selectionColor", QVariant::fromValue(m_selectionColor));
    QJSEngine* pEngine = m_spUi->pQmlWidget->engine();

    tspProject spProject = spDbManager->FindProject(iId);
    if (nullptr != spProject)
    {
      for (const QString& sScene : qAsConst(vsScenes))
      {
        tspScene spScene = spDbManager->FindScene(spProject, sScene);
        CSceneScriptWrapper* pValue = new CSceneScriptWrapper(pEngine, spScene);
        bool bOk = QMetaObject::invokeMethod(pRootObject, "onAddScene",
                                             Q_ARG(QVariant, QVariant::fromValue(pValue)));
        assert(bOk);
        Q_UNUSED(bOk);
      }
    }
  }

  m_bLoadedQml = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotOverlayOpened()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, false);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, false);
  m_spUi->pQmlWidget->setStyleSheet(styleSheet());
  m_spUi->pQmlWidget->setClearColor(m_spUi->pQmlWidget->palette().window().color());
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotOverlayClosed()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  m_spUi->pQmlWidget->setClearColor(Qt::transparent);
  m_spUi->pQmlWidget->setStyleSheet("background-color: transparent;");
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotResizeDone()
{
  if (!m_bLoadedQml) { return; }

  // set size properties manually setzen, since this isn't done automatically
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QSize newSize = m_spUi->pQmlWidget->size();
  if (nullptr != pRootObject)
  {
    pRootObject->setProperty("width", QVariant::fromValue(newSize.width()));
    pRootObject->setProperty("height",QVariant::fromValue(newSize.height()));
    bool bOk = QMetaObject::invokeMethod(pRootObject, "onResize");
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::SlotUnloadFinished()
{
  FinishUnloadPrivate();
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::resizeEvent(QResizeEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    SlotResizeDone();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::FinishUnloadPrivate()
{
  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();
  m_spUi->pQmlWidget->setSource(QUrl());

  QQuickWidget::Status status = m_spUi->pQmlWidget->status();
  assert(status == QQuickWidget::Null);
  Q_UNUSED(status);

  m_bLoadedQml = false;
}

//----------------------------------------------------------------------------------------
//
void CSceneCardSelectionWidget::InitQmlMain()
{
  SlotOverlayClosed();

  QQmlEngine::setObjectOwnership(m_spUi->pQmlWidget->engine(), QQmlEngine::CppOwnership);

  // engine will always take owership of this object
  CDatabaseImageProvider* pProvider = new CDatabaseImageProvider(m_wpDbManager);
  m_spUi->pQmlWidget->engine()->addImageProvider("DataBaseImageProivider", pProvider);
}
