#include "EditorLayoutBase.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgets/EditorCodeWidget.h"
#include "Editor/EditorWidgets/EditorProjectSettingsWidget.h"
#include "Editor/EditorWidgets/EditorResourceDisplayWidget.h"
#include "Editor/EditorWidgets/EditorResourceWidget.h"
#include "Editor/EditorWidgets/EditorSceneNodeWidget.h"
#include "Editor/Tutorial/EditorTutorialOverlay.h"
#include <QPointer>

CEditorLayoutBase::CEditorLayoutBase(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spStateSwitchHandler(nullptr),
  m_pLayoutViewProvider(),
  m_pEditorModel(nullptr),
  m_bWithTutorial(false)
{

}

CEditorLayoutBase::~CEditorLayoutBase()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutBase::Initialize(std::weak_ptr<IEditorLayoutViewProvider> pLayoutViewProvider,
                                   QPointer<CEditorModel> pEditorModel,
                                   bool bWithTutorial)
{
  m_pLayoutViewProvider = pLayoutViewProvider;
  m_pEditorModel = pEditorModel;
  m_bWithTutorial = bWithTutorial;
  Initialize();
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorModel> CEditorLayoutBase::EditorModel() const
{
  return m_pEditorModel;
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorTutorialOverlay> CEditorLayoutBase::GetTutorialOverlay() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return spLayoutProvider->GetTutorialOverlay();
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
template<>
QPointer<CEditorResourceWidget> CEditorLayoutBase::GetWidget<CEditorResourceWidget, true>() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return qobject_cast<CEditorResourceWidget*>(
          spLayoutProvider->GetEditorWidget(EEditorWidget::eResourceWidget).data());
  }
  return nullptr;
}
template<>
QPointer<CEditorResourceDisplayWidget> CEditorLayoutBase::GetWidget<CEditorResourceDisplayWidget, true>() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return qobject_cast<CEditorResourceDisplayWidget*>(
          spLayoutProvider->GetEditorWidget(EEditorWidget::eResourceDisplay).data());
  }
  return nullptr;
}
template<>
QPointer<CEditorProjectSettingsWidget> CEditorLayoutBase::GetWidget<CEditorProjectSettingsWidget, true>() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return qobject_cast<CEditorProjectSettingsWidget*>(
          spLayoutProvider->GetEditorWidget(EEditorWidget::eProjectSettings).data());
  }
  return nullptr;
}
template<>
QPointer<CEditorSceneNodeWidget> CEditorLayoutBase::GetWidget<CEditorSceneNodeWidget, true>() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return qobject_cast<CEditorSceneNodeWidget*>(
          spLayoutProvider->GetEditorWidget(EEditorWidget::eSceneNodeWidget).data());
  }
  return nullptr;
}
template<>
QPointer<CEditorCodeWidget> CEditorLayoutBase::GetWidget<CEditorCodeWidget, true>() const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return qobject_cast<CEditorCodeWidget*>(
          spLayoutProvider->GetEditorWidget(EEditorWidget::eSceneCodeEditorWidget).data());
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorWidgetBase> CEditorLayoutBase::GetWidget(EEditorWidget widget) const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return spLayoutProvider->GetEditorWidget(widget);
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutBase::VisitWidgets(
    const std::function<void(QPointer<CEditorWidgetBase>, EEditorWidget)>& fnVisitor)
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    spLayoutProvider->VisitWidgets(fnVisitor);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorLayoutBase::Initialize()
{
  SetInitialized(false);
  InitializeImpl(m_bWithTutorial);
  SetInitialized(true);
}
