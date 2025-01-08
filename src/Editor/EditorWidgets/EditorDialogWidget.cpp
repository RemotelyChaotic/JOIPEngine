#include "EditorDialogWidget.h"
#include "Application.h"

#include "Editor/DialogEditor/CommandAddNewDialogFile.h"
#include "Editor/DialogEditor/CommandAddRemoveNode.h"
#include "Editor/DialogEditor/CommandChangeParameters.h"
#include "Editor/DialogEditor/DialogEditorDelegate.h"
#include "Editor/DialogEditor/DialogEditorSortFilterProxyModel.h"
#include "Editor/DialogEditor/DialogEditorTreeItem.h"
#include "Editor/DialogEditor/DialogEditorTreeModel.h"
#include "Editor/DialogEditor/DialogPropertyEditor.h"
#include "Editor/DialogEditor/DialogTagsEditorOverlay.h"

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
  const QString c_sDialogTreeHelpId =  "Editor/Dialog/DialogTree";
}

DECLARE_EDITORWIDGET(CEditorDialogWidget, EEditorWidget::eDialogEditor)

//----------------------------------------------------------------------------------------
//
CEditorDialogWidget::CEditorDialogWidget(QWidget *parent) :
  CEditorWidgetBase(parent),
  m_spUi(std::make_unique<Ui::CEditorDialogWidget>()),
  m_spPropertiesOverlay(std::make_unique<CDialogPropertyEditor>(this)),
  m_spTagOverlay(std::make_unique<CDialogTagsEditorOverlay>(this)),
  m_spCurrentProject(nullptr),
  m_pProxy(new CDialogEditorSortFilterProxyModel(this))
{
  m_spUi->setupUi(this);
  m_spUi->pTreeView->setItemDelegate(new CDialogEditorDelegate(m_spUi->pTreeView));

  m_pCopyAction = new QAction("Copy", m_spUi->pTreeView);
  m_pCopyAction->setShortcut(QKeySequence(QKeySequence::StandardKey::Copy));
  m_pCopyAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
  m_spUi->pTreeView->addAction(m_pCopyAction);
  connect(m_pCopyAction, &QAction::triggered, this, &CEditorDialogWidget::SlotCopy);

  m_pPasteAction = new QAction("Paste", m_spUi->pTreeView);
  m_pPasteAction->setShortcut(QKeySequence(QKeySequence::StandardKey::Paste));
  m_pPasteAction->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
  m_spUi->pTreeView->addAction(m_pPasteAction);
  connect(m_pPasteAction, &QAction::triggered, this, &CEditorDialogWidget::SlotPaste);

  connect(m_spPropertiesOverlay.get(), &CDialogPropertyEditor::SignalDialogChanged,
          this, &CEditorDialogWidget::SlotDialogChanged);

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          this, &CEditorDialogWidget::SlotFilterChanged);
}

CEditorDialogWidget::~CEditorDialogWidget()
{
  if (nullptr != m_spPropertiesOverlay) m_spPropertiesOverlay.reset();
  if (nullptr != m_spTagOverlay) m_spTagOverlay.reset();

  disconnect(m_spUi->pTreeView->model(), &QAbstractItemModel::modelReset,
             this, &CEditorDialogWidget::SlotExpandAllNodes);

  dynamic_cast<CDialogEditorSortFilterProxyModel*>(m_spUi->pTreeView->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::Initialize()
{
  m_bInitialized = false;

  m_spUi->pTreeView->installEventFilter(this);
  m_spUi->pTreeView->viewport()->installEventFilter(this);

  m_pProxy->sort(dialog_item::c_iColumnId, Qt::AscendingOrder);
  m_pProxy->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_pProxy->setSourceModel(DialogModel());

  connect(DialogModel(), &CDialogEditorTreeModel::SignalProjectEdited,
          this, &CEditorDialogWidget::SignalProjectEdited);

  auto pDelegate = dynamic_cast<CDialogEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetUndoStack(UndoStack());
  m_spUi->pTreeView->setModel(m_pProxy);

  QHeaderView* pHeader = m_spUi->pTreeView->header();
  pHeader->setSectionResizeMode(dialog_item::c_iColumnId, QHeaderView::Interactive);
  pHeader->setSectionResizeMode(dialog_item::c_iColumnString, QHeaderView::Stretch);
  pHeader->setSectionResizeMode(dialog_item::c_iColumnWaitMS, QHeaderView::Interactive);
  pHeader->setSectionResizeMode(dialog_item::c_iColumnSkippable, QHeaderView::Fixed);
  pHeader->setSectionResizeMode(dialog_item::c_iColumnMedia, QHeaderView::Interactive);
  pHeader->setStretchLastSection(false);
  pHeader->resizeSection(dialog_item::c_iColumnId, 150);
  pHeader->resizeSection(dialog_item::c_iColumnWaitMS, 50);
  pHeader->resizeSection(dialog_item::c_iColumnSkippable, 50);
  pHeader->resizeSection(dialog_item::c_iColumnMedia, 150);

  connect(m_spUi->pTreeView->model(), &QAbstractItemModel::modelReset,
          this, &CEditorDialogWidget::SlotExpandAllNodes);

  setAcceptDrops(true);

  m_spUi->pFilter->SetFilterUndo(true);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTreeView->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDialogTreeHelpId);
    wpHelpFactory->RegisterHelp(c_sDialogTreeHelpId, ":/resources/help/editor/dialog/dialogtree_button_help.html");
  }

  m_spPropertiesOverlay->Initialize(ResourceTreeModel());

  m_spTagOverlay->SetUndoStack(UndoStack());
  m_spTagOverlay->SetModel(DialogModel());

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::LoadProject(tspProject spCurrentProject)
{
  m_spCurrentProject = spCurrentProject;

  auto pDelegate = dynamic_cast<CDialogEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetCurrentProject(m_spCurrentProject);

  m_spPropertiesOverlay->LoadProject(spCurrentProject);
  m_spTagOverlay->SetProject(spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::UnloadProject()
{
  auto pDelegate = dynamic_cast<CDialogEditorDelegate*>(m_spUi->pTreeView->itemDelegate());
  pDelegate->SetCurrentProject(nullptr);

  m_spPropertiesOverlay->UnloadProject();
  m_spPropertiesOverlay->Hide();

  m_spTagOverlay->SetProject(nullptr);

  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SaveProject()
{
  auto spRoot = DialogModel()->Root();
  if (nullptr != spRoot)
  {
    dialog_tree::SaveDialogs(spRoot, m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->AddDialog, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogClicked);
    disconnect(ActionBar()->m_spUi->AddDialogFrament, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogFragmentClicked);
    disconnect(ActionBar()->m_spUi->AddDialogCategoryButton, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogcategoryClicked);
    disconnect(ActionBar()->m_spUi->RemoveDialog, &QPushButton::clicked,
               this, &CEditorDialogWidget::SlotRemoveDialogClicked);
    disconnect(ActionBar()->m_spUi->EditDialogContent, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotEditDialogClicked);
    disconnect(ActionBar()->m_spUi->EditDialogTags, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotEditDialogTagsClicked);

    ActionBar()->m_spUi->AddDialog->setEnabled(true);
    ActionBar()->m_spUi->RemoveDialog->setEnabled(true);
    ActionBar()->m_spUi->AddDialogCategoryButton->setEnabled(true);
    ActionBar()->m_spUi->EditDialogContent->setEnabled(true);
    ActionBar()->m_spUi->EditDialogTags->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowDialogActionBar();
    connect(ActionBar()->m_spUi->AddDialog, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogClicked);
    connect(ActionBar()->m_spUi->AddDialogFrament, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogFragmentClicked);
    connect(ActionBar()->m_spUi->AddDialogCategoryButton, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotAddDialogcategoryClicked);
    connect(ActionBar()->m_spUi->RemoveDialog, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotRemoveDialogClicked);
    connect(ActionBar()->m_spUi->EditDialogContent, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotEditDialogClicked);
    connect(ActionBar()->m_spUi->EditDialogTags, &QPushButton::clicked,
            this, &CEditorDialogWidget::SlotEditDialogTagsClicked);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddDialog->setEnabled(false);
      ActionBar()->m_spUi->RemoveDialog->setEnabled(false);
      ActionBar()->m_spUi->AddDialogCategoryButton->setEnabled(false);
      ActionBar()->m_spUi->EditDialogContent->setEnabled(false);
      ActionBar()->m_spUi->EditDialogTags->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CEditorDialogWidget::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr == pObj || nullptr == pEvt) { return QWidget::eventFilter(pObj, pEvt); }
  if (m_spUi->pTreeView == pObj ||
      m_spUi->pTreeView->viewport() == pObj)
  {
    if (QEvent::ContextMenu == pEvt->type())
    {
      QContextMenuEvent* pContextEvt = static_cast<QContextMenuEvent*>(pEvt);
      CDialogEditorTreeModel* pModel = nullptr;
      QModelIndex index;
      if (m_spUi->pTreeView == pObj)
      {
        index = m_spUi->pTreeView->indexAt(
            m_spUi->pTreeView->viewport()->mapFromGlobal(pContextEvt->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pTreeView->model()))
        {
          pModel = dynamic_cast<CDialogEditorTreeModel*>(pProxy->sourceModel());
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
void CEditorDialogWidget::dragEnterEvent(QDragEnterEvent* pEvent)
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
void CEditorDialogWidget::dropEvent(QDropEvent* pEvent)
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
      if (nullptr != dialog_tree::LoadDialogsFromSource({sPath}, m_spCurrentProject, &bOk) && bOk)
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
  QString GetResourceSource(CEditorDialogWidget* pThis,
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
        if (QFileInfo(PhysicalResourcePath(spResource)).suffix() != joip_resource::c_sDialogFileType)
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
                CCommandAddNewDialogFile* pCmd =
                    new CCommandAddNewDialogFile(spCurrentProject, pEditorModel, pThis);
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
        pActionBar->m_spUi->AddDialog->parentWidget()->mapToGlobal(
            pActionBar->m_spUi->AddDialog->pos());
    modelMenu.exec(p);

    return sSourceResource;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotAddDialogClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (nullptr == ActionBar() || nullptr == EditorModel()) { return; }

  QPointer<CEditorDialogWidget> pThis(this);

  qint32 iInsertPos = -1;
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  qint32 iTypeOfSelectedNode =
    DialogModel()->data(idxSelected, CDialogEditorTreeModel::eTypeRole).toInt();
  QString sSourceResource;

  switch(iTypeOfSelectedNode)
  {
    case EDialogTreeNodeType::eDialogFragment:
      // not allowed
      return;
    case EDialogTreeNodeType::eRoot: [[fallthrough]];
    case EDialogTreeNodeType::eCategory:
    {
      iInsertPos = DialogModel()->rowCount(idxSelected);
    } break;
    case EDialogTreeNodeType::eDialog:
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
    std::shared_ptr<CDialogNodeDialog> spNewNode = std::make_shared<CDialogNodeDialog>();
    spNewNode->m_sName = QUuid::createUuid().toString();
    spNewNode->m_sFileId = sSourceResource;
    spNewNode->m_bHasCondition = false;

    std::shared_ptr<CDialogData> spNewNodeData = std::make_shared<CDialogData>();
    spNewNodeData->m_wpParent = spNewNode;
    spNewNodeData->m_sName = QUuid::createUuid().toString();
    spNewNodeData->m_sFileId = sSourceResource;

    spNewNode->m_vspChildren.push_back(spNewNodeData);
    vsPath << spNewNode->m_sName;

    UndoStack()->push(new CCommandAddDialogNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotAddDialogFragmentClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (nullptr == ActionBar() || nullptr == EditorModel()) { return; }

  QPointer<CEditorDialogWidget> pThis(this);

  qint32 iInsertPos = -1;
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  qint32 iTypeOfSelectedNode =
      DialogModel()->data(idxSelected, CDialogEditorTreeModel::eTypeRole).toInt();
  QString sSourceResource;
  bool bNewNode = false;

  switch(iTypeOfSelectedNode)
  {
    case EDialogTreeNodeType::eRoot: [[fallthrough]];
    case EDialogTreeNodeType::eCategory:
    {
      bNewNode = true;
      iInsertPos = DialogModel()->rowCount(idxSelected);
    } break;
    case EDialogTreeNodeType::eDialogFragment:
      bNewNode = false;
      iInsertPos = idxSelected.row()+1;
      vsPath.erase(vsPath.begin()+vsPath.size()-1);
      sSourceResource =
          DialogModel()->data(idxSelected.parent(), CDialogEditorTreeModel::eResourceRole).toString();
      break;
    case EDialogTreeNodeType::eDialog:
    {
      if (DialogModel()->HasCondition(idxSelected))
      {
        bNewNode = false;
        iInsertPos = DialogModel()->rowCount(idxSelected);
        sSourceResource =
            DialogModel()->data(idxSelected, CDialogEditorTreeModel::eResourceRole).toString();
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
    std::shared_ptr<CDialogNode> spNewNode;
    if (bNewNode)
    {
      std::shared_ptr<CDialogNodeDialog> spNewNodeI = std::make_shared<CDialogNodeDialog>();
      spNewNodeI->m_sName = QUuid::createUuid().toString();
      spNewNodeI->m_sFileId = sSourceResource;
      spNewNodeI->m_bHasCondition = true;

      std::shared_ptr<CDialogData> spNewNodeData = std::make_shared<CDialogData>();
      spNewNodeData->m_wpParent = spNewNodeI;
      spNewNodeData->m_sName = QUuid::createUuid().toString();
      spNewNodeData->m_sFileId = sSourceResource;

      spNewNodeI->m_vspChildren.push_back(spNewNodeData);
      vsPath << spNewNodeI->m_sName;

      spNewNode = spNewNodeI;
    }
    else
    {
      std::shared_ptr<CDialogData> spNewNodeData = std::make_shared<CDialogData>();
      spNewNodeData->m_sName = QUuid::createUuid().toString();
      spNewNodeData->m_sFileId = sSourceResource;
      spNewNode = spNewNodeData;
    }

    UndoStack()->push(new CCommandAddDialogNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotAddDialogcategoryClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  qint32 iInsertPos = DialogModel()->rowCount(idxSelected);

  qint32 iTypeOfSelectedNode =
      DialogModel()->data(idxSelected, CDialogEditorTreeModel::eTypeRole).toInt();

  if (nullptr != UndoStack() && EDialogTreeNodeType::eDialogFragment != iTypeOfSelectedNode)
  {
    switch (iTypeOfSelectedNode)
    {
      case EDialogTreeNodeType::eDialog:
      {
        vsPath.erase(vsPath.begin() + vsPath.size()-1);
        iInsertPos += idxSelected.row()+1;
      } break;
      default: break;
    }

    std::shared_ptr<CDialogNodeCategory> spNewNode = std::make_shared<CDialogNodeCategory>();
    spNewNode->m_sName = "New Category";
    vsPath << spNewNode->m_sName;

    UndoStack()->push(new CCommandAddDialogNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNewNode, DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotRemoveDialogClicked()
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandRemoveDialogNode(m_spCurrentProject, vsPath,
                                                   DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotEditDialogClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  std::shared_ptr<CDialogNode> spNode = DialogModel()->Node(idxSelected);
  std::shared_ptr<CDialogNodeDialog> spDNode =
      std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);

  if (EDialogTreeNodeType::eDialogFragment == spNode->m_type._to_integral() ||
      (EDialogTreeNodeType::eDialog == spNode->m_type._to_integral() && !spDNode->m_bHasCondition))
  {
    m_spPropertiesOverlay->SetNode(vsPath, spNode);
    m_spPropertiesOverlay->Toggle();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotDialogChanged(QStringList vsPath, const std::shared_ptr<CDialogNode>& spNode)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  if (nullptr != UndoStack())
  {
    UndoStack()->push(new CCommandChangeParameters(m_spCurrentProject, vsPath, spNode,
                                                   DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotEditDialogTagsClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  std::shared_ptr<CDialogNode> spNode = DialogModel()->Node(idxSelected);
  std::shared_ptr<CDialogNodeDialog> spDNode =
      std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);

  if (EDialogTreeNodeType::eDialog == spNode->m_type._to_integral() && nullptr != spDNode)
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
void CEditorDialogWidget::SlotExpandAllNodes()
{
  IterateTreeItems(m_spUi->pTreeView, QModelIndex(), [this](const QModelIndex& idx) {
    m_spUi->pTreeView->expand(idx);
  });
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotFilterChanged(const QString& sText)
{
  WIDGET_INITIALIZED_GUARD

  QPointer<CDialogEditorSortFilterProxyModel> pProxyModel =
      dynamic_cast<CDialogEditorSortFilterProxyModel*>(m_spUi->pTreeView->model());

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
void CEditorDialogWidget::SlotCopy()
{
  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  std::shared_ptr<CDialogNode> spNode = DialogModel()->Node(idxSelected);

  QClipboard* pClipboard = QGuiApplication::clipboard();
  pClipboard->setText(QString::fromUtf8(dialog_tree::SerializeNode(spNode)));
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::SlotPaste()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QClipboard* pClipboard = QGuiApplication::clipboard();
  QString sCopied = pClipboard->text();

  // Try to read first to see if there are errros.
  std::shared_ptr<CDialogNode> spNode =
      dialog_tree::DeserializeNode(sCopied.toUtf8(), m_spCurrentProject);
  if (nullptr == spNode)
  {
    return;
  }

  QModelIndex idxSelected = m_pProxy->mapToSource(m_spUi->pTreeView->currentIndex());
  QStringList vsPath = DialogModel()->Path(idxSelected);
  qint32 iInsertPos = DialogModel()->rowCount(idxSelected);
  qint32 iTypeOfSelectedNode =
      DialogModel()->data(idxSelected, CDialogEditorTreeModel::eTypeRole).toInt();

  switch(iTypeOfSelectedNode)
  {
    case EDialogTreeNodeType::eRoot: [[fallthrough]];
    case EDialogTreeNodeType::eCategory:
    {
      switch(spNode->m_type)
      {
        case EDialogTreeNodeType::eRoot: return;
        case EDialogTreeNodeType::eCategory:
        {
          iInsertPos = DialogModel()->rowCount(idxSelected);
        } break;
        case EDialogTreeNodeType::eDialog:
        {
          iInsertPos = DialogModel()->rowCount(idxSelected);
        } break;
        case EDialogTreeNodeType::eDialogFragment: return;
      }
    } break;
    case EDialogTreeNodeType::eDialog:
    {
      switch(spNode->m_type)
      {
        case EDialogTreeNodeType::eRoot: return;
        case EDialogTreeNodeType::eCategory:
        {
          iInsertPos = idxSelected.row()+1;
          vsPath.erase(vsPath.begin()+vsPath.size()-1);
        } break;
        case EDialogTreeNodeType::eDialog:
        {
          iInsertPos = idxSelected.row()+1;
          vsPath.erase(vsPath.begin()+vsPath.size()-1);
        } break;
        case EDialogTreeNodeType::eDialogFragment:
        {
          if (DialogModel()->HasCondition(idxSelected))
          {
            iInsertPos = DialogModel()->rowCount(idxSelected);
          }
          else
          {
            return;
          }
        } break;
      }
    } break;
    case EDialogTreeNodeType::eDialogFragment:
    {
      switch(spNode->m_type)
      {
        case EDialogTreeNodeType::eRoot: [[fallthrough]];
        case EDialogTreeNodeType::eCategory: [[fallthrough]];
        case EDialogTreeNodeType::eDialog: return;
        case EDialogTreeNodeType::eDialogFragment:
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
    UndoStack()->push(new CCommandAddDialogNode(m_spCurrentProject, vsPath,
                                                iInsertPos, spNode, DialogModel()));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDialogWidget::ShowContextMenu(CDialogEditorTreeModel* pModel, const QModelIndex& idx,
                                          const QPoint& globalPos)
{
  if (nullptr != pModel && pModel == dynamic_cast<const CDialogEditorTreeModel*>(idx.model()))
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
    pAction->setEnabled(EDialogTreeNodeType::eDialog == spNode->m_type._to_integral() ||
                        EDialogTreeNodeType::eDialogFragment == spNode->m_type._to_integral());
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      auto spNodeDat = std::dynamic_pointer_cast<CDialogData>(spNode);
      QStringList vsStr;
      if (nullptr == spNodeDat && spNode->m_vspChildren.size() > 0)
      {
        for (const auto& spChild : spNode->m_vspChildren)
        {
          auto spChildCasted = std::dynamic_pointer_cast<CDialogData>(spChild);
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
    pAction->setEnabled(EDialogTreeNodeType::eDialogFragment == spNode->m_type._to_integral());
    connect(pAction, &QAction::triggered, pModel, [spNode]() {
      QClipboard* pClipboard = QGuiApplication::clipboard();
      auto spNodeDat = std::dynamic_pointer_cast<CDialogData>(spNode);
      QStringList vsStr;
      if (nullptr == spNodeDat && spNode->m_vspChildren.size() > 0)
      {
        for (const auto& spChild : spNode->m_vspChildren)
        {
          auto spChildCasted = std::dynamic_pointer_cast<CDialogData>(spChild);
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
      auto spNodeDat = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
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
