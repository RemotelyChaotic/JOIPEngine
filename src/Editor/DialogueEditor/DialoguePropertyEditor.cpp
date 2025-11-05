#include "DialoguePropertyEditor.h"
#include "ui_DialoguePropertyEditor.h"
#include "Application.h"

#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"

#include "Systems/Database/Resource.h"

#include <QScrollBar>

CDialoguePropertyEditor::CDialoguePropertyEditor(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CDialoguePropertyEditor>()),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spUi->setupUi(this);
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CDialoguePropertyEditor::~CDialoguePropertyEditor()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  m_bInitialized = false;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->FilterForTypes({EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CDialoguePropertyEditor::SlotCurrentChanged);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::SetNode(QStringList vsPath,
                                    const std::shared_ptr<CDialogueNode>& spNode)
{
  m_bInitialized = false;

  m_vsPath = vsPath;
  m_spNode = spNode->Clone();

  m_spUi->pFileComboBox->clear();
  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);
    for (const auto& [sName, spResource] : m_spCurrentProject->m_spResourcesMap)
    {
      QReadLocker l(&spResource->m_rwLock);
      if (EResourceType::eDatabase != spResource->m_type._to_integral()) { continue; }
      if (QFileInfo(PhysicalResourcePath(spResource)).suffix() != joip_resource::c_sDialogueFileType)
      { continue; }
      m_spUi->pFileComboBox->addItem(sName, sName);
    }
  }

  switch (spNode->m_type)
  {
    case EDialogueTreeNodeType::eDialogue:
    {
      auto spDialogue = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
      if (!spDialogue->m_vspChildren.empty())
      {
        auto spDialogueData = std::dynamic_pointer_cast<CDialogueData>(spDialogue->m_vspChildren[0]);
        m_spNode->m_vspChildren.push_back(spDialogueData->Clone());
        spDialogueData->m_wpParent = m_spNode;

        m_spUi->pNameLineEdit->setText(spDialogue->m_sName);
        qint32 iIdx = m_spUi->pFileComboBox->findData(spDialogue->m_sFileId);
        if (-1 != iIdx)
        {
          m_spUi->pFileComboBox->setCurrentIndex(iIdx);
        }
        m_spUi->pConditionContainer->setVisible(false);
        m_spUi->pSkippableCheckBox->setChecked(spDialogueData->m_bSkipable);
        m_spUi->pAutoCheckBox->setChecked(spDialogueData->m_iWaitTimeMs < 0);
        m_spUi->pWaitTimeEdit->setEnabled(spDialogueData->m_iWaitTimeMs >= 0);
        m_spUi->pWaitTimeEdit->setTime(spDialogueData->m_iWaitTimeMs < 0 ?
                                           QTime() :
                                           QTime().addMSecs(qint32(spDialogueData->m_iWaitTimeMs)));
        m_spUi->pTextEdit->setText(spDialogueData->m_sString);
        m_spUi->pResourceLineEdit->setText(spDialogueData->m_sSoundResource);
      }
    } break;
    case EDialogueTreeNodeType::eDialogueFragment:
    {
      auto spDialogueData = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
      m_spUi->pNameLineEdit->setText(spDialogueData->m_sName);
      qint32 iIdx = m_spUi->pFileComboBox->findData(spDialogueData->m_sFileId);
      if (-1 != iIdx)
      {
        m_spUi->pFileComboBox->setCurrentIndex(iIdx);
      }
      m_spUi->pConditionContainer->setVisible(true);
      m_spUi->pConditionLineEdit->setText(spDialogueData->m_sCondition);
      m_spUi->pAutoCheckBox->setChecked(spDialogueData->m_iWaitTimeMs < 0);
      m_spUi->pWaitTimeEdit->setEnabled(spDialogueData->m_iWaitTimeMs >= 0);
      m_spUi->pWaitTimeEdit->setTime(spDialogueData->m_iWaitTimeMs < 0 ?
                                         QTime() :
                                         QTime().addMSecs(qint32(spDialogueData->m_iWaitTimeMs)));
      m_spUi->pTextEdit->setText(spDialogueData->m_sString);
      m_spUi->pResourceLineEdit->setText(spDialogueData->m_sSoundResource);
    } break;
    default: break;
  }

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::UnloadProject()
{
  m_spCurrentProject = nullptr;
  m_vsPath = QStringList();
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::Resize()
{
  QSize newSize = m_preferredSize;
  if (m_pTargetWidget->geometry().width() < m_preferredSize.width())
  {
    newSize.setWidth(m_pTargetWidget->geometry().width());
  }
  if (m_pTargetWidget->geometry().height() < m_preferredSize.height())
  {
    newSize.setHeight(m_pTargetWidget->geometry().height());
  }

  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(newSize.width() / 2, newSize.height() / 2);

  move(newPos.x(), newPos.y());
  resize(newSize);

  m_spUi->pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_spUi->pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_spUi->pScrollArea->widget()->setMinimumWidth(
      newSize.width() - m_spUi->pScrollArea->verticalScrollBar()->width() -
      m_spUi->pScrollArea->widget()->layout()->spacing());
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pNameLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_spNode->m_sName = m_spUi->pNameLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pFileComboBox_currentIndexChanged(qint32 iIdx)
{
  Q_UNUSED(iIdx)
  if (!m_bInitialized) { return; }
  const QString sFile = m_spUi->pFileComboBox->currentData().toString();
  m_spNode->m_sFileId = sFile;
  if (m_spNode->m_vspChildren.size() > 0)
  {
    m_spNode->m_vspChildren[0]->m_sFileId = sFile;
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pConditionLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  if (auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode))
  {
    spDialogue->m_sCondition = m_spUi->pConditionLineEdit->text();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pSkippableCheckBox_toggled(bool bChecked)
{
  Q_UNUSED(bChecked)
  if (!m_bInitialized) { return; }
  auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
  if (nullptr == spDialogue && !m_spNode->m_vspChildren.empty())
  {
    spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialogue)
  {
    spDialogue->m_bSkipable = m_spUi->pSkippableCheckBox->isChecked();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pAutoCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }

  m_spUi->pWaitTimeEdit->setEnabled(!bChecked);

  auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
  if (nullptr == spDialogue && !m_spNode->m_vspChildren.empty())
  {
    spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialogue)
  {
    spDialogue->m_iWaitTimeMs = bChecked ? -1 : m_spUi->pWaitTimeEdit->time().msecsSinceStartOfDay();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pWaitTimeEdit_timeChanged(const QTime &time)
{
  if (!m_bInitialized) { return; }
  auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
  if (nullptr == spDialogue && !m_spNode->m_vspChildren.empty())
  {
    spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialogue)
  {
    spDialogue->m_iWaitTimeMs = time.msecsSinceStartOfDay();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
  if (nullptr == spDialogue && !m_spNode->m_vspChildren.empty())
  {
    spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialogue)
  {
    spDialogue->m_sSoundResource = m_spUi->pResourceLineEdit->text();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  auto spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode);
  if (nullptr == spDialogue && !m_spNode->m_vspChildren.empty())
  {
    spDialogue = std::dynamic_pointer_cast<CDialogueData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialogue)
  {
    spDialogue->m_sString = m_spUi->pTextEdit->toPlainText();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pResourceLineEdit->clear();
  on_pResourceLineEdit_editingFinished();
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());

  if (sText.isNull() || sText.isEmpty())
  {
    pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pConfirmButton_clicked()
{
  if (!m_bInitialized) { return; }
  emit SignalDialogueChanged(m_vsPath, m_spNode);
  Hide();
  m_spNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::on_pCancelButton_clicked()
{
  Hide();
  m_spNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDialoguePropertyEditor::SlotCurrentChanged(const QModelIndex &current,
                                               const QModelIndex &previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
      dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    m_spUi->pResourceLineEdit->setText(sName);
    on_pResourceLineEdit_editingFinished();
  }
}
