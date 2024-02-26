#include "CodeDisplayWidget.h"
#include "Application.h"
#include "BackgroundSnippetOverlay.h"
#include "CodeDisplayDefaultEditorImpl.h"
#include "CodeDisplayEosEditorImpl.h"
#include "CodeDisplayLayoutEditorImpl.h"
#include "CommandScriptContentChange.h"
#include "DeviceSnippetOverlay.h"
#include "IconSnippetOverlay.h"
#include "NotificationSnippetOverlay.h"
#include "MetronomeSnippetOverlay.h"
#include "ResourceSnippetOverlay.h"
#include "TextSnippetOverlay.h"
#include "TimerSnippetOverlay.h"
#include "ThreadSnippetOverlay.h"
#include "ui_CodeDisplayWidget.h"
#include "ui_EditorActionBar.h"

#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorModel.h"
#include "Editor/Resources/ResourceTreeItemModel.h"

#include "Systems/HelpFactory.h"
#include "Systems/Project.h"

#include "Utils/UndoRedoFilter.h"
#include "Widgets/HelpOverlay.h"

#include <QUndoStack>

namespace
{
  const qint32 c_iIndexNoScripts    = 0;
  const qint32 c_iIndexScriptJs     = 1;
  const qint32 c_iIndexScriptEos    = 2;
  const qint32 c_iIndexScriptLua    = 1;
  const qint32 c_iIndexScriptQml    = 1;
  const qint32 c_iIndexScriptLayout = 1;

  const QString c_sSciptEditorHelpId =      "Editor/SciptEditor";
  const QString c_sSciptAPIHelpId =         "Editor/SciptAPI/Editor";
  const QString c_sEOSSciptEditorHelpId =   "Editor/EOS/SciptEditor";
  const QString c_sEvalSciptAPIHelpId =     "Editor/SciptAPI/Editor";

  //--------------------------------------------------------------------------------------
  //
  qint32 PageIndexFromScriptType(const QString& sType)
  {
    static std::map<QString, qint32> typeToPageMap = {
      { SScriptDefinitionData::c_sScriptTypeJs, c_iIndexScriptJs },
      { SScriptDefinitionData::c_sScriptTypeEos, c_iIndexScriptEos },
      { SScriptDefinitionData::c_sScriptTypeLua, c_iIndexScriptLua },
      { SScriptDefinitionData::c_sScriptTypeQml, c_iIndexScriptQml },
      { SScriptDefinitionData::c_sScriptTypeLayout, c_iIndexScriptLayout }
    };
    if (auto it = typeToPageMap.find(sType); typeToPageMap.end() != it)
    {
      return it->second;
    }
    return c_iIndexNoScripts;
  }
}

//----------------------------------------------------------------------------------------
//
CCodeDisplayWidget::CCodeDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CCodeDisplayWidget>()),
  m_displayImplMap(),
  m_spBackgroundSnippetOverlay(nullptr),
  m_spDeviceSnippetOverlay(nullptr),
  m_spIconSnippetOverlay(nullptr),
  m_spMetronomeSnippetOverlay(nullptr),
  m_spNotificationSnippetOverlay(nullptr),
  m_spResourceSnippetOverlay(nullptr),
  m_spTextSnippetOverlay(nullptr),
  m_spTimerSnippetOverlay(nullptr),
  m_spThreadSnippetOverlay(nullptr)
{
  m_spUi->setupUi(this);

  m_displayImplMap[SScriptDefinitionData::c_sScriptTypeJs] = std::make_unique<CCodeDisplayDefaultEditorImpl>(m_spUi->pCodeEdit);
  m_displayImplMap[SScriptDefinitionData::c_sScriptTypeEos] = std::make_unique<CCodeDisplayEosEditorImpl>(m_spUi->pEosEdit);
  m_displayImplMap[SScriptDefinitionData::c_sScriptTypeLua] = std::make_unique<CCodeDisplayDefaultEditorImpl>(m_spUi->pCodeEdit);
  m_displayImplMap[SScriptDefinitionData::c_sScriptTypeQml] = std::make_unique<CCodeDisplayLayoutEditorImpl>(m_spUi->pCodeEdit);

  m_spBackgroundSnippetOverlay = std::make_unique<CBackgroundSnippetOverlay>(this);
  m_spDeviceSnippetOverlay = std::make_unique<CDeviceSnippetOverlay>(this);
  m_spIconSnippetOverlay = std::make_unique<CIconSnippetOverlay>(this);
  m_spMetronomeSnippetOverlay = std::make_unique<CMetronomeSnippetOverlay>(this);
  m_spNotificationSnippetOverlay = std::make_unique<CNotificationSnippetOverlay>(this);
  m_spResourceSnippetOverlay = std::make_unique<CResourceSnippetOverlay>(this);
  m_spTextSnippetOverlay = std::make_unique<CTextSnippetOverlay>(this);
  m_spTimerSnippetOverlay = std::make_unique<CTimerSnippetOverlay>(this);
  m_spThreadSnippetOverlay = std::make_unique<CThreadSnippetOverlay>(this);

  connect(m_spUi->pCodeEdit->document(), &QTextDocument::contentsChange,
          this, &CCodeDisplayWidget::SignalContentsChange);
  connect(m_spUi->pEosEdit, &CEosScriptEditorView::SignalContentsChange,
          this, &CCodeDisplayWidget::SignalContentsChange);
}

CCodeDisplayWidget::~CCodeDisplayWidget()
{
  m_spThreadSnippetOverlay.reset();
  m_spTimerSnippetOverlay.reset();
  m_spTextSnippetOverlay.reset();
  m_spResourceSnippetOverlay.reset();
  m_spNotificationSnippetOverlay.reset();
  m_spMetronomeSnippetOverlay.reset();
  m_spIconSnippetOverlay.reset();
  m_spDeviceSnippetOverlay.reset();
  m_spBackgroundSnippetOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::Initialize(QPointer<CEditorModel> pEditorModel,
                                    QPointer<CResourceTreeItemModel> pResourceTreeModel,
                                    QPointer<QUndoStack> pUndoStack)
{
  m_pInitialized = false;

  m_pEditorModel = pEditorModel;
  m_pResourceTreeModel = pResourceTreeModel;
  m_pUndoStack = pUndoStack;

  for (auto& [sKey, spDisplayI] : m_displayImplMap)
  {
    Q_UNUSED(sKey)
    spDisplayI->Initialize(m_pEditorModel);
  }

  m_spBackgroundSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spIconSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spMetronomeSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spNotificationSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spResourceSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spTextSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spThreadSnippetOverlay->Initialize(m_pResourceTreeModel);

  m_spBackgroundSnippetOverlay->Hide();
  m_spDeviceSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spMetronomeSnippetOverlay->Hide();
  m_spNotificationSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();

  connect(m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spDeviceSnippetOverlay.get(), &CDeviceSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spMetronomeSnippetOverlay.get(), &CMetronomeSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spNotificationSnippetOverlay.get(), &CNotificationSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::SignalCodeGenerated,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pCodeEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSciptEditorHelpId);
    wpHelpFactory->RegisterHelp(c_sSciptEditorHelpId, ":/resources/help/editor/code/script_editor_help.html");
    wpHelpFactory->RegisterHelp(c_sSciptAPIHelpId, ":/resources/help/editor/code/script_reference_help.html");
    m_spUi->pEosEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEOSSciptEditorHelpId);
    wpHelpFactory->RegisterHelp(c_sEOSSciptEditorHelpId, ":/resources/help/editor/code/eos_editor_help.html");
    wpHelpFactory->RegisterHelp(c_sEvalSciptAPIHelpId, ":/resources/help/editor/code/eval_api_reference_help.html");
  }

  CUndoRedoFilter* pFilter =
      new CUndoRedoFilter(m_spUi->pCodeEdit,
                          std::bind(&CScriptEditorWidget::CreateContextMenu, m_spUi->pCodeEdit));
  connect(m_spUi->pCodeEdit->document(), &QTextDocument::undoCommandAdded,
          this, &CCodeDisplayWidget::SlotUndoForScriptContentAdded);
  connect(pFilter, &CUndoRedoFilter::UndoTriggered, this, [this]() { m_pUndoStack->undo(); });
  connect(pFilter, &CUndoRedoFilter::RedoTriggered, this, [this]() { m_pUndoStack->redo(); });

  m_pInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::LoadProject(tspProject spProject)
{
  m_spResourceSnippetOverlay->LoadProject(spProject);
  m_spMetronomeSnippetOverlay->LoadProject(spProject);
  m_spNotificationSnippetOverlay->LoadProject(spProject);
  m_spTextSnippetOverlay->LoadProject(spProject);

  bool bReadOnly = m_pEditorModel->IsReadOnly();
  m_spUi->pCodeEdit->setReadOnly(bReadOnly);
  m_spUi->pEosEdit->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::UnloadProject()
{
  m_spUi->pCodeEdit->blockSignals(true);
  m_spUi->pCodeEdit->document()->blockSignals(true);
  m_spUi->pCodeEdit->ResetAddons();
  m_spUi->pCodeEdit->clear();
  m_spUi->pCodeEdit->document()->blockSignals(false);
  m_spUi->pCodeEdit->blockSignals(false);
  m_spUi->pCodeEdit->setReadOnly(false);

  m_spResourceSnippetOverlay->UnloadProject();
  m_spMetronomeSnippetOverlay->UnloadProject();
  m_spNotificationSnippetOverlay->UnloadProject();
  m_spTextSnippetOverlay->UnloadProject();

  m_spBackgroundSnippetOverlay->Hide();
  m_spDeviceSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spMetronomeSnippetOverlay->Hide();
  m_spNotificationSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SaveProject()
{

}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::OnActionBarAboutToChange(std::unique_ptr<Ui::CEditorActionBar>* pspUiActionBar)
{
  m_pspUiActionBar = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::OnActionBarChanged(std::unique_ptr<Ui::CEditorActionBar>* pspUiActionBar)
{
  m_pspUiActionBar = pspUiActionBar;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::Clear()
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->Clear();
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::ResetWidget()
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->ResetWidget();
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetContent(const QString& sContent)
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->SetContent(sContent);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetHighlightDefinition(const QString& sType)
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->SetHighlightDefinition(sType);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetScriptType(const QString& sScriptType)
{
  if (!m_pInitialized) { return; }

  auto it = m_displayImplMap.find(m_sScriptType);
  if (nullptr != m_pspUiActionBar)
  {
    if (m_displayImplMap.end() != it)
    {
      it->second->HideButtons((*m_pspUiActionBar).get());
    }
  }

  m_sScriptType = sScriptType;

  it = m_displayImplMap.find(m_sScriptType);
  if (nullptr != m_pspUiActionBar)
  {
    if (m_displayImplMap.end() != it)
    {
      it->second->ShowButtons((*m_pspUiActionBar).get());
    }
  }

  m_spBackgroundSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spDeviceSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spIconSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spMetronomeSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spNotificationSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spResourceSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spTextSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spTimerSnippetOverlay->SetCurrentScriptType(m_sScriptType);
  m_spThreadSnippetOverlay->SetCurrentScriptType(m_sScriptType);

  m_spUi->pEditorStackedWidget->setCurrentIndex(PageIndexFromScriptType(sScriptType));
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::Update()
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->Update();
  }
}

//----------------------------------------------------------------------------------------
//
QString CCodeDisplayWidget::GetCurrentText() const
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    return it->second->GetCurrentText();
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotExecutionError(QString sException, qint32 iLine, QString sStack)
{
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->ExecutionError(sException, iLine, sStack);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotShowOverlay(const QWidget* pWidget)
{
  if (!m_pInitialized || nullptr == m_pspUiActionBar) { return; }

  if (pWidget == (*m_pspUiActionBar)->AddShowBackgroundCode)
  {
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spBackgroundSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddDeviceCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddShowIconCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddShowImageCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddTextCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddMetronomeCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddNotificationCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddTimerCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Toggle();
  }
  else if (pWidget == (*m_pspUiActionBar)->AddThreadCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spDeviceSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Toggle();
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotInsertGeneratedCode(const QString& sCode)
{
  if (!m_pInitialized) { return; }
  auto it = m_displayImplMap.find(m_sScriptType);
  if (m_displayImplMap.end() != it)
  {
    it->second->InsertGeneratedCode(sCode);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotUndoForScriptContentAdded()
{
  if (!m_pInitialized) { return; }
  m_pUndoStack->push(new CCommandScriptContentChange(m_spUi->pCodeEdit->document()));
}
