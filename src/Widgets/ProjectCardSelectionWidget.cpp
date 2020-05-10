#include "ProjectCardSelectionWidget.h"
#include "Application.h"
#include "Constants.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
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
  m_selectionColor(Qt::white),
  m_iSelectedProjectId(-1)
{
  m_spUi->setupUi(this);
  Initialize();
}

CProjectCardSelectionWidget::~CProjectCardSelectionWidget()
{
  UnloadProjects();

  delete m_spUi->pQmlWidget;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    wpHelpFactory->RegisterHelp(c_sCardHelpId, ":/resources/help/player/card_button_help.html");
    m_spUi->pQmlWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCardHelpId);
    wpHelpFactory->RegisterHelp(c_sSearchBarHelpId, ":/resources/help/editor/resources/filter_widget_help.html");
    m_spUi->pSearchWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSearchBarHelpId);
  }

  InitQmlMain();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::LoadProjects()
{
  m_iSelectedProjectId = -1;

  m_spUi->pQmlWidget->setSource(QUrl("qrc:/qml/resources/qml/ProjectCardSelection.qml"));

  connect(m_spUi->pQmlWidget->rootObject(), SIGNAL(selectedProjectIndex(int)),
          this, SLOT(SlotCardClicked(int)));

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  pRootObject->setProperty("selectionColor", QVariant::fromValue(m_selectionColor));

  QJSEngine* pEngine = m_spUi->pQmlWidget->engine();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    std::set<qint32, std::less<qint32>> ids = spDbManager->ProjectIds();
    for (auto it = ids.begin(); ids.end() != it; ++it)
    {
      tspProject spProject = spDbManager->FindProject(*it);
      if (nullptr != spProject)
      {
        if (-1 == m_iSelectedProjectId)
        {
          QReadLocker locker(&spProject->m_rwLock);
          m_iSelectedProjectId = spProject->m_iId;
        }

        CProject* pValue = new CProject(pEngine, spProject);
        m_spUi->pQmlWidget->rootObject()->setProperty("currentlyAddedProject", QVariant::fromValue(pValue));
        QMetaObject::invokeMethod(pRootObject, "onAddProject");
        m_vpProjects.push_back(pValue);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::UnloadProjects()
{
  m_iSelectedProjectId = -1;

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "onUnLoad");

  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();

  for (CProject* pProject : qAsConst(m_vpProjects))
  {
    delete pProject;
  }
  m_vpProjects.clear();

  disconnect(m_spUi->pQmlWidget->rootObject(), SIGNAL(selectedProjectIndex(int)),
             this, SLOT(SlotCardClicked(int)));
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SetSelectionColor(const QColor& color)
{
  if (m_selectionColor != color)
  {
    m_selectionColor = color;

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
void CProjectCardSelectionWidget::SlotResizeDone()
{
  // set size properties manually setzen, since this isn't done automatically
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QSize newSize = size() -
      QSize(contentsMargins().left() +
            contentsMargins().right(),
            contentsMargins().top() +
            contentsMargins().bottom()) -
      QSize(0, m_spUi->pSearchWidget->size().height() + layout()->spacing());
  if (nullptr != pRootObject)
  {
    pRootObject->setProperty("width", QVariant::fromValue(newSize.width()));
    pRootObject->setProperty("height",QVariant::fromValue(newSize.height()));
    QMetaObject::invokeMethod(pRootObject, "onResize");
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotTriedToLoadMovie(const QString& sMovie)
{
  Q_UNUSED(sMovie);
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::resizeEvent(QResizeEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    QMetaObject::invokeMethod(this, "SlotResizeDone", Qt::QueuedConnection);
  }
  pEvent->accept();
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::InitQmlMain()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  m_spUi->pQmlWidget->setClearColor(Qt::transparent);
  m_spUi->pQmlWidget->setStyleSheet("background-color: transparent;");

  QQmlEngine::setObjectOwnership(m_spUi->pQmlWidget->engine(), QQmlEngine::CppOwnership);

  // engine will allways take owership of this object
  CDatabaseImageProvider* pProvider = new CDatabaseImageProvider(m_wpDbManager);
  connect(pProvider, &CDatabaseImageProvider::SignalTriedToLoadMovie,
          this, &CProjectCardSelectionWidget::SlotTriedToLoadMovie, Qt::QueuedConnection);
  m_spUi->pQmlWidget->engine()->addImageProvider("DataBaseImageProivider", pProvider);
}

