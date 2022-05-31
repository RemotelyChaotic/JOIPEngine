#include "MetronomeSnippetOverlay.h"
#include "Application.h"
#include "ScriptEditorWidget.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "ui_MetronomeSnippetOverlay.h"

#include <QLineEdit>
#include <QScrollBar>

namespace  {
  const char* c_sIndexProperty = "Index";
  const double c_dSliderScaling = 10000;
}

//----------------------------------------------------------------------------------------
//
CMetronomeSnippetOverlay::CMetronomeSnippetOverlay(CScriptEditorWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CMetronomeSnippetOverlay>()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pEditor(pParent),
  m_bInitialized(true),
  m_data()
{
  m_spUi->setupUi(this);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eSound});
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  m_spUi->pGeneralScrollArea->setWidgetResizable(true);
  m_spUi->pBeatScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
  m_vScrollAreas = {
    m_spUi->pGeneralScrollArea,
    m_spUi->pBeatScrollArea
  };
}

CMetronomeSnippetOverlay::~CMetronomeSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  m_bInitialized = false;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CMetronomeSnippetOverlay::SlotCurrentChanged);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::UnloadProject()
{
  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::Resize()
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
  if (m_pTargetWidget->width() > m_pTargetWidget->height())
  {
    m_spUi->pTabWidget->setTabPosition(QTabWidget::TabPosition::North);
  }
  else
  {
    m_spUi->pTabWidget->setTabPosition(QTabWidget::TabPosition::East);
  }

  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(newSize);

  qint32 iLeftMargin = 0;
  qint32 iRightMargin = 0;
  layout()->getContentsMargins(&iLeftMargin, nullptr, &iRightMargin, nullptr);
  for (QPointer<QScrollArea> pArea : m_vScrollAreas)
  {
    qint32 iLeftMarginInner = 0;
    qint32 iRightMarginInner = 0;
    pArea->widget()->layout()->getContentsMargins(&iLeftMarginInner, nullptr,
                                                  &iRightMarginInner, nullptr);
    pArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pArea->widget()->setMinimumWidth(
          newSize.width() - pArea->verticalScrollBar()->width() -
          pArea->widget()->layout()->spacing() - iLeftMargin - iLeftMarginInner -
          iRightMargin - iRightMarginInner - 1);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::Show()
{
  COverlayBase::Show();
  Initialize();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pPlayRadioButton_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bStart = bState;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pStopRadioButton_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bStop = bState;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pBpmCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetBpm = bState;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pBpmSpinBox_valueChanged(int iValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_iBpm = iValue;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pPatternCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetPattern = bState;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_AddPatternElemButton_clicked()
{
  if (!m_bInitialized) { return; }

  QDoubleSpinBox* pSpinbox = new QDoubleSpinBox(m_spUi->pPatternTableWidget);
  pSpinbox->setRange(0.1, 99.0);
  pSpinbox->setValue(1.0);
  pSpinbox->setSingleStep(0.5);

  connect(pSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CMetronomeSnippetOverlay::SlotPatternValueChanged);

  qint32 iIndex = 0;
  if (m_data.m_vdPatternElems.size() == 0)
  {
    iIndex = 0;
  }
  else
  {
    iIndex = std::next(m_data.m_vdPatternElems.end(), -1)->first + 1;
  }
  m_data.m_vdPatternElems.insert({iIndex, 1.0});

  m_spUi->pPatternTableWidget->insertRow(iIndex);
  m_spUi->pPatternTableWidget->setCellWidget(iIndex, 0, pSpinbox);
  pSpinbox->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_RemovePatternElemButton_clicked()
{
  if (!m_bInitialized) { return; }

  QModelIndex index = m_spUi->pPatternTableWidget->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vdPatternElems.size() > static_cast<size_t>(index.row()))
  {
    QDoubleSpinBox* pSpinbox = dynamic_cast<QDoubleSpinBox*>(
      m_spUi->pPatternTableWidget->cellWidget(index.row(), index.column()));

    if (nullptr != pSpinbox)
    {
      qint32 iIndex = pSpinbox->property(c_sIndexProperty).toInt();
      auto it = m_data.m_vdPatternElems.find(iIndex);
      if (m_data.m_vdPatternElems.end() != it)
      {
        m_data.m_vdPatternElems.erase(it);
        m_spUi->pPatternTableWidget->removeRow(index.row());
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::SlotPatternValueChanged(double dValue)
{
  qint32 iIndex = sender()->property(c_sIndexProperty).toInt();
  auto it = m_data.m_vdPatternElems.find(iIndex);
  if (m_data.m_vdPatternElems.end() != it)
  {
    it->second = dValue;
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pMuteCheckBox_toggled(bool bValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetMute = bValue;
}


//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pVolumeCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetVolume = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pVolumeSlider_sliderReleased()
{
  if (!m_bInitialized) { return; }
  double dVolume = static_cast<double>(m_spUi->pVolumeSlider->value()) / c_dSliderScaling;
  m_data.m_dVolume = dVolume;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pSetBeatSoundCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetBeatSound = bState;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sBeatSound = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CMetronomeSnippetOverlay::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }

  m_data.m_sBeatSound = QString();
  m_spUi->pResourceLineEdit->setText(QString());
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pConfirmButton_clicked()
{
  QString sCode;

  if (m_data.m_bSetBpm)
  {
    QString sText = "metronome.setBpm(%1);\n";
    sCode += sText.arg(m_data.m_iBpm);
  }
  if (m_data.m_bSetPattern)
  {
    QString sText = "metronome.setPattern([%1]);\n";
    QStringList vsPatternElems;
    for (auto it = m_data.m_vdPatternElems.begin(); m_data.m_vdPatternElems.end() != it; ++it)
    {
      vsPatternElems << QString::number(it->second);
    }
    sCode += sText.arg(vsPatternElems.join(","));
  }

  sCode += QString("metronome.setMuted(%1);\n")
      .arg(m_data.m_bSetMute ? "true" : "false");

  if (m_data.m_bSetBeatSound)
  {
    EResourceType type = EResourceType::eSound;
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager && nullptr != m_spCurrentProject)
    {
      tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_data.m_sBeatSound);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        type = spResource->m_type;
      }
    }
    sCode += QString("metronome.setBeatResource(%1);\n")
        .arg((m_data.m_sBeatSound.isNull() || type._to_integral() != EResourceType::eSound) ?
             "null" :
             "\"" + m_data.m_sBeatSound + "\"");
  }
  if (m_data.m_bSetVolume)
  {
    sCode += QString("metronome.setVolume(%2);\n").arg(m_data.m_dVolume);
  }
  if (m_data.m_bStart)
  {
    sCode += QString("metronome.start();\n");
  }
  if (m_data.m_bStop)
  {
    sCode += QString("metronome.stop();\n");
  }

  emit SignalMetronomeSnippetCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
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

//----------------------------------------------------------------------------------------
//
void CMetronomeSnippetOverlay::Initialize()
{
  m_bInitialized = false;
  m_data = SMetronomeSnippetCode();
  m_spUi->pPlayRadioButton->setChecked(true);
  m_spUi->pBpmCheckBox->setChecked(false);
  m_spUi->pBpmSpinBox->setValue(60);
  m_spUi->pPatternCheckBox->setChecked(false);
  m_spUi->pSetBeatSoundCheckBox->setChecked(false);
  m_spUi->pResourceLineEdit->clear();
  while (m_spUi->pPatternTableWidget->rowCount() > 0)
  {
    m_spUi->pPatternTableWidget->removeRow(0);
  }
  m_bInitialized = true;
}
