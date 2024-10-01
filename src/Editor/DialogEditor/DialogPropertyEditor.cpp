#include "DialogPropertyEditor.h"
#include "ui_DialogPropertyEditor.h"
#include "Application.h"

#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"

#include "Systems/Resource.h"

#include <QScrollBar>

CDialogPropertyEditor::CDialogPropertyEditor(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CDialogPropertyEditor>()),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spUi->setupUi(this);
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CDialogPropertyEditor::~CDialogPropertyEditor()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  m_bInitialized = false;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->FilterForTypes({EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CDialogPropertyEditor::SlotCurrentChanged);

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
void CDialogPropertyEditor::SetNode(QStringList vsPath,
                                    const std::shared_ptr<CDialogNode>& spNode)
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
      if (QFileInfo(PhysicalResourcePath(spResource)).suffix() != joip_resource::c_sDialogFileType)
      { continue; }
      m_spUi->pFileComboBox->addItem(sName, sName);
    }
  }

  switch (spNode->m_type)
  {
    case EDialogTreeNodeType::eDialog:
    {
      auto spDialog = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
      if (!spDialog->m_vspChildren.empty())
      {
        auto spDialogData = std::dynamic_pointer_cast<CDialogData>(spDialog->m_vspChildren[0]);
        m_spNode->m_vspChildren.push_back(spDialogData->Clone());
        spDialogData->m_wpParent = m_spNode;

        m_spUi->pNameLineEdit->setText(spDialog->m_sName);
        qint32 iIdx = m_spUi->pFileComboBox->findData(spDialog->m_sFileId);
        if (-1 != iIdx)
        {
          m_spUi->pFileComboBox->setCurrentIndex(iIdx);
        }
        m_spUi->pConditionContainer->setVisible(false);
        m_spUi->pSkippableCheckBox->setChecked(spDialogData->m_bSkipable);
        m_spUi->pAutoCheckBox->setChecked(spDialogData->m_iWaitTimeMs < 0);
        m_spUi->pWaitTimeEdit->setEnabled(spDialogData->m_iWaitTimeMs >= 0);
        m_spUi->pWaitTimeEdit->setTime(spDialogData->m_iWaitTimeMs < 0 ?
                                           QTime() :
                                           QTime().addMSecs(qint32(spDialogData->m_iWaitTimeMs)));
        m_spUi->pTextEdit->setText(spDialogData->m_sString);
        m_spUi->pResourceLineEdit->setText(spDialogData->m_sSoundResource);
      }
    } break;
    case EDialogTreeNodeType::eDialogFragment:
    {
      auto spDialogData = std::dynamic_pointer_cast<CDialogData>(m_spNode);
      m_spUi->pNameLineEdit->setText(spDialogData->m_sName);
      qint32 iIdx = m_spUi->pFileComboBox->findData(spDialogData->m_sFileId);
      if (-1 != iIdx)
      {
        m_spUi->pFileComboBox->setCurrentIndex(iIdx);
      }
      m_spUi->pConditionContainer->setVisible(true);
      m_spUi->pConditionLineEdit->setText(spDialogData->m_sCondition);
      m_spUi->pAutoCheckBox->setChecked(spDialogData->m_iWaitTimeMs < 0);
      m_spUi->pWaitTimeEdit->setEnabled(spDialogData->m_iWaitTimeMs >= 0);
      m_spUi->pWaitTimeEdit->setTime(spDialogData->m_iWaitTimeMs < 0 ?
                                         QTime() :
                                         QTime().addMSecs(qint32(spDialogData->m_iWaitTimeMs)));
      m_spUi->pTextEdit->setText(spDialogData->m_sString);
      m_spUi->pResourceLineEdit->setText(spDialogData->m_sSoundResource);
    } break;
    default: break;
  }

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::UnloadProject()
{
  m_spCurrentProject = nullptr;
  m_vsPath = QStringList();
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::Resize()
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
void CDialogPropertyEditor::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pNameLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_spNode->m_sName = m_spUi->pNameLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pFileComboBox_currentIndexChanged(qint32 iIdx)
{
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
void CDialogPropertyEditor::on_pConditionLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  if (auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode))
  {
    spDialog->m_sCondition = m_spUi->pConditionLineEdit->text();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pSkippableCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode);
  if (nullptr == spDialog && !m_spNode->m_vspChildren.empty())
  {
    spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialog)
  {
    spDialog->m_bSkipable = m_spUi->pSkippableCheckBox->isChecked();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pAutoCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }

  m_spUi->pWaitTimeEdit->setEnabled(!bChecked);

  auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode);
  if (nullptr == spDialog && !m_spNode->m_vspChildren.empty())
  {
    spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialog)
  {
    spDialog->m_iWaitTimeMs = bChecked ? -1 : m_spUi->pWaitTimeEdit->time().msecsSinceStartOfDay();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pWaitTimeEdit_timeChanged(const QTime &time)
{
  if (!m_bInitialized) { return; }
  auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode);
  if (nullptr == spDialog && !m_spNode->m_vspChildren.empty())
  {
    spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialog)
  {
    spDialog->m_iWaitTimeMs = time.msecsSinceStartOfDay();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode);
  if (nullptr == spDialog && !m_spNode->m_vspChildren.empty())
  {
    spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialog)
  {
    spDialog->m_sSoundResource = m_spUi->pResourceLineEdit->text();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  auto spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode);
  if (nullptr == spDialog && !m_spNode->m_vspChildren.empty())
  {
    spDialog = std::dynamic_pointer_cast<CDialogData>(m_spNode->m_vspChildren[0]);
  }
  if (nullptr != spDialog)
  {
    spDialog->m_sString = m_spUi->pTextEdit->toPlainText();
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pResourceLineEdit->clear();
  on_pResourceLineEdit_editingFinished();
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CDialogPropertyEditor::on_pConfirmButton_clicked()
{
  if (!m_bInitialized) { return; }
  emit SignalDialogChanged(m_vsPath, m_spNode);
  Hide();
  m_spNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::on_pCancelButton_clicked()
{
  Hide();
  m_spNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDialogPropertyEditor::SlotCurrentChanged(const QModelIndex &current,
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
