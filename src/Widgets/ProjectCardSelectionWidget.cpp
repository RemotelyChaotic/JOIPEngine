#include "ProjectCardSelectionWidget.h"
#include "Application.h"
#include "Constants.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_ProjectCardSelectionWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <QMovie>
#include <QPropertyAnimation>
#include <cassert>

CProjectCardSelectionWidget::CProjectCardSelectionWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CProjectCardSelectionWidget>()),
  m_spSpinner(std::make_shared<QMovie>("://resources/gif/spinner_transparent.gif")),
  m_pLastSelectedWidget(nullptr),
  m_selectionColor(Qt::white),
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
        const bool bUsesWeb = spProject->m_bUsesWeb;
        const bool bUsesCodecs = spProject->m_bNeedsCodecs;
        spProject->m_rwLock.unlock();

        auto spResource = spDbManager->FindResource(spProject, sName);
        bool bIsImage = true;
        if (nullptr != spResource)
        {
          spResource->m_rwLock.lockForRead();
          bIsImage = spResource->m_type._to_integral() == EResourceType::eImage;
          spResource->m_rwLock.unlock();
        }

        // construct a single card
        QWidget* pRoot = new QWidget(m_spUi->pScrollArea);
        pRoot->setFixedSize(QSize(320, 420));

        CResourceDisplayWidget* pWidget = new CResourceDisplayWidget(pRoot);
        pWidget->setMinimumSize(QSize(300, 400));
        pWidget->setMaximumSize(QSize(300, 400));
        pWidget->setGeometry(10, 10, 300, 400);
        pWidget->SetProjectId(iId);
        connect(pWidget, &CResourceDisplayWidget::OnClick,
                this, &CProjectCardSelectionWidget::SlotCardClicked);
        if (bIsImage)
        {
          pWidget->LoadResource(spResource);
        }
        pWidget->installEventFilter(this);

        AddDropShadow(pWidget, Qt::black);
        if (-1 == m_iSelectedProjectId)
        {
          m_iSelectedProjectId = iId;
          m_pLastSelectedWidget = pWidget;
          AddDropShadow(pWidget, m_selectionColor);
          pWidget->setGeometry(10, 10, 300, 400);
        }

        qint32 iXOffset = 20;
        if (bUsesWeb)
        {
          QLabel* pWebIcon = new QLabel("", pRoot);
          pWebIcon->setFixedSize(48, 48);
          pWebIcon->setGeometry(iXOffset, pRoot->size().height() - 68, 48, 48);
          pWebIcon->setScaledContents(true);
          pWebIcon->setPixmap(QPixmap("://resources/img/ButtonWeb.png"));
          pWebIcon->setToolTip(tr("Uses web resources and requires an internet connection."));
          iXOffset += 58;
        }
        if (bUsesCodecs)
        {
          QLabel* pMediaIcon = new QLabel("", pRoot);
          pMediaIcon->setFixedSize(48, 48);
          pMediaIcon->setGeometry(iXOffset, pRoot->size().height() - 68, 48, 48);
          pMediaIcon->setScaledContents(true);
          pMediaIcon->setPixmap(QPixmap("://resources/img/ButtonImage.png"));
          pMediaIcon->setToolTip(tr("Uses media resources and might be loud."));
          iXOffset += 58;
        }

        // add constructed card to layout
        pLayout->addWidget(pRoot);
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
    if (nullptr != item)
    {
      delete item->widget();
    }
    delete item;
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::SetSelectionColor(const QColor& color)
{
  if (m_selectionColor != color)
  {
    m_selectionColor = color;
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
void CProjectCardSelectionWidget::SlotCardClicked()
{
  AddDropShadow(m_pLastSelectedWidget, Qt::black);
  CResourceDisplayWidget* pCardWidget = dynamic_cast<CResourceDisplayWidget*>(sender());
  m_iSelectedProjectId = pCardWidget->ProjectId();
  m_pLastSelectedWidget = pCardWidget;
  AddDropShadow(pCardWidget, m_selectionColor);
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::AddDropShadow(QWidget* pWidget, const QColor& color)
{
  if (nullptr == pWidget) { return; }

  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pWidget);
  pShadow->setBlurRadius(5);
  pShadow->setXOffset(0);
  pShadow->setYOffset(0);
  pShadow->setColor(color);
  pWidget->setGraphicsEffect(pShadow);
}
