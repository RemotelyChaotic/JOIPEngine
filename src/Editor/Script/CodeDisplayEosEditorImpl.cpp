#include "CodeDisplayEosEditorImpl.h"
#include "CommandUpdateEosCommand.h"
#include "EosScriptEditorView.h"
#include "EosScriptModel.h"
#include "EosScriptModelItem.h"
#include "EosScriptOverlayDelegate.h"
#include "EosCommandModels.h"
#include "ui_EditorActionBar.h"

#include "Editor/EditorModel.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"

// Testing
//#include <QtTest/QAbstractItemModelTester>

#include <QDebug>
#include <QHeaderView>
#include <QItemSelectionModel>

//----------------------------------------------------------------------------------------
//
CCodeDisplayEosEditorImpl::CCodeDisplayEosEditorImpl(QPointer<CEosScriptEditorView> pTarget) :
  QObject(nullptr),
  ICodeDisplayWidgetImpl(),
  m_spEosParser(std::make_unique<CJsonInstructionSetParser>()),
  m_pTarget(pTarget),
  m_pEditorDelegate(new CEosScriptOverlayDelegate(pTarget)),
  m_pProxy(new CEosSortFilterProxyModel(m_pTarget)),
  m_pJsonParserModel(new CEosScriptModel(m_pTarget))
{
  m_spEosParser->RegisterInstructionSetPath("Commands", "/");
  m_spEosParser->RegisterInstruction<CCommandEosAudioModel>(eos::c_sCommandAudioPlay);
  m_spEosParser->RegisterInstruction<CCommandEosChoiceModel>(eos::c_sCommandChoice);
  m_spEosParser->RegisterInstruction<CCommandEosDisableSceneModel>(eos::c_sCommandDisableScreen);
  m_spEosParser->RegisterInstruction<CCommandEosEnableSceneModel>(eos::c_sCommandEnableScreen);
  m_spEosParser->RegisterInstruction<CCommandEosEndModel>(eos::c_sCommandEnd);
  m_spEosParser->RegisterInstruction<CCommandEosEvalModel>(eos::c_sCommandEval);
  m_spEosParser->RegisterInstruction<CCommandEosGotoModel>(eos::c_sCommandGoto);
  m_spEosParser->RegisterInstruction<CCommandEosIfModel>(eos::c_sCommandIf);
  m_spEosParser->RegisterInstruction<CCommandEosImageModel>(eos::c_sCommandImage);
  m_spEosParser->RegisterInstruction<CCommandEosNoopModel>(eos::c_sCommandNoop);
  m_spEosParser->RegisterInstruction<CCommandEosNotificationCreateModel>(eos::c_sCommandNotificationCreate);
  m_spEosParser->RegisterInstruction<CCommandEosNotificationCloseModel>(eos::c_sCommandNotificationClose);
  m_spEosParser->RegisterInstruction<CCommandEosPromptModel>(eos::c_sCommandPrompt);
  m_spEosParser->RegisterInstruction<CCommandEosSayModel>(eos::c_sCommandSay);
  m_spEosParser->RegisterInstruction<CCommandEosTimerModel>(eos::c_sCommandTimer);

  m_pProxy->setSourceModel(m_pJsonParserModel);
  m_pTarget->setModel(m_pProxy);
  m_pTarget->header()->setSectionHidden(eos_item::c_iColumnType, true);
  m_pTarget->setHeaderHidden(true);
  m_pTarget->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_pTarget->setAlternatingRowColors(true);

  QItemSelectionModel* pSelectionModel = m_pTarget->selectionModel();
  connect(m_pTarget, &CEosScriptEditorView::SignalClickedOutside,
          this, &CCodeDisplayEosEditorImpl::SlotClickedOutsideView);
  connect(pSelectionModel, &QItemSelectionModel::selectionChanged,
          this, &CCodeDisplayEosEditorImpl::CurrentItemChanged);
  connect(m_pEditorDelegate, &CEosScriptOverlayDelegate::SignalCurrentItemChanged,
          this, &CCodeDisplayEosEditorImpl::CurrentItemDataChanged);
  connect(m_pEditorDelegate, &CEosScriptOverlayDelegate::SignalInvalidateItemChildren,
          this, &CCodeDisplayEosEditorImpl::CurrentItemInvalidated);
  connect(m_pJsonParserModel, &CEosScriptModel::dataChanged,
          this, &CCodeDisplayEosEditorImpl::SlotItemModelDataChanged);
}
CCodeDisplayEosEditorImpl::~CCodeDisplayEosEditorImpl()
{

}

void CCodeDisplayEosEditorImpl::Initialize(CEditorModel* pEditorModel)
{
  m_pEditorModel = pEditorModel;
  m_pJsonParserModel->SetUndoStack(pEditorModel->UndoStack());
  m_pEditorDelegate->Initialize(m_pEditorModel->ResourceTreeModel());
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::Clear()
{
  m_pJsonParserModel->SetRunner(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ExecutionError(QString, qint32, QString)
{
  // Nothing we can do yet
  //m_pCodeEdit->SlotExecutionError(sException, iLine, sStack);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::InsertGeneratedCode(const QString&)
{
  // Nothing to do, we handle buttons differently in this editor
  //m_pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ResetWidget()
{
  // Nothing to do, qt handles everything and we have no error messages
  //m_pJsonParserModel->SetRunner(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SetContent(const QString& sContent)
{
  // Debug
  //QFile file("in.json"); file.open(QIODevice::ReadWrite | QIODevice::Truncate);
  //file.write(sContent.toUtf8());

  auto spEosRunnerMain = m_spEosParser->ParseJson(sContent);
  if (nullptr == spEosRunnerMain)
  {
    qWarning() << "Could not correctly parse script.";
    return;
  }
  m_pJsonParserModel->SetRunner(spEosRunnerMain);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SetHighlightDefinition(const QString&)
{
  // No highlight definitions used
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::HideButtons(Ui::CEditorActionBar* pActionBar)
{
  if (nullptr != pActionBar)
  {
    disconnect(pActionBar->AddAutioCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddAudio);
    disconnect(pActionBar->AddChoiceCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddChoice);
    disconnect(pActionBar->AddDisableCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddDisable);
    disconnect(pActionBar->AddEnableCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEnable);
    disconnect(pActionBar->AddEndCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEnd);
    disconnect(pActionBar->AddEvalCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEval);
    disconnect(pActionBar->AddGotoCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddGoto);
    disconnect(pActionBar->AddIfCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddIf);
    disconnect(pActionBar->AddImageCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddImage);
    disconnect(pActionBar->AddNotificationCloseCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddNotificationRemove);
    disconnect(pActionBar->AddNotificationCreateCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddNotificationCreate);
    disconnect(pActionBar->AddPromptCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddPrompt);
    disconnect(pActionBar->AddSayCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddSay);
    disconnect(pActionBar->AddTimerCode2, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddTimer);
    disconnect(pActionBar->RemoveCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotRemoveInstruction);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ShowButtons(Ui::CEditorActionBar* pActionBar)
{
  if (nullptr != pActionBar)
  {
    pActionBar->pCodeEditorContainerStack->setCurrentWidget(pActionBar->pCodeEditorContainerPageEos);
    connect(pActionBar->AddAutioCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddAudio);
    connect(pActionBar->AddChoiceCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddChoice);
    connect(pActionBar->AddDisableCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddDisable);
    connect(pActionBar->AddEnableCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEnable);
    connect(pActionBar->AddEndCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEnd);
    connect(pActionBar->AddEvalCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddEval);
    connect(pActionBar->AddGotoCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddGoto);
    connect(pActionBar->AddIfCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddIf);
    connect(pActionBar->AddImageCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddImage);
    connect(pActionBar->AddNotificationCloseCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddNotificationRemove);
    connect(pActionBar->AddNotificationCreateCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddNotificationCreate);
    connect(pActionBar->AddPromptCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddPrompt);
    connect(pActionBar->AddSayCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddSay);
    connect(pActionBar->AddTimerCode2, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotAddTimer);
    connect(pActionBar->RemoveCode, &QPushButton::clicked, this, &CCodeDisplayEosEditorImpl::SlotRemoveInstruction);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::Update()
{
  m_pTarget->update();
}

//----------------------------------------------------------------------------------------
//
QString CCodeDisplayEosEditorImpl::GetCurrentText() const
{
  auto spRunner = m_pJsonParserModel->Runner();
  if (nullptr != spRunner)
  {
    return m_spEosParser->ToJson(spRunner);
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::CurrentItemChanged(const QItemSelection& selected,
                                                   const QItemSelection& deselected)
{
  Q_UNUSED(deselected)
  m_bEnableDelegateUpdate = false;
  if (selected.count() == 1 && selected.indexes()[0].isValid())
  {
    QModelIndex currentMapped = m_pProxy->mapToSource(selected.indexes()[0]);
    CEosScriptModelItem* pItem = m_pJsonParserModel->GetItem(currentMapped);
    if (currentMapped.isValid() && nullptr != pItem &&
        (pItem->Type()._to_integral() == EosScriptModelItem::eInstruction ||
         pItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild))
    {
      if (pItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
      {
        CEosScriptModelItem* pParent = pItem->Parent();
        SItemIndexPath path;
        m_pJsonParserModel->GetIndexPath(m_pJsonParserModel->parent(currentMapped), -1, &path);
        m_pEditorDelegate->Show(path,
              pParent->Data(eos_item::c_iColumnType, eos_item::c_iRoleEosItemType).toString(),
              *pParent->Arguments());
      }
      else
      {
        SItemIndexPath path;
        m_pJsonParserModel->GetIndexPath(currentMapped, -1, &path);
        m_pEditorDelegate->Show(path,
                                pItem->Data(eos_item::c_iColumnType, eos_item::c_iRoleEosItemType).toString(),
                                *pItem->Arguments());
      }
    }
    else
    {
      m_pEditorDelegate->Hide();
    }
  }
  else
  {
    m_pEditorDelegate->Hide();
  }
  m_bEnableDelegateUpdate = true;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::CurrentItemDataChanged()
{
  QModelIndex currentMapped = m_pJsonParserModel->GetIndex(m_pEditorDelegate->CurrentPath());
  if (currentMapped.isValid())
  {
    m_bEnableDelegateUpdate = false;
    CEosScriptModelItem* pItem = m_pJsonParserModel->GetItem(currentMapped);
    m_pEditorModel->UndoStack()->push(
          new CCommandUpdateEosCommand(m_pJsonParserModel, m_pTarget,
                                       m_pEditorDelegate->CurrentPath(),
                                       *pItem->Arguments(),
                                       m_pEditorDelegate->CurrentProperties(),
                                       false));
    m_bEnableDelegateUpdate = true;
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::CurrentItemInvalidated()
{
  QModelIndex currentMapped = m_pJsonParserModel->GetIndex(m_pEditorDelegate->CurrentPath());
  if (currentMapped.isValid())
  {
    m_bEnableDelegateUpdate = false;
    CEosScriptModelItem* pItem = m_pJsonParserModel->GetItem(currentMapped);
    m_pEditorModel->UndoStack()->push(
          new CCommandUpdateEosCommand(m_pJsonParserModel, m_pTarget,
                                       m_pEditorDelegate->CurrentPath(),
                                       *pItem->Arguments(),
                                       m_pEditorDelegate->CurrentProperties(),
                                       true));
    m_bEnableDelegateUpdate = true;
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SlotItemModelDataChanged(
    const QModelIndex& begin, const QModelIndex& end)
{
  // testing
  //QAbstractItemModelTester test(m_pJsonParserModel,
  //                              QAbstractItemModelTester::FailureReportingMode::Warning);

  if (m_pEditorDelegate->isVisible() && m_bEnableDelegateUpdate)
  {
    CEosScriptModelItem* pItem = m_pJsonParserModel->GetItem(begin);
    SItemIndexPath path;
    m_pJsonParserModel->GetIndexPath(begin, -1, &path);
    m_pEditorDelegate->Show(path,
                            pItem->Data(eos_item::c_iColumnType, eos_item::c_iRoleEosItemType).toString(),
                            *pItem->Arguments());
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SlotAddAudio()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandAudioPlay, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddChoice()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandChoice, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddDisable()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandDisableScreen, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddEnable()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandEnableScreen, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddEnd()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandEnd, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddEval()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandEval, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddGoto()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandGoto, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddIf()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandIf, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddImage()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandImage, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddNotificationRemove()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandNotificationClose, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddNotificationCreate()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandNotificationCreate, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddPrompt()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandPrompt, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddSay()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandSay, {});
  m_bEnableDelegateUpdate = true;
}
void CCodeDisplayEosEditorImpl::SlotAddTimer()
{
  m_bEnableDelegateUpdate = false;
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->InsertInstruction(currentMapped,
                                        eos::c_sCommandTimer, {});
  m_bEnableDelegateUpdate = true;
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SlotClickedOutsideView()
{
  if (!m_pEditorDelegate->IsForcedOpen())
  {
    m_pEditorDelegate->Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SlotRemoveInstruction()
{
  QModelIndex currentMapped = m_pProxy->mapToSource(m_pTarget->selectionModel()->currentIndex());
  m_pJsonParserModel->RemoveInstruction(currentMapped);
}
