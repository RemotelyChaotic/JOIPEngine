#include "ProjectCardSelectionWidget.h"
#include "Application.h"
#include "Constants.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_ProjectCardSelectionWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QObject>
#include <QMouseEvent>
#include <QMovie>
#include <cassert>

CProjectCardSelectionWidget::CProjectCardSelectionWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CProjectCardSelectionWidget>()),
  m_spSpinner(std::make_shared<QMovie>("://resources/gif/spinner_transparent.gif")),
  m_pLastSelectedWidget(nullptr),
  m_iSelectedProjectId(-1)
{
  m_spUi->setupUi(this);
  Initialize();
}

CProjectCardSelectionWidget::~CProjectCardSelectionWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::LoadProjects()
{
  m_iSelectedProjectId = -1;
  m_pLastSelectedWidget = nullptr;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    std::set<qint32, std::less<qint32>> ids = spDbManager->ProjectIds();
    for (auto it = ids.begin(); ids.end() != it; ++it)
    {
      tspProject spProject = spDbManager->FindProject(*it);
      if (nullptr != spProject)
      {
        QLayout* pLayout = m_spUi->pScrollAreaWidgetContents->layout();
        assert(nullptr != pLayout);

        spProject->m_rwLock.lockForRead();
        const QString sName = spProject->m_sTitleCard;
        const qint32 iId = spProject->m_iId;
        spProject->m_rwLock.unlock();

        auto spResource = spDbManager->FindResource(spProject, sName);
        bool bIsImage = true;
        if (nullptr != spResource)
        {
          spResource->m_rwLock.lockForRead();
          bIsImage = spResource->m_type._to_integral() == EResourceType::eImage;
          spResource->m_rwLock.unlock();
        }

        CResourceDisplayWidget* pWidget = new CResourceDisplayWidget(m_spUi->pScrollAreaWidgetContents);
        pWidget->setFixedSize(QSize(300, 400));
        pWidget->SetProjectId(iId);
        connect(pWidget, &CResourceDisplayWidget::OnClick,
                this, &CProjectCardSelectionWidget::SlotCardClicked);
        if (bIsImage)
        {
          pWidget->LoadResource(spResource);
        }
        pWidget->installEventFilter(this);
        AddDropShadow(pWidget, Qt::black);
        pLayout->addWidget(pWidget);

        if (-1 == m_iSelectedProjectId)
        {
          m_iSelectedProjectId = iId;
          m_pLastSelectedWidget = pWidget;
          AddDropShadow(pWidget, QColor(BRIGHT_PURPLE));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::UnloadProjects()
{
  m_iSelectedProjectId = -1;
  m_pLastSelectedWidget = nullptr;

  QLayout* pLayout = m_spUi->pScrollAreaWidgetContents->layout();
  assert(nullptr != pLayout);
  while (auto item = pLayout->takeAt(0))
  {
    delete item->widget();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SlotCardClicked()
{
  AddDropShadow(m_pLastSelectedWidget, Qt::black);
  CResourceDisplayWidget* pCardWidget = dynamic_cast<CResourceDisplayWidget*>(sender());
  m_iSelectedProjectId = pCardWidget->ProjectId();
  m_pLastSelectedWidget = pCardWidget;
  AddDropShadow(pCardWidget, QColor(BRIGHT_PURPLE));
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::AddDropShadow(QWidget* pWidget, const QColor& color)
{
  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pWidget);
  pShadow->setBlurRadius(5);
  pShadow->setXOffset(0);
  pShadow->setYOffset(0);
  pShadow->setColor(color);
  pWidget->setGraphicsEffect(pShadow);
}
