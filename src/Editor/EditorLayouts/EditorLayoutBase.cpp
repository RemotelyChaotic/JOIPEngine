#include "EditorLayoutBase.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgets/EditorWidgetBase.h"
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
QPointer<CEditorWidgetBase> CEditorLayoutBase::GetWidget(EEditorWidget widget) const
{
  if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
  {
    return spLayoutProvider->GetEditorWidget(widget);
  }
  return QPointer<CEditorWidgetBase>(nullptr);
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
