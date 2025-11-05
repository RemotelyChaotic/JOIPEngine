#include "ResourceSnippetOverlay.h"
#include "Application.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Database/Resource.h"
#include "ui_ResourceSnippetOverlay.h"
#include <QScrollBar>

namespace
{
  const double c_dSliderScaling = 10000;
}

//----------------------------------------------------------------------------------------
//
CResourceSnippetOverlay::CResourceSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(std::make_unique<Ui::CResourceSnippetOverlay>()),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_data()
{
  m_spUi->setupUi(this);
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CResourceSnippetOverlay::~CResourceSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  SetInitialized(false);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie, EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceSnippetOverlay::SlotCurrentChanged);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::Resize()
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
void CResourceSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sResource = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pResourceLineEdit->clear();
  m_data.m_sResource = QString();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pPlayRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::ePlayShow;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pPauseRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::ePause;
  }
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pStopRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::eStop;
  }
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pSeekRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::eSeek;
  }
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pSeekSpinBox_valueChanged(qint32 iValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_iSeekTime = iValue;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pWaitForFinishedCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bWaitForFinished = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pVolumeCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetVolume = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pVolumeSlider_sliderReleased()
{
  if (!m_bInitialized) { return; }
  double dVolume = static_cast<double>(m_spUi->pVolumeSlider->value()) / c_dSliderScaling;
  m_data.m_dVolume = dVolume;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pLoopsCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bLoops = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pLoopsSpinBox_valueChanged(qint32 iValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_iLoops = iValue;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pStartAtCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bStartAt = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pStartAtSpinBox_valueChanged(qint32 iValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_iStartAt = iValue;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pEndAtCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bEndAt = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pEndAtSpinBox_valueChanged(qint32 iValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_iEndAt = iValue;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CResourceSnippetOverlay::on_pConfirmButton_clicked()
{
  auto spGenerator = CodeGenerator();
  if (nullptr != spGenerator)
  {
    emit SignalCodeGenerated(spGenerator->Generate(m_data, m_spCurrentProject));
  }
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
                                                   const QModelIndex& previous)
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
