#include "ProjectCardSelectionWidget.h"
#include "Application.h"
#include "ProjectCardWidget.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "ui_ProjectCardSelectionWidget.h"
#include <QMovie>
#include <cassert>

CProjectCardSelectionWidget::CProjectCardSelectionWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CProjectCardSelectionWidget>()),
  m_spSpinner(std::make_shared<QMovie>("://resources/gif/spinner_transparent.gif")),
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

        CProjectCardWidget* pWidget =
          new CProjectCardWidget(m_spSpinner, m_spUi->pScrollAreaWidgetContents);
        pWidget->Initialize(spProject);
        connect(pWidget, &CProjectCardWidget::Clicked,
                this, &CProjectCardSelectionWidget::SlotCardClicked);
        pLayout->addWidget(pWidget);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardSelectionWidget::UnloadProjects()
{
  m_iSelectedProjectId = -1;

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
  CProjectCardWidget* pCardWidget = dynamic_cast<CProjectCardWidget*>(sender());
  m_iSelectedProjectId = pCardWidget->ProjectId();
}
