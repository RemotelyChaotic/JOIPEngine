#include "CodeDisplayWidget.h"
#include "Application.h"
#include "BackgroundSnippetOverlay.h"
#include "CommandScriptContentChange.h"
#include "IconSnippetOverlay.h"
#include "NotificationSnippetOverlay.h"
#include "MetronomeSnippetOverlay.h"
#include "ResourceSnippetOverlay.h"
#include "ScriptEditorModel.h"
#include "TextSnippetOverlay.h"
#include "TimerSnippetOverlay.h"
#include "ThreadSnippetOverlay.h"
#include "ui_CodeDisplayWidget.h"
#include "ui_EditorActionBar.h"

#include "Editor/EditorModel.h"
#include "Editor/Resources/ResourceTreeItemModel.h"

#include "Systems/HelpFactory.h"
#include "Systems/Project.h"

#include "Utils/UndoRedoFilter.h"
#include "Widgets/HelpOverlay.h"

#include <QUndoStack>

namespace
{
  const qint32 c_iIndexNoScripts = 0;
  const qint32 c_iIndexScriptJs  = 1;
  const qint32 c_iIndexScriptEos = 2;

  const QString c_sSciptEditorHelpId =      "Editor/SciptEditor";
  const QString c_sSciptAPIHelpId =         "Editor/SciptAPI/Editor";

  //--------------------------------------------------------------------------------------
  //
  qint32 PageIndexFromScriptType(const QString& sType)
  {
    return c_iIndexScriptJs;
    /*
    static std::map<QString, qint32> typeToPageMap = {
      { "js", c_iIndexScriptJs },
      { "eos", c_iIndexScriptEos }
    };
    if (auto it = typeToPageMap.find(sType); typeToPageMap.end() != it)
    {
      return it->second;
    }
    return c_iIndexNoScripts;
    */
  }
}

//----------------------------------------------------------------------------------------
//
CCodeDisplayWidget::CCodeDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CCodeDisplayWidget>()),
  m_spBackgroundSnippetOverlay(nullptr),
  m_spIconSnippetOverlay(nullptr),
  m_spMetronomeSnippetOverlay(nullptr),
  m_spNotificationSnippetOverlay(nullptr),
  m_spResourceSnippetOverlay(nullptr),
  m_spTextSnippetOverlay(nullptr),
  m_spTimerSnippetOverlay(nullptr),
  m_spThreadSnippetOverlay(nullptr)
{
  m_spUi->setupUi(this);

  m_spBackgroundSnippetOverlay = std::make_unique<CBackgroundSnippetOverlay>(this);
  m_spIconSnippetOverlay = std::make_unique<CIconSnippetOverlay>(this);
  m_spMetronomeSnippetOverlay = std::make_unique<CMetronomeSnippetOverlay>(this);
  m_spNotificationSnippetOverlay = std::make_unique<CNotificationSnippetOverlay>(this);
  m_spResourceSnippetOverlay = std::make_unique<CResourceSnippetOverlay>(this);
  m_spTextSnippetOverlay = std::make_unique<CTextSnippetOverlay>(this);
  m_spTimerSnippetOverlay = std::make_unique<CTimerSnippetOverlay>(this);
  m_spThreadSnippetOverlay = std::make_unique<CThreadSnippetOverlay>(this);

  connect(m_spUi->pCodeEdit->document(), &QTextDocument::contentsChange,
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
  m_spBackgroundSnippetOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::Initialize(QPointer<CEditorModel> pEditorModel,
                                    QPointer<CScriptEditorModel> pScriptEditorModel,
                                    QPointer<CResourceTreeItemModel> pResourceTreeModel,
                                    QPointer<QUndoStack> pUndoStack)
{
  m_pInitialized = false;

  m_pEditorModel = pEditorModel;
  m_pScriptEditorModel = pScriptEditorModel;
  m_pResourceTreeModel = pResourceTreeModel;
  m_pUndoStack = pUndoStack;

  m_spBackgroundSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spIconSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spMetronomeSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spNotificationSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spResourceSnippetOverlay->Initialize(m_pResourceTreeModel);
  m_spTextSnippetOverlay->Initialize(m_pResourceTreeModel);

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spMetronomeSnippetOverlay->Hide();
  m_spNotificationSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();

  connect(m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::SignalBackgroundCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::SignalIconCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spMetronomeSnippetOverlay.get(), &CMetronomeSnippetOverlay::SignalMetronomeSnippetCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spNotificationSnippetOverlay.get(), &CNotificationSnippetOverlay::SignalNotificationSnippetCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::SignalResourceCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::SignalTextSnippetCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::SignalTimerCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);
  connect(m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::SignalThreadCode,
          this, &CCodeDisplayWidget::SlotInsertGeneratedCode);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pCodeEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSciptEditorHelpId);
    wpHelpFactory->RegisterHelp(c_sSciptEditorHelpId, ":/resources/help/editor/code/script_editor_help.html");

    wpHelpFactory->RegisterHelp(c_sSciptAPIHelpId, ":/resources/help/editor/code/script_reference_help.html");
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

  m_spUi->pCodeEdit->setReadOnly(m_pEditorModel->IsReadOnly());
  //m_spUi->pEosEdit;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::UnloadProject()
{
  m_spUi->pCodeEdit->blockSignals(true);
  m_spUi->pCodeEdit->document()->blockSignals(true);
  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->clear();
  m_spUi->pCodeEdit->document()->blockSignals(false);
  m_spUi->pCodeEdit->blockSignals(false);
  m_spUi->pCodeEdit->setReadOnly(false);

  m_spResourceSnippetOverlay->UnloadProject();
  m_spMetronomeSnippetOverlay->UnloadProject();
  m_spNotificationSnippetOverlay->UnloadProject();
  m_spTextSnippetOverlay->UnloadProject();

  m_spBackgroundSnippetOverlay->Hide();
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
  m_spUi->pCodeEdit->clear();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::ResetWidget()
{
  m_spUi->pCodeEdit->ResetWidget();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetContent(const QString& sContent)
{
  m_spUi->pCodeEdit->setPlainText(sContent);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetHighlightDefinition(const QString& sType)
{
  m_spUi->pCodeEdit->SetHighlightDefinition(sType);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SetScriptType(const QString& sScriptType)
{
  if (!m_pInitialized || nullptr == m_pspUiActionBar) { return; }

  // do this for now, later use a registry
  if ("js" == sScriptType)
  {
    (*m_pspUiActionBar)->AddShowBackgroundCode->setVisible(true);
    (*m_pspUiActionBar)->AddShowIconCode->setVisible(true);
    (*m_pspUiActionBar)->AddShowImageCode->setVisible(true);
    (*m_pspUiActionBar)->AddTextCode->setVisible(true);
    (*m_pspUiActionBar)->AddMetronomeCode->setVisible(true);
    (*m_pspUiActionBar)->AddNotificationCode->setVisible(true);
    (*m_pspUiActionBar)->AddTimerCode->setVisible(true);
    (*m_pspUiActionBar)->AddThreadCode->setVisible(true);
  }
  else
  {
    (*m_pspUiActionBar)->AddShowBackgroundCode->setVisible(false);
    (*m_pspUiActionBar)->AddShowIconCode->setVisible(false);
    (*m_pspUiActionBar)->AddShowImageCode->setVisible(false);
    (*m_pspUiActionBar)->AddTextCode->setVisible(false);
    (*m_pspUiActionBar)->AddMetronomeCode->setVisible(false);
    (*m_pspUiActionBar)->AddNotificationCode->setVisible(false);
    (*m_pspUiActionBar)->AddTimerCode->setVisible(false);
    (*m_pspUiActionBar)->AddThreadCode->setVisible(false);
  }

  m_spUi->pEditorStackedWidget->setCurrentIndex(PageIndexFromScriptType(sScriptType));
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::Update()
{
  m_spUi->pCodeEdit->update();
}

//----------------------------------------------------------------------------------------
//
QString CCodeDisplayWidget::GetCurrentText() const
{
  return m_spUi->pCodeEdit->toPlainText();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotExecutionError(QString sException, qint32 iLine, QString sStack)
{
  m_spUi->pCodeEdit->SlotExecutionError(sException, iLine, sStack);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotShowOverlay()
{
  if (!m_pInitialized || nullptr == m_pspUiActionBar) { return; }

  if (sender() == (*m_pspUiActionBar)->AddShowBackgroundCode)
  {
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spBackgroundSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddShowIconCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddShowImageCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddTextCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddMetronomeCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddNotificationCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddTimerCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
    m_spIconSnippetOverlay->Hide();
    m_spMetronomeSnippetOverlay->Hide();
    m_spNotificationSnippetOverlay->Hide();
    m_spResourceSnippetOverlay->Hide();
    m_spTextSnippetOverlay->Hide();
    m_spThreadSnippetOverlay->Hide();
    m_spTimerSnippetOverlay->Toggle();
  }
  else if (sender() == (*m_pspUiActionBar)->AddThreadCode)
  {
    m_spBackgroundSnippetOverlay->Hide();
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
  m_spUi->pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayWidget::SlotUndoForScriptContentAdded()
{
  if (!m_pInitialized) { return; }
  m_pUndoStack->push(new CCommandScriptContentChange(m_spUi->pCodeEdit->document()));
}
