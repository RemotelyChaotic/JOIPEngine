#include "EditorDialogueWidget.h"
#include "Application.h"

#include "Editor/DialogueEditor/CommandAddNewDialogueFile.h"
#include "Editor/DialogueEditor/CommandAddRemoveNode.h"
#include "Editor/DialogueEditor/CommandChangeParameters.h"
#include "Editor/DialogueEditor/DialogueEditorDelegate.h"
#include "Editor/DialogueEditor/DialogueEditorSortFilterProxyModel.h"
#include "Editor/DialogueEditor/DialogueEditorTreeItem.h"
#include "Editor/DialogueEditor/DialogueEditorTreeModel.h"
#include "Editor/DialogueEditor/DialoguePropertyEditor.h"
#include "Editor/DialogueEditor/DialogueTagsEditorOverlay.h"

#include "Editor/Resources/CommandAddResource.h"
#include "Editor/Resources/ResourceTreeItemModel.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"

#include "Systems/HelpFactory.h"
#include "Systems/Project.h"

#include "Widgets/HelpOverlay.h"
#include "Widgets/PositionalMenu.h"

#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <QUndoStack>
#include <QWidgetAction>
#include <QListWidget>
#include <QUuid>

namespace {
  const QString c_sDialogueTreeHelpId =  "Editor/Dialogue/DialogueTree";
}

DECLARE_EDITORWIDGET(CEditorDialogueWidget, EEditorWidget::eDialogueEditor)

//----------------------------------------------------------------------------------------
//
CEditorDialogueWidget::CEditorDialogueWidget(QWidget *parent) :
  CEditorWidgetBase(parent),
  m_spUi(std::make_unique<Ui::CEditorDialogueWidget>()),
  m_spPropertiesOverlay(std::make_unique<CDialoguePropertyEditor>(this)),
  m_spTagOverlay(std::make_unique<CDialogueTagsEditorOverlay>(this)),
  m_spCurrentProject(nullptr),
  m_pProxy(new CDialogueEditorSortFilterProxyModel(this))
{
  m_spUi->setupUi(this);
  m_spUi->pTreeView->setItemDelegate(new CDialogueEditorDelegate(m_spUi->pTreeView));

  m_pCopyAction = new QAction("Copy", m_spUi->pTreeView);
  m_pCopyAction->setShortcut(QKeySequence(QKeySequence::StandardKey::Copy));
  m_pCopyAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
  m_spUi->pTreeView->addAction(m_pCopyAction);
  connect(m_pCopyAction, &QAction::triggered, this, &CEditorDialogueWidget::SlotCopy);

  m_pPasteAction = new QAction("Paste", m_spUi->pTreeView);
  m_pPasteAction->setShortcut(QKeySequence(QKeySequence::StandardKey::Paste));
  m_pPasteAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
  m_spUi->pTreeView->addAction(m_pPasteAction);
  connect(m_pPasteAction, &QAction::triggered, this, &CEditorDialogueWidget::SlotPaste);

  connect(m_spPropertiesOverlay.get(), &CDialoguePropertyEditor::SignalDialogueChanged,
          this, &CEditorDialogueWidget::SlotDialogueChanged);

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          this, &CEditorDialogueWidget::SlotFilterChanged);
}

CEditorDialogueWidget::~CEditorDialogueWidget()
{
  if (nullptr != m_spPropertiesOverlay) m_spPropertiesOverlay.reset();
  if (nullptr != m_spTagOverlay) m_spTagOverlay.reset();

  disconnect(m_spUi->pTreeView->model(), &QAbstractItemModel::modelReset,
             this, &CEditorDialogueWidget::SlotExpandAllNodes);

  dynamic_cast<CDialogueEditorSortFilterProxyModel*>(m_spUi->pTreeView->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::Initialize()
{
  m_bInitialized = false;

  m_spUi->pTreeView->installEventFilter(this);
  m_spUi->pTreeView->viewport()->installEventFilter(this);

  m_pProxy->sort(dialogue_item::c_iColumnId, Qt::AscendingOrder);
  m_pProxy->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_pProxy->setSourceModel(DialogueModel());

  connect(DialogueModel(), &CDialogueEditorTreeModel::SignalProjectEdited,
          this, &CEditorDialogueWidget::SignalProjectEdited);

  auto pDelegate = dynamic_cast<CDialogueEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetUndoStack(UndoStack());
  m_spUi->pTreeView->setModel(m_pProxy);

  QHeaderView* pHeader = m_spUi->pTreeView->header();
  pHeader->setSectionResizeMode(dialogue_item::c_iColumnId, QHeaderView::Interactive);
  pHeader->setSectionResizeMode(dialogue_item::c_iColumnString, QHeaderView::Stretch);
  pHeader->setSectionResizeMode(dialogue_item::c_iColumnWaitMS, QHeaderView::Interactive);
  pHeader->setSectionResizeMode(dialogue_item::c_iColumnSkippable, QHeaderView::Fixed);
  pHeader->setSectionResizeMode(dialogue_item::c_iColumnMedia, QHeaderView::Interactive);
  pHeader->setStretchLastSection(false);
  pHeader->resizeSection(dialogue_item::c_iColumnId, 150);
  pHeader->resizeSection(dialogue_item::c_iColumnWaitMS, 50);
  pHeader->resizeSection(dialogue_item::c_iColumnSkippable, 50);
  pHeader->resizeSection(dialogue_item::c_iColumnMedia, 150);

  connect(m_spUi->pTreeView->model(), &QAbstractItemModel::modelReset,
          this, &CEditorDialogueWidget::SlotExpandAllNodes);

  setAcceptDrops(true);

  m_spUi->pFilter->SetFilterUndo(true);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTreeView->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDialogueTreeHelpId);
    wpHelpFactory->RegisterHelp(c_sDialogueTreeHelpId, ":/resources/help/editor/dialog/dialogtree_button_help.html");
  }

  m_spPropertiesOverlay->Initialize(ResourceTreeModel());

  m_spTagOverlay->SetUndoStack(UndoStack());
  m_spTagOverlay->SetModel(DialogueModel());

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::LoadProject(tspProject spCurrentProject)
{
  m_spCurrentProject = spCurrentProject;

  bool bReadOnly = EditorModel()->IsReadOnly();

  auto pDelegate = dynamic_cast<CDialogueEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetCurrentProject(m_spCurrentProject);
  pDelegate->SetReadOnly(bReadOnly);

  m_spPropertiesOverlay->LoadProject(spCurrentProject);
  m_spTagOverlay->SetProject(spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::UnloadProject()
{
  auto pDelegate = dynamic_cast<CDialogueEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetCurrentProject(nullptr);

  m_spPropertiesOverlay->UnloadProject();
  m_spPropertiesOverlay->Hide();

  m_spTagOverlay->SetProject(nullptr);

  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SaveProject()
{
  auto spRoot = DialogueModel()->Root();
  if (nullptr != spRoot)
  {
    dialogue_tree::SaveDialogues(spRoot, m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->AddDialogue, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueClicked);
    disconnect(ActionBar()->m_spUi->AddDialogueFrament, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueFragmentClicked);
    disconnect(ActionBar()->m_spUi->AddDialogueCategoryButton, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueCategoryClicked);
    disconnect(ActionBar()->m_spUi->RemoveDialogue, &QPushButton::clicked,
               this, &CEditorDialogueWidget::SlotRemoveDialogueClicked);
    disconnect(ActionBar()->m_spUi->EditDialogueContent, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotEditDialogueClicked);
    disconnect(ActionBar()->m_spUi->EditDialogueTags, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotEditDialogueTagsClicked);

    ActionBar()->m_spUi->AddDialogue->setEnabled(true);
    ActionBar()->m_spUi->RemoveDialogue->setEnabled(true);
    ActionBar()->m_spUi->AddDialogueCategoryButton->setEnabled(true);
    ActionBar()->m_spUi->EditDialogueContent->setEnabled(true);
    ActionBar()->m_spUi->EditDialogueTags->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowDialogueActionBar();
    connect(ActionBar()->m_spUi->AddDialogue, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueClicked);
    connect(ActionBar()->m_spUi->AddDialogueFrament, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueFragmentClicked);
    connect(ActionBar()->m_spUi->AddDialogueCategoryButton, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotAddDialogueCategoryClicked);
    connect(ActionBar()->m_spUi->RemoveDialogue, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotRemoveDialogueClicked);
    connect(ActionBar()->m_spUi->EditDialogueContent, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotEditDialogueClicked);
    connect(ActionBar()->m_spUi->EditDialogueTags, &QPushButton::clicked,
            this, &CEditorDialogueWidget::SlotEditDialogueTagsClicked);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddDialogue->setEnabled(false);
      ActionBar()->m_spUi->RemoveDialogue->setEnabled(false);
      ActionBar()->m_spUi->AddDialogueCategoryButton->setEnabled(false);
      ActionBar()->m_spUi->EditDialogueContent->setEnabled(false);
      ActionBar()->m_spUi->EditDialogueTags->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CEditorDialogueWidget::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr == pObj || nullptr == pEvt) { return QWidget::eventFilter(pObj, pEvt); }
  if (m_spUi->pTreeView == pObj ||
      m_spUi->pTreeView->viewport() == pObj)
  {
    if (QEvent::ContextMenu == pEvt->type())
    {
      QContextMenuEvent* pContextEvt = static_cast<QContextMenuEvent*>(pEvt);
      CDialogueEditorTreeModel* pModel = nullptr;
      QModelIndex index;
      if (m_spUi->pTreeView == pObj)
      {
        index = m_spUi->pTreeView->indexAt(
            m_spUi->pTreeView->viewport()->mapFromGlobal(pContextEvt->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pTreeView->model()))
        {
          pModel = dynamic_cast<CDialogueEditorTreeModel*>(pProxy->sourceModel());
          index = pProxy->mapToSource(index);
        }
      }
      ShowContextMenu(pModel, index, pContextEvt->globalPos());
    }
  }
  return QWidget::eventFilter(pObj, pEvt);
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
  if (EditorModel()->IsReadOnly()) { return; }

  const QMimeData* pMimeData = pEvent->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (pMimeData->hasUrls())
  {
    pEvent->acceptProposedAction();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::dropEvent(QDropEvent* pEvent)
{
  if (EditorModel()->IsReadOnly()) { return; }

  const QMimeData* pMimeData = pEvent->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (pMimeData->hasUrls())
  {
    QStringList vsFileNames;
    QList<QUrl> vUrlList = pMimeData->urls();
    std::map<QUrl, QByteArray> vsFiles;
    for (const QUrl& sPath : qAsConst(vUrlList))
    {
      // Try to read first to see if there are errros.
      bool bOk = true;
      if (nullptr != dialogue_tree::LoadDialoguesFromSource({sPath}, m_spCurrentProject, &bOk) && bOk)
      {
        vsFiles.insert({sPath, QByteArray()});
      }
    }

    if (!vsFiles.empty())
    {
      UndoStack()->push(new CCommandAddResource(m_spCurrentProject, this, vsFiles));
      emit SignalProjectEdited();
    }
  }
}

//----------------------------------------------------------------------------------------
//
namespace
{
  QString GetResourceSource(CEditorDialogueWidget* pThis,
                            QPointer<CEditorActionBar> pActionBar,
                            QPointer<CEditorModel> pEditorModel,
                            QPointer<QUndoStack> pUndo,
                            const tspProject& spCurrentProject)
  {
    constexpr qint32 c_iAllwaysShowRole = Qt::UserRole;

    QString sSourceResource;

    CPositionalMenu modelMenu(
        pActionBar->ActionBarPosition() == CEditorActionBar::eRight ?
            EMenuPopupPosition::eLeft : EMenuPopupPosition::eLeft);

    //Add filterbox to the context menu
    auto *txtBox = new CSearchWidget(&modelMenu);

    auto* txtBoxAction = new QWidgetAction(&modelMenu);
    txtBoxAction->setDefaultWidget(txtBox);

    modelMenu.addAction(txtBoxAction);

    //Add result treeview to the context menu
    auto* pListView = new QListWidget(&modelMenu);

    auto* pViewAction = new QWidgetAction(&modelMenu);
    pViewAction->setDefaultWidget(pListView);

    modelMenu.addAction(pViewAction);

    QMap<QString, QListWidgetItem*> topLevelItems;
    {
      QReadLocker locker(&spCurrentProject->m_rwLock);
      for (const auto& [sName, spResource] : spCurrentProject->m_spResourcesMap)
      {
        QReadLocker l(&spResource->m_rwLock);
        if (EResourceType::eDatabase != spResource->m_type._to_integral()) { continue; }
        if (QFileInfo(PhysicalResourcePath(spResource)).suffix() != joip_resource::c_sDialogueFileType)
        { continue; }

        auto item = new QListWidgetItem(pListView);
        item->setText(sName);
        item->setData(c_iAllwaysShowRole, false);
        topLevelItems.insert(sName, item);
      }
    }

    const QString sNewResource = "<New Resource>";
    auto item = new QListWidgetItem(pListView);
    item->setText(sNewResource);
    item->setData(c_iAllwaysShowRole, true);
    topLevelItems.insert(sNewResource, item);

    QObject::connect(pListView, &QListWidget::itemClicked, pListView,
            [&](QListWidgetItem *item)
            {
              if (!item->data(c_iAllwaysShowRole).toBool())
              {
                sSourceResource = item->text();
              }
              else
              {
                CCommandAddNewDialogueFile* pCmd =
                    new CCommandAddNewDialogueFile(spCurrentProject, pEditorModel, pThis);
                pUndo->push(pCmd);
                sSourceResource = pCmd->AddedResource();
              }
              modelMenu.close();
            });

    //Setup filtering
    QObject::connect(txtBox, &CSearchWidget::SignalFilterChanged, txtBox, [&](const QString &text)
            {
              for (auto& topLvlItem : topLevelItems)
              {
                bool bAlwaysShow = topLvlItem->data(c_iAllwaysShowRole).toBool();
                auto modelName = topLvlItem->text();
                const bool match = (modelName.contains(text, Qt::CaseInsensitive));
                topLvlItem->setHidden(!bAlwaysShow & !match);
              }
            });

    // make sure the text box gets focus so the user doesn't have to click on it
    txtBox->setFocus();

    QPoint p =
        pActionBar->m_spUi->AddDialogue->parentWidget()->mapToGlobal(
            pActionBar->m_spUi->AddDialogue->pos());
    modelMenu.exec(p);

    return sSourceResource;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotAddDialogueClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (nullptr == ActionBar() || nullptr == EditorModel()) { return; }

  QPointer<CEditorDialogueWidget> pThis(this);

  qint32 iInsertPos = -1;
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  qint32 iTypeOfSelectedNode =
    DialogueModel()->data(idxSelected, CDialogueEditorTreeModel::eTypeRole).toInt();
  QString sSourceResource;

  switch(iTypeOfSelectedNode)
  {
    case EDialogueTreeNodeType::eDialogueFragment:
      // not allowed
      return;
    case EDialogueTreeNodeType::eRoot: [[fallthrough]];
    case EDialogueTreeNodeType::eCategory:
    {
      iInsertPos = DialogueModel()->rowCount(idxSelected);
    } break;
    case EDialogueTreeNodeType::eDialogue:
    {
      iInsertPos = idxSelected.row()+1;
      vsPath.erase(vsPath.begin()+vsPath.size()-1);
    } break;
  }

  sSourceResource = GetResourceSource(this, ActionBar(), EditorModel(), UndoStack(),
                                      m_spCurrentProject);
  if (sSourceResource.isEmpty()) { return; }

  if (nullptr == pThis) { return; }

  if (nullptr != UndoStack())
  {
    std::shared_ptr<CDialogueNodeDialogue> spNewNode = std::make_shared<CDialogueNodeDialogue>();
    spNewNode->m_sName = QUuid::createUuid().toString();
    spNewNode->m_sFileId = sSourceResource;
    spNewNode->m_bHasCondition = false;

    std::shared_ptr<CDialogueData> spNewNodeData = std::make_shared<CDialogueData>();
    spNewNodeData->m_wpParent = spNewNode;
    spNewNodeData->m_sName = QUuid::createUuid().toString();
    spNewNodeData->m_sFileId = sSourceResource;

    spNewNode->m_vspChildren.push_back(spNewNodeData);
    vsPath << spNewNode->m_sName;

    UndoStack()->push(new CCommandAddDialogueNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotAddDialogueFragmentClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (nullptr == ActionBar() || nullptr == EditorModel()) { return; }

  QPointer<CEditorDialogueWidget> pThis(this);

  qint32 iInsertPos = -1;
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  qint32 iTypeOfSelectedNode =
      DialogueModel()->data(idxSelected, CDialogueEditorTreeModel::eTypeRole).toInt();
  QString sSourceResource;
  bool bNewNode = false;

  switch(iTypeOfSelectedNode)
  {
    case EDialogueTreeNodeType::eRoot: [[fallthrough]];
    case EDialogueTreeNodeType::eCategory:
    {
      bNewNode = true;
      iInsertPos = DialogueModel()->rowCount(idxSelected);
    } break;
    case EDialogueTreeNodeType::eDialogueFragment:
      bNewNode = false;
      iInsertPos = idxSelected.row()+1;
      vsPath.erase(vsPath.begin()+vsPath.size()-1);
      sSourceResource =
          DialogueModel()->data(idxSelected.parent(), CDialogueEditorTreeModel::eResourceRole).toString();
      break;
    case EDialogueTreeNodeType::eDialogue:
    {
      if (DialogueModel()->HasCondition(idxSelected))
      {
        bNewNode = false;
        iInsertPos = DialogueModel()->rowCount(idxSelected);
        sSourceResource =
            DialogueModel()->data(idxSelected, CDialogueEditorTreeModel::eResourceRole).toString();
      }
      else
      {
        bNewNode = true;
        iInsertPos = idxSelected.row()+1;
        vsPath.erase(vsPath.begin()+vsPath.size()-1);
      }
    } break;
  }

  if (bNewNode)
  {
    sSourceResource = GetResourceSource(this, ActionBar(), EditorModel(), UndoStack(),
                                        m_spCurrentProject);
    if (sSourceResource.isEmpty()) { return; }
  }

  if (nullptr == pThis) { return; }

  if (nullptr != UndoStack())
  {
    std::shared_ptr<CDialogueNode> spNewNode;
    if (bNewNode)
    {
      std::shared_ptr<CDialogueNodeDialogue> spNewNodeI = std::make_shared<CDialogueNodeDialogue>();
      spNewNodeI->m_sName = QUuid::createUuid().toString();
      spNewNodeI->m_sFileId = sSourceResource;
      spNewNodeI->m_bHasCondition = true;

      std::shared_ptr<CDialogueData> spNewNodeData = std::make_shared<CDialogueData>();
      spNewNodeData->m_wpParent = spNewNodeI;
      spNewNodeData->m_sName = QUuid::createUuid().toString();
      spNewNodeData->m_sFileId = sSourceResource;

      spNewNodeI->m_vspChildren.push_back(spNewNodeData);
      vsPath << spNewNodeI->m_sName;

      spNewNode = spNewNodeI;
    }
    else
    {
      std::shared_ptr<CDialogueData> spNewNodeData = std::make_shared<CDialogueData>();
      spNewNodeData->m_sName = QUuid::createUuid().toString();
      spNewNodeData->m_sFileId = sSourceResource;
      spNewNode = spNewNodeData;
    }

    UndoStack()->push(new CCommandAddDialogueNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotAddDialogueCategoryClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  qint32 iInsertPos = DialogueModel()->rowCount(idxSelected);

  qint32 iTypeOfSelectedNode =
      DialogueModel()->data(idxSelected, CDialogueEditorTreeModel::eTypeRole).toInt();

  if (nullptr != UndoStack() && EDialogueTreeNodeType::eDialogueFragment != iTypeOfSelectedNode)
  {
    switch (iTypeOfSelectedNode)
    {
      case EDialogueTreeNodeType::eDialogue:
      {
        vsPath.erase(vsPath.begin() + vsPath.size()-1);
        iInsertPos += idxSelected.row()+1;
      } break;
      default: break;
    }

    std::shared_ptr<CDialogueNodeCategory> spNewNode = std::make_shared<CDialogueNodeCategory>();
    spNewNode->m_sName = "New Category";
    vsPath << spNewNode->m_sName;

    UndoStack()->push(new CCommandAddDialogueNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotRemoveDialogueClicked()
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandRemoveDialogueNode(m_spCurrentProject, vsPath,
                                                   DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotEditDialogueClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  std::shared_ptr<CDialogueNode> spNode = DialogueModel()->Node(idxSelected);
  std::shared_ptr<CDialogueNodeDialogue> spDNode =
      std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);

  if (EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral() ||
      (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral() && !spDNode->m_bHasCondition))
  {
    m_spPropertiesOverlay->SetNode(vsPath, spNode);
    m_spPropertiesOverlay->Toggle();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotDialogueChanged(QStringList vsPath, const std::shared_ptr<CDialogueNode>& spNode)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandChangeDialogueParameters(m_spCurrentProject, vsPath, spNode,
                                                   DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotEditDialogueTagsClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  std::shared_ptr<CDialogueNode> spNode = DialogueModel()->Node(idxSelected);
  std::shared_ptr<CDialogueNodeDialogue> spDNode =
      std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);

  if (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral() && nullptr != spDNode)
  {
    m_spTagOverlay->SetPath(vsPath);
    m_spTagOverlay->Toggle();
  }
}

//----------------------------------------------------------------------------------------
//
namespace
{
  void IterateTreeItems(QAbstractItemView* pView, const QModelIndex& index,
                        const std::function<void(const QModelIndex&)>& fnToCall)
  {
    if (index.isValid())
    {
      fnToCall(index);
    }

    if (!pView->model()->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren))
    {
      return;
    }
    qint32 iRows = pView->model()->rowCount(index);
    for (qint32 i = 0; i < iRows; ++i)
    {
      IterateTreeItems(pView, pView->model()->index(i, 0, index), fnToCall);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotExpandAllNodes()
{
  IterateTreeItems(m_spUi->pTreeView, QModelIndex(), [this](const QModelIndex& idx) {
    m_spUi->pTreeView->expand(idx);
  });
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotFilterChanged(const QString& sText)
{
  WIDGET_INITIALIZED_GUARD

  QPointer<CDialogueEditorSortFilterProxyModel> pProxyModel =
      dynamic_cast<CDialogueEditorSortFilterProxyModel*>(m_spUi->pTreeView->model());

  if (nullptr != pProxyModel)
  {
    if (sText.simplified().isEmpty())
    {
      pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
    }
    else
    {
      pProxyModel->setFilterRegExp(sText);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotCopy()
{
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  std::shared_ptr<CDialogueNode> spNode = DialogueModel()->Node(idxSelected);

  QClipboard* pClipboard = QGuiApplication::clipboard();
  pClipboard->setText(QString::fromUtf8(dialogue_tree::SerializeNode(spNode)));
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::SlotPaste()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QClipboard* pClipboard = QGuiApplication::clipboard();
  QString sCopied = pClipboard->text();

  // Try to read first to see if there are errros.
  std::shared_ptr<CDialogueNode> spNode =
      dialogue_tree::DeserializeNode(sCopied.toUtf8(), m_spCurrentProject);
  if (nullptr == spNode)
  {
    return;
  }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogueModel()->Path(idxSelected);
  qint32 iInsertPos = DialogueModel()->rowCount(idxSelected);
  qint32 iTypeOfSelectedNode =
      DialogueModel()->data(idxSelected, CDialogueEditorTreeModel::eTypeRole).toInt();

  switch(iTypeOfSelectedNode)
  {
    case EDialogueTreeNodeType::eRoot: [[fallthrough]];
    case EDialogueTreeNodeType::eCategory:
    {
      switch(spNode->m_type)
      {
        case EDialogueTreeNodeType::eRoot: return;
        case EDialogueTreeNodeType::eCategory:
        {
          iInsertPos = DialogueModel()->rowCount(idxSelected);
        } break;
        case EDialogueTreeNodeType::eDialogue:
        {
          iInsertPos = DialogueModel()->rowCount(idxSelected);
        } break;
        case EDialogueTreeNodeType::eDialogueFragment: return;
      }
    } break;
    case EDialogueTreeNodeType::eDialogue:
    {
      switch(spNode->m_type)
      {
        case EDialogueTreeNodeType::eRoot: return;
        case EDialogueTreeNodeType::eCategory:
        {
          iInsertPos = idxSelected.row()+1;
          vsPath.erase(vsPath.begin()+vsPath.size()-1);
        } break;
        case EDialogueTreeNodeType::eDialogue:
        {
          iInsertPos = idxSelected.row()+1;
          vsPath.erase(vsPath.begin()+vsPath.size()-1);
        } break;
        case EDialogueTreeNodeType::eDialogueFragment:
        {
          if (DialogueModel()->HasCondition(idxSelected))
          {
            iInsertPos = DialogueModel()->rowCount(idxSelected);
          }
          else
          {
            return;
          }
        } break;
      }
    } break;
    case EDialogueTreeNodeType::eDialogueFragment:
    {
      switch(spNode->m_type)
      {
        case EDialogueTreeNodeType::eRoot: [[fallthrough]];
        case EDialogueTreeNodeType::eCategory: [[fallthrough]];
        case EDialogueTreeNodeType::eDialogue: return;
        case EDialogueTreeNodeType::eDialogueFragment:
        {
          iInsertPos = idxSelected.row()+1;
          vsPath.erase(vsPath.begin()+vsPath.size()-1);
        } break;
      }
    } break;
  }

  if (nullptr != UndoStack())
  {
    vsPath << spNode->m_sName;
    UndoStack()->push(new CCommandAddDialogueNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNode, DialogueModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogueWidget::ShowContextMenu(CDialogueEditorTreeModel* pModel, const QModelIndex& idx,
                                          const QPoint& globalPos)
{
  if (nullptr != pModel && pModel == dynamic_cast<const CDialogueEditorTreeModel*>(idx.model()))
  {
    auto spNode = pModel->Node(idx);

    QMenu menu(this);

    menu.addAction(m_pCopyAction);
    menu.addAction(m_pPasteAction);

    menu.addSeparator();

    QAction* pAction = new QAction("Copy Name", &menu);
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      pClipboard->setText(spNode->m_sName);
    });
    menu.addAction(pAction);

    pAction = new QAction("Copy Text", &menu);
    pAction->setEnabled(EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral() ||
                        EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral());
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      auto spNodeDat = std::dynamic_pointer_cast<CDialogueData>(spNode);
      QStringList vsStr;
      if (nullptr == spNodeDat && spNode->m_vspChildren.size() > 0)
      {
        for (const auto& spChild : spNode->m_vspChildren)
        {
          auto spChildCasted = std::dynamic_pointer_cast<CDialogueData>(spChild);
          vsStr << spChildCasted->m_sString;
        }
      }
      if (!vsStr.isEmpty())
      {
        pClipboard->setText(vsStr.join("\n"));
      }
    });
    menu.addAction(pAction);

    pAction = new QAction("Copy Condition", &menu);
    pAction->setEnabled(EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral());
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      auto spNodeDat = std::dynamic_pointer_cast<CDialogueData>(spNode);
      QStringList vsStr;
      if (nullptr == spNodeDat && spNode->m_vspChildren.size() > 0)
      {
        for (const auto& spChild : spNode->m_vspChildren)
        {
          auto spChildCasted = std::dynamic_pointer_cast<CDialogueData>(spChild);
          vsStr << spChildCasted->m_sString;
        }
      }
      if (!vsStr.isEmpty())
      {
        pClipboard->setText(vsStr.join("\n"));
      }
    });
    menu.addAction(pAction);

    pAction = new QAction("Copy Source", &menu);
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      pClipboard->setText(spNode->m_sFileId);
    });
    menu.addAction(pAction);

    QMenu* pSubMenu = new QMenu("Copy Tag", &menu);
    {
      auto spNodeDat = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
      if (nullptr != spNodeDat)
      {
        for (const auto& spTag : spNodeDat->m_tags)
        {
          pAction = new QAction(spTag.first, pSubMenu);
          connect(pAction, &QAction::triggered, pModel, [sTag = spTag.first]() {
            QClipboard* pClipboard = QGuiApplication::clipboard();
            pClipboard->setText(sTag);
          });
          pSubMenu->addAction(pAction);
        }
      }
    }
    menu.addMenu(pSubMenu);

    menu.exec(globalPos);
  }
}
