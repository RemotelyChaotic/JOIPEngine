#include "EditorLayoutClassic.h"
#include "Application.h"
#include "ui_EditorLayoutClassic.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"
#include "Editor/EditorWidgets/EditorCodeWidget.h"
#include "Editor/EditorWidgets/EditorResourceWidget.h"
#include "Editor/EditorWidgets/EditorResourceDisplayWidget.h"
#include "Editor/Tutorial/ClassicTutorialStateSwitchHandler.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"
#include <QAction>
#include <map>

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorNamesMap =
  {
    { EEditorWidget::eResourceWidget, "Resource Manager (%1)" },
    { EEditorWidget::eResourceDisplay, "Resource View & Undo Stack (%1)" },
    { EEditorWidget::eProjectSettings, "Project Settings (%1)" },
    { EEditorWidget::eSceneNodeWidget, "Scene Node Editor (%1)" },
    { EEditorWidget::eSceneCodeEditorWidget, "Scene Code Editor (%1)" }
  };

  const std::map<EEditorWidget, QString> m_sEditorKeyBindingMap =
  {
    { EEditorWidget::eResourceWidget, "Resource" },
    { EEditorWidget::eResourceDisplay, "MediaPlayer" },
    { EEditorWidget::eProjectSettings, "Settings" },
    { EEditorWidget::eSceneNodeWidget, "Nodes" },
    { EEditorWidget::eSceneCodeEditorWidget, "Code" }
  };

  const std::map<CEditorActionBar::EActionBarPosition, QString> m_sSideKeyBindingMap =
  {
    { CEditorActionBar::eLeft, "LeftTab" },
    { CEditorActionBar::eRight, "RightTab" }
  };

  const char* c_sEditorProperty = "Editor";
  const char* c_sSideProperty = "Side";

  const QString c_sViewSelectorHelpId =  "Editor/ViewSelector";
}

//----------------------------------------------------------------------------------------
//
CEditorLayoutClassic::CEditorLayoutClassic(QWidget* pParent) :
  CEditorLayoutBase(pParent),
  m_spUi(std::make_shared<Ui::CEditorLayoutClassic>()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vpKeyBindingActions(),
  m_iLastLeftIndex(-1),
  m_iLastRightIndex(-2)
{
  m_spUi->setupUi(this);
}
CEditorLayoutClassic::~CEditorLayoutClassic()
{
  m_vpKeyBindingActions.clear();
  m_spUi->pRightComboBox->blockSignals(true);
  m_spUi->pRightComboBox->clear();
  m_spUi->pRightComboBox->blockSignals(false);
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pLeftComboBox->clear();
  m_spUi->pLeftComboBox->blockSignals(false);

  ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, -1);
  ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, -1);
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::ProjectLoaded(tspProject spCurrentProject, bool bModified)
{
  Q_UNUSED(bModified)

  m_spCurrentProject = spCurrentProject;

  // init indicees
  m_spUi->pRightComboBox->blockSignals(true);
  m_spUi->pRightComboBox->setCurrentIndex(EEditorWidget::eProjectSettings);
  on_pRightComboBox_currentIndexChanged(EEditorWidget::eProjectSettings);
  m_spUi->pRightComboBox->blockSignals(false);

  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pLeftComboBox->setCurrentIndex(EEditorWidget::eResourceWidget);
  on_pLeftComboBox_currentIndexChanged(EEditorWidget::eResourceWidget);
  m_spUi->pLeftComboBox->blockSignals(false);

  SlotKeyBindingsChanged();

  // reload action bars
  ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, 2);
  ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, 0);
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::ProjectUnloaded()
{
  m_spCurrentProject = nullptr;

  // disconnect shortcuts
  for (QAction* pAction : m_vpKeyBindingActions)
  {
    pAction->disconnect();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
                                       CEditorActionBar* pActionBar, qint32 iIndex)
{
  QLayout* pLayout = pContainer->layout();
  while (auto item = pLayout->takeAt(0))
  {
    CEditorWidgetBase* pEditor = qobject_cast<CEditorWidgetBase*>(item->widget());
    pEditor->TakeFromLayout();
    pEditor->OnHidden();
    pEditor->setVisible(false);
    delete item;
  }

  if (-1 < iIndex && pComboBox->count() > iIndex)
  {
    qint32 iEnumValue = pComboBox->itemData(iIndex, Qt::UserRole).toInt();

    QPointer<CEditorWidgetBase> pWidget = GetWidget(EEditorWidget::_from_integral(iEnumValue));
    if (nullptr != pWidget)
    {
      pWidget->setVisible(true);
      pLayout->addWidget(pWidget);
      pWidget->OnShown();
      pWidget->SetActionBar(pActionBar);
    }
  }

  m_spUi->splitter->setSizes({ width() * 1/4 , width() * 3/4 });
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorActionBar> CEditorLayoutClassic::LeftActionBar() const
{
  return m_spUi->pActionBarLeft;
}

//----------------------------------------------------------------------------------------
//
QPointer<QComboBox> CEditorLayoutClassic::LeftComboBox() const
{
  return m_spUi->pLeftComboBox;
}

//----------------------------------------------------------------------------------------
//
QPointer<QWidget> CEditorLayoutClassic::LeftContainer() const
{
  return m_spUi->pLeftContainer;
}

//----------------------------------------------------------------------------------------
//
QPointer<QGroupBox> CEditorLayoutClassic::LeftGroupBox() const
{
  return m_spUi->pLeftPanelGroupBox;
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorActionBar> CEditorLayoutClassic::RightActionBar() const
{
  return m_spUi->pActionBarRight;
}

//----------------------------------------------------------------------------------------
//
QPointer<QComboBox> CEditorLayoutClassic::RightComboBox() const
{
  return m_spUi->pRightComboBox;
}

//----------------------------------------------------------------------------------------
//
QPointer<QWidget> CEditorLayoutClassic::RightContainer() const
{
  return m_spUi->pRightContainer;
}

//----------------------------------------------------------------------------------------
//
QPointer<QGroupBox> CEditorLayoutClassic::RightGroupBox() const
{
  return m_spUi->pRightPanelGroupBox;
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::on_pLeftComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!IsInitialized()) { return; }
  if (m_iLastLeftIndex == iIndex) { return; }

  ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, iIndex);

  if (iIndex == m_spUi->pRightComboBox->currentIndex())
  {
    m_spUi->pRightComboBox->blockSignals(true);
    m_spUi->pRightComboBox->setCurrentIndex(m_iLastLeftIndex);
    ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, m_iLastLeftIndex);
    m_iLastRightIndex = m_iLastLeftIndex;
    m_spUi->pRightComboBox->blockSignals(false);
  }
  m_iLastLeftIndex = iIndex;
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::on_pRightComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!IsInitialized()) { return; }
  if (m_iLastRightIndex == iIndex) { return; }

  ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, iIndex);

  if (iIndex == m_spUi->pLeftComboBox->currentIndex())
  {
    m_spUi->pLeftComboBox->blockSignals(true);
    m_spUi->pLeftComboBox->setCurrentIndex(m_iLastRightIndex);
    ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, m_iLastRightIndex);
    m_iLastLeftIndex = m_iLastRightIndex;
    m_spUi->pLeftComboBox->blockSignals(false);
  }
  m_iLastRightIndex = iIndex;
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::SlotDisplayResource(const QString& sName)
{
  if (!IsInitialized() && nullptr != m_spCurrentProject) { return; }

  CEditorResourceDisplayWidget* pWidget = GetWidget<CEditorResourceDisplayWidget>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pWidget)
  {
    // get resource type
    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
    EResourceType type = EResourceType::eOther;
    if (nullptr != spResource)
    {
      QReadLocker rcLocker(&spResource->m_rwLock);
      type = spResource->m_type;
    }

    // script selected?
    if (EResourceType::eScript == type._to_integral())
    {
      CEditorCodeWidget* pCodeWidget = GetWidget<CEditorCodeWidget>();
      pCodeWidget->LoadResource(spResource);
    }
    // normal resource, just show in resource viewer
    else
    {
      pWidget->UnloadResource();
      pWidget->LoadResource(spResource);
      if (m_spUi->pRightComboBox->itemData(m_spUi->pRightComboBox->currentIndex(), Qt::UserRole).toInt() == EEditorWidget::eResourceDisplay ||
          m_spUi->pLeftComboBox->itemData(m_spUi->pLeftComboBox->currentIndex(), Qt::UserRole).toInt() == EEditorWidget::eResourceDisplay)
      {
        pWidget->UpdateActionBar();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::SlotKeyBindingsChanged()
{
  auto spSettings = CApplication::Instance()->Settings();
  for (QAction* pAction : m_vpKeyBindingActions)
  {
    EEditorWidget widget =
        EEditorWidget::_from_integral(pAction->property(c_sEditorProperty).toInt());
    CEditorActionBar::EActionBarPosition position =
        static_cast<CEditorActionBar::EActionBarPosition>(pAction->property(c_sSideProperty).toInt());

    pAction->disconnect();

    auto itSide = m_sSideKeyBindingMap.find(position);
    auto itKey = m_sEditorKeyBindingMap.find(widget);
    if (m_sSideKeyBindingMap.end() != itSide && m_sEditorKeyBindingMap.end() != itKey)
    {
      qint32 iIndex = widget._to_integral();

      QKeySequence seq = spSettings->keyBinding(QString("%1_%2").arg(itSide->second).arg(itKey->second));
      pAction->setShortcut(seq);
      if (position == CEditorActionBar::eLeft)
      {
        m_spUi->pLeftComboBox->setItemText(iIndex, m_sEditorNamesMap.find(widget)->second.arg(seq.toString()));
        connect(pAction, &QAction::triggered, this, [this, iIndex](){
          if (sender()->property(c_sEditorProperty).toInt() ==
              m_spUi->pLeftComboBox->itemData(iIndex).toInt())
          {
            m_spUi->pLeftComboBox->setCurrentIndex(iIndex);
          }
        });
      }
      else if (position == CEditorActionBar::eRight)
      {
        m_spUi->pRightComboBox->setItemText(iIndex, m_sEditorNamesMap.find(widget)->second.arg(seq.toString()));
        connect(pAction, &QAction::triggered, this, [this, iIndex](){
          if (sender()->property(c_sEditorProperty).toInt() ==
              m_spUi->pRightComboBox->itemData(iIndex).toInt())
          {
            m_spUi->pRightComboBox->setCurrentIndex(iIndex);
          }
        });
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutClassic::InitializeImpl(bool bWithTutorial)
{
  // state switch handler
  if (bWithTutorial)
  {
    m_spStateSwitchHandler =
        std::make_shared<CClassicTutorialStateSwitchHandler>(this, GetTutorialOverlay());
    EditorModel()->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);
  }

  connect(CApplication::Instance()->Settings().get(), &CSettings::keyBindingsChanged,
          this, &CEditorLayoutClassic::SlotKeyBindingsChanged);

  m_spUi->pActionBarLeft->SetActionBarPosition(CEditorActionBar::eLeft);
  m_spUi->pActionBarLeft->Initialize();
  m_spUi->pActionBarRight->SetActionBarPosition(CEditorActionBar::eRight);
  m_spUi->pActionBarRight->Initialize();

  // initialize action map
  for (qint32 i = CEditorActionBar::eLeft; CEditorActionBar::eRight+1 > i; ++i)
  {
    for (EEditorWidget eval : EEditorWidget::_values())
    {
      QAction* pAction = new QAction(this);
      pAction->setProperty(c_sEditorProperty, eval._to_integral());
      pAction->setProperty(c_sSideProperty, static_cast<qint32>(i));
      m_vpKeyBindingActions.push_back(pAction);
      addAction(pAction);
    }
  }

  // initialize widgets
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pRightComboBox->blockSignals(true);

  VisitWidgets([this](QPointer<CEditorWidgetBase> pWidget, EEditorWidget type) {
    Q_UNUSED(pWidget)

    // Key-bindings will be set later
    m_spUi->pLeftComboBox->addItem(m_sEditorNamesMap.find(type)->second.arg(""),
                                   type._to_integral());
    m_spUi->pRightComboBox->addItem(m_sEditorNamesMap.find(type)->second.arg(""),
                                    type._to_integral());
  });
  m_spUi->pLeftComboBox->blockSignals(false);
  m_spUi->pRightComboBox->blockSignals(false);

  // custom stuff
  connect(GetWidget<CEditorResourceWidget>(), &CEditorResourceWidget::SignalResourceSelected,
          this, &CEditorLayoutClassic::SlotDisplayResource);

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pLeftComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
    wpHelpFactory->RegisterHelp(c_sViewSelectorHelpId, ":/resources/help/editor/selection_combobox_help.html");
    m_spUi->pRightComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
  }
}
