#include "TimelineSeqeunceInstructionWidgets.h"

#include "ui_TimelineSeqeunceInstructionWidgetEval.h"
#include "ui_TimelineSeqeunceInstructionWidgetLinearToy.h"
#include "ui_TimelineSeqeunceInstructionWidgetPauseAudio.h"
#include "ui_TimelineSeqeunceInstructionWidgetPlayAudio.h"
#include "ui_TimelineSeqeunceInstructionWidgetPlayVideo.h"
#include "ui_TimelineSeqeunceInstructionWidgetRotateToy.h"
#include "ui_TimelineSeqeunceInstructionWidgetRunScript.h"
#include "ui_TimelineSeqeunceInstructionWidgetShowMedia.h"
#include "ui_TimelineSeqeunceInstructionWidgetShowText.h"
#include "ui_TimelineSeqeunceInstructionWidgetSingleBeat.h"
#include "ui_TimelineSeqeunceInstructionWidgetStartPattern.h"
#include "ui_TimelineSeqeunceInstructionWidgetStopAudio.h"
#include "ui_TimelineSeqeunceInstructionWidgetVibrate.h"

#include "Editor/Resources/ResourceToolTip.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"

#include <QToolTip>

namespace
{
  const char* c_sIndexProperty = "Index";
  const double c_dSliderScaling = 10000;

  template<typename T>
  std::function<CTimelineSeqeunceInstructionWidgetBase*(QWidget* /*pParent*/)> Creator()
  {
    return [](QWidget* pParent) { return new T(pParent); };
  }
}

CTimelineSeqeunceInstructionWidgetBase* sequence::CreateWidgetFromInstruction(
    const std::shared_ptr<SSequenceInstruction>& spInstr, QWidget* pParent)
{
  static const std::map<QString, std::function<CTimelineSeqeunceInstructionWidgetBase*(QWidget* /*pParent*/)>> m_creatorMap =
  {
    {sequence::c_sInstructionIdBeat, Creator<CTimelineSeqeunceInstructionWidgetSingleBeat>()},
    {sequence::c_sInstructionIdStartPattern, Creator<CTimelineSeqeunceInstructionWidgetStartPattern>()},
    {sequence::c_sInstructionIdVibrate, Creator<CTimelineSeqeunceInstructionWidgetVibrate>()},
    {sequence::c_sInstructionIdLinearToy, Creator<CTimelineSeqeunceInstructionWidgetLinearToy>()},
    {sequence::c_sInstructionIdRotateToy, Creator<CTimelineSeqeunceInstructionWidgetRotateToy>()},
    {sequence::c_sInstructionIdShow, Creator<CTimelineSeqeunceInstructionWidgetShowMedia>()},
    {sequence::c_sInstructionIdPlayVideo, Creator<CTimelineSeqeunceInstructionWidgetPlayVideo>()},
    {sequence::c_sInstructionIdPlayAudio, Creator<CTimelineSeqeunceInstructionWidgetPlayAudio>()},
    {sequence::c_sInstructionIdPauseAudio, Creator<CTimelineSeqeunceInstructionWidgetPauseAudio>()},
    {sequence::c_sInstructionIdStopAudio, Creator<CTimelineSeqeunceInstructionWidgetStopAudio>()},
    {sequence::c_sInstructionIdShowText, Creator<CTimelineSeqeunceInstructionWidgetShowText>()},
    {sequence::c_sInstructionIdRunScript, Creator<CTimelineSeqeunceInstructionWidgetRunScript>()},
    {sequence::c_sInstructionIdEval, Creator<CTimelineSeqeunceInstructionWidgetEval>()}
  };
  auto it = m_creatorMap.find(spInstr->m_sInstructionType);
  if (m_creatorMap.end() != it)
  {
    return it->second(pParent);
  }
  return nullptr;
}

namespace
{
  //--------------------------------------------------------------------------------------
  //
  bool ResourceClickHandler(const QModelIndex& index, QTreeView* pResourceSelectTree,
                            CSlidingStackedWidget* pStackedWidget,
                            std::function<void(const QString&)> fnOnEdit)
  {
    auto pProxy =
        dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(
            pResourceSelectTree->model());
    auto pSource = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
    QModelIndex idxModel = pProxy->mapToSource(index);
    if (idxModel.isValid())
    {
      if (pSource->IsResourceType(idxModel))
      {
        tspResource spResource = pSource->ResourceForIndex(idxModel);
        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          pStackedWidget->SlideInPrev();
          if (nullptr != fnOnEdit)
          {
            fnOnEdit(spResource->m_sName);
          }
          return true;
        }
      }
    }
    return false;
  }

  //--------------------------------------------------------------------------------------
  //
  bool ResourceTreeEventFilter(QObject* pObj, QEvent* pEvt,
                               QTreeView* pResourceSelectTree,
                               CSlidingStackedWidget* pStackedWidget,
                               std::function<void(const QString&)> fnOnEdit)
  {
    if (pObj == pResourceSelectTree && nullptr != pEvt)
    {
      if (QEvent::KeyPress == pEvt->type())
      {
        QKeyEvent* pKeyEvt = static_cast<QKeyEvent*>(pEvt);
        if (pKeyEvt->key() == Qt::Key_Enter || pKeyEvt->key() == Qt::Key_Return)
        {
          QModelIndex idxProxy = pResourceSelectTree->currentIndex();
          return ResourceClickHandler(idxProxy, pResourceSelectTree, pStackedWidget,
                                      fnOnEdit);
        }
      }
    }
    if ((pObj == pResourceSelectTree ||
         pResourceSelectTree->viewport() == pObj) && nullptr != pEvt)
    {
      if (QEvent::ToolTip == pEvt->type())
      {
        QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(pEvt);

        QModelIndex index = pResourceSelectTree->indexAt(
            pResourceSelectTree->viewport()->mapFromGlobal(pHelpEvent->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(pResourceSelectTree->model()))
        {
          index = pProxy->mapToSource(index);
        }

        CResourceTreeItem* pItem = static_cast<CResourceTreeItem*>(index.internalPointer());
        if (nullptr != pItem)
        {
          if (EResourceTreeItemType::eResource == pItem->Type()._to_integral())
          {
            CResourceToolTip::showResource(pHelpEvent->globalPos(),
                                           pItem->Resource(),
                                           pItem->Data(resource_item::c_iColumnWarning).toString(),
                                           qobject_cast<QWidget*>(pObj));
          }
          else
          {
            QToolTip::showText(pHelpEvent->globalPos(),
                               pItem->Data(resource_item::c_iColumnToolTip).toString(),
                               qobject_cast<QWidget*>(pObj));
          }
          pEvt->ignore();
          return true;
        }
      }
    }
    return false;
  }
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetBase::CTimelineSeqeunceInstructionWidgetBase(QWidget* pParent) :
  QWidget{pParent},
  m_bIsInitializing(false)
{
  setAttribute(Qt::WA_TranslucentBackground);
}
CTimelineSeqeunceInstructionWidgetBase::~CTimelineSeqeunceInstructionWidgetBase()
{}

void CTimelineSeqeunceInstructionWidgetBase::EmitPropertiesChanged()
{
  if (!m_bIsInitializing)
  {
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetSingleBeat::CTimelineSeqeunceInstructionWidgetSingleBeat(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetSingleBeat>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pEnabledCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetSingleBeat::on_pEnabledCheckBox_toggled);
  connect(m_spUi->pVolumeSlider, &QSlider::valueChanged, this,
          &CTimelineSeqeunceInstructionWidgetSingleBeat::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetSingleBeat::~CTimelineSeqeunceInstructionWidgetSingleBeat() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetSingleBeat::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SSingleBeatInstruction>(spInstr);
  m_spUi->pEnabledCheckBox->setChecked(spInstrCasted->m_dVolume.has_value());
  m_spUi->pVolumeSlider->setValue(spInstrCasted->m_dVolume.value_or(0.0) * c_dSliderScaling);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetSingleBeat::Properties() const
{
  QVariantList args;
  if (m_spUi->pEnabledCheckBox->isChecked())
  {
    args << QVariant::fromValue(static_cast<double>(m_spUi->pVolumeSlider->value()) / c_dSliderScaling);
  }
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetSingleBeat::on_pEnabledCheckBox_toggled(bool bOn)
{
  m_spUi->pVolumeSlider->setEnabled(bOn);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetStartPattern::CTimelineSeqeunceInstructionWidgetStartPattern(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetStartPattern>())
{
  m_spUi->setupUi(this);
  m_spUi->pResourcesTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_spUi->pResourcesTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
  m_spUi->pResourcesTableWidget->horizontalHeader()->setStretchLastSection(false);
  m_spUi->pResourcesTableWidget->horizontalHeader()->resizeSection(1, 30);
  m_spUi->pResourcesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_spUi->AddButton->setProperty("styleSmall", true);
  m_spUi->RemoveButton->setProperty("styleSmall", true);
  m_spUi->AddPatternElemButton->setProperty("styleSmall", true);
  m_spUi->RemovePatternElemButton->setProperty("styleSmall", true);

  connect(m_spUi->AddButton, &QPushButton::clicked,
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::on_AddButton_clicked);
  connect(m_spUi->RemoveButton, &QPushButton::clicked,
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::on_RemoveButton_clicked);
  connect(m_spUi->AddPatternElemButton, &QPushButton::clicked,
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::on_AddPatternElemButton_clicked);
  connect(m_spUi->RemovePatternElemButton, &QPushButton::clicked,
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::on_RemovePatternElemButton_clicked);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::on_pResourceSelectTree_doubleClicked);

  connect(m_spUi->pBpmSpinBox, qOverload<qint32>(&QSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::EmitPropertiesChanged);
  connect(m_spUi->pVolumeCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetStartPattern::on_pVolumeCheckBox_toggled);
  connect(m_spUi->pVolumeSlider, &QSlider::valueChanged, this,
          &CTimelineSeqeunceInstructionWidgetStartPattern::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetStartPattern::~CTimelineSeqeunceInstructionWidgetStartPattern()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);


  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;
  m_iEditingRow = -1;

  while (m_spUi->pPatternTableWidget->rowCount() > 0)
  {
    m_spUi->pPatternTableWidget->removeRow(0);
  }
  while (m_spUi->pResourcesTableWidget->rowCount() > 0)
  {
    m_spUi->pResourcesTableWidget->removeRow(0);
  }

  auto spInstrCasted = std::dynamic_pointer_cast<SStartPatternInstruction>(spInstr);
  for (double dValue : spInstrCasted->m_vdPattern)
  {
    AddPatternElement(dValue);
  }
  m_spUi->pBpmSpinBox->setValue(spInstrCasted->m_iBpm);
  if (spInstrCasted->m_vsResources.has_value())
  {
    for (const QString& sResource : spInstrCasted->m_vsResources.value())
    {
      AddResourceElement(sResource);
    }
  }
  m_spUi->pVolumeCheckBox->setChecked(spInstrCasted->m_dVolume.has_value());
  m_spUi->pVolumeSlider->setValue(spInstrCasted->m_dVolume.value_or(0.0) * c_dSliderScaling);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetStartPattern::Properties() const
{
  QVariantList args;
  QVariantList vdPattern;
  for (qint32 i = 0; m_spUi->pPatternTableWidget->rowCount() > i; ++i)
  {
    QModelIndex index = m_spUi->pPatternTableWidget->model()->index(i, 0);
    QDoubleSpinBox* pSpinbox = dynamic_cast<QDoubleSpinBox*>(
        m_spUi->pPatternTableWidget->cellWidget(index.row(), index.column()));
    vdPattern.push_back(QVariant::fromValue(pSpinbox->value()));
  }
  args << QVariant::fromValue(vdPattern);
  args << QVariant::fromValue(m_spUi->pBpmSpinBox->value());
  QStringList vsResources;
  for (qint32 i = 0; m_spUi->pResourcesTableWidget->rowCount() > i; ++i)
  {
    QModelIndex index = m_spUi->pPatternTableWidget->model()->index(i, 0);
    vsResources << m_spUi->pPatternTableWidget->model()->data(index, Qt::EditRole).toString();
  }
  args << QVariant::fromValue(vsResources);
  if (m_spUi->pVolumeCheckBox->isChecked())
  {
    args << QVariant::fromValue(static_cast<double>(m_spUi->pVolumeSlider->value()) / c_dSliderScaling);
  }
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_pVolumeCheckBox_toggled(bool bOn)
{
  m_spUi->pVolumeSlider->setEnabled(bOn);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget,
                                        [this](const QString& sResource) {
      QModelIndex idx = m_spUi->pResourcesTableWidget->model()->index(m_iEditingRow, 0);
      m_spUi->pResourcesTableWidget->model()->setData(idx, sResource);
      m_iEditingRow = -1;
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_AddButton_clicked()
{
  if (m_bIsInitializing) { return; }

  AddResourceElement(QString());

  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_RemoveButton_clicked()
{
  if (m_bIsInitializing) { return; }

  QModelIndex index = m_spUi->pResourcesTableWidget->currentIndex();
  if (index.row() >= 0 &&
      m_spUi->pResourcesTableWidget->rowCount() > index.row())
  {
    m_spUi->pResourcesTableWidget->removeRow(index.row());
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_AddPatternElemButton_clicked()
{
  if (m_bIsInitializing) { return; }

  AddPatternElement(1.0);

  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::on_RemovePatternElemButton_clicked()
{
  if (m_bIsInitializing) { return; }

  QModelIndex index = m_spUi->pPatternTableWidget->currentIndex();
  if (index.row() >= 0 &&
      m_spUi->pPatternTableWidget->rowCount() > index.row())
  {
    m_spUi->pPatternTableWidget->removeRow(index.row());
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::AddPatternElement(double dValue)
{
  QDoubleSpinBox* pSpinbox = new QDoubleSpinBox(m_spUi->pPatternTableWidget);
  pSpinbox->setRange(0.1, 99.0);
  pSpinbox->setValue(dValue);
  pSpinbox->setSingleStep(0.5);

  connect(pSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetStartPattern::EmitPropertiesChanged);

  qint32 iIndex = m_spUi->pPatternTableWidget->rowCount();
  m_spUi->pPatternTableWidget->insertRow(iIndex);
  m_spUi->pPatternTableWidget->setCellWidget(iIndex, 0, pSpinbox);
  pSpinbox->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStartPattern::AddResourceElement(const QString& sElem)
{
  Q_UNUSED(sElem)
  qint32 iIndex = m_spUi->pPatternTableWidget->rowCount();
  QPushButton* pButton = new QPushButton();
  pButton->setObjectName("SearchButton");
  pButton->setProperty("styleSmall", true);

  connect(pButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, [this, iIndex]() mutable
  {
    m_iEditingRow = iIndex;
    m_spUi->pStackedWidget->SlideInNext();
  });

  m_spUi->pPatternTableWidget->insertRow(iIndex);
  QTableWidgetItem* item = new QTableWidgetItem("");
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  m_spUi->pPatternTableWidget->setItem(iIndex, 0, item);
  m_spUi->pPatternTableWidget->setCellWidget(iIndex, 1, pButton);
  pButton->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetStartPattern::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget,
                                           [this](const QString& sResource) {
    QModelIndex idx = m_spUi->pResourcesTableWidget->model()->index(m_iEditingRow, 0);
    m_spUi->pResourcesTableWidget->model()->setData(idx, sResource);
    m_iEditingRow = -1;
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetVibrate::CTimelineSeqeunceInstructionWidgetVibrate(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetVibrate>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pSpeedVibrateSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetVibrate::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetVibrate::~CTimelineSeqeunceInstructionWidgetVibrate() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetVibrate::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SVibrateInstruction>(spInstr);
  m_spUi->pSpeedVibrateSpinBox->setValue(spInstrCasted->m_dSpeed);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetVibrate::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pSpeedVibrateSpinBox->value());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetLinearToy::CTimelineSeqeunceInstructionWidgetLinearToy(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetLinearToy>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pDurationSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetLinearToy::EmitPropertiesChanged);
  connect(m_spUi->pPositionSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetLinearToy::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetLinearToy::~CTimelineSeqeunceInstructionWidgetLinearToy() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetLinearToy::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SLinearToyInstruction>(spInstr);
  m_spUi->pDurationSpinBox->setValue(spInstrCasted->m_dDurationS);
  m_spUi->pPositionSpinBox->setValue(spInstrCasted->m_dPosition);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetLinearToy::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pDurationSpinBox->value());
  args << QVariant::fromValue(m_spUi->pPositionSpinBox->value());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetRotateToy::CTimelineSeqeunceInstructionWidgetRotateToy(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetRotateToy>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pClockwiseCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetRotateToy::EmitPropertiesChanged);
  connect(m_spUi->pSpeedRotateSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetRotateToy::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetRotateToy::~CTimelineSeqeunceInstructionWidgetRotateToy() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetRotateToy::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SRotateToyInstruction>(spInstr);
  m_spUi->pClockwiseCheckBox->setChecked(spInstrCasted->m_bClockwise);
  m_spUi->pSpeedRotateSpinBox->setValue(spInstrCasted->m_dSpeed);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetRotateToy::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pClockwiseCheckBox->isChecked());
  args << QVariant::fromValue(m_spUi->pSpeedRotateSpinBox->value());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetShowMedia::CTimelineSeqeunceInstructionWidgetShowMedia(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetShowMedia>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetShowMedia::EmitPropertiesChanged);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, &CSlidingStackedWidget::SlideInNext);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetShowMedia::on_pResourceSelectTree_doubleClicked);
}
CTimelineSeqeunceInstructionWidgetShowMedia::~CTimelineSeqeunceInstructionWidgetShowMedia()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowMedia::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);


  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowMedia::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SShowMediaInstruction>(spInstr);
  m_spUi->pResourceLineEdit->setText(spInstrCasted->m_sResource);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetShowMedia::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pResourceLineEdit->text());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowMedia::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetShowMedia::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    emit SignalChangedProperties();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetPlayVideo::CTimelineSeqeunceInstructionWidgetPlayVideo(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetPlayVideo>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetPlayVideo::EmitPropertiesChanged);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, &CSlidingStackedWidget::SlideInNext);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetPlayVideo::on_pResourceSelectTree_doubleClicked);

  connect(m_spUi->pLoopsCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::on_pLoopsCheckBox_toggled);
  connect(m_spUi->pLoopsSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::EmitPropertiesChanged);
  connect(m_spUi->pStartAtCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::on_pStartAtCheckBox_toggled);
  connect(m_spUi->pStartAtSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::EmitPropertiesChanged);
  connect(m_spUi->pEndAtCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::on_pEndAtCheckBox_toggled);
  connect(m_spUi->pEndAtSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayVideo::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetPlayVideo::~CTimelineSeqeunceInstructionWidgetPlayVideo()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eMovie});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SPlayVideoInstruction>(spInstr);
  m_spUi->pResourceLineEdit->setText(spInstrCasted->m_sResource);
  m_spUi->pLoopsCheckBox->setChecked(spInstrCasted->m_iLoops.has_value());
  m_spUi->pLoopsSpinBox->setValue(spInstrCasted->m_iLoops.value_or(0));
  m_spUi->pStartAtCheckBox->setChecked(spInstrCasted->m_iStartAt.has_value());
  m_spUi->pStartAtSpinBox->setValue(spInstrCasted->m_iStartAt.value_or(0));
  m_spUi->pEndAtCheckBox->setChecked(spInstrCasted->m_iEndAt.has_value());
  m_spUi->pEndAtSpinBox->setValue(spInstrCasted->m_iEndAt.value_or(0));

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetPlayVideo::Properties() const
{
  std::shared_ptr<SPlayVideoInstruction> spInstr = std::make_shared<SPlayVideoInstruction>();
  spInstr->m_sInstructionType = sequence::c_sInstructionIdPlayVideo;
  spInstr->m_sResource = m_spUi->pResourceLineEdit->text();
  if (m_spUi->pLoopsCheckBox->isChecked())
  {
    spInstr->m_iLoops = m_spUi->pLoopsSpinBox->value();
  }
  if (m_spUi->pStartAtCheckBox->isChecked())
  {
    spInstr->m_iStartAt = m_spUi->pStartAtSpinBox->value();
  }
  if (m_spUi->pEndAtCheckBox->isChecked())
  {
    spInstr->m_iEndAt = m_spUi->pEndAtSpinBox->value();
  }
  return spInstr;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::on_pLoopsCheckBox_toggled(bool bEnabled)
{
  m_spUi->pLoopsSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::on_pStartAtCheckBox_toggled(bool bEnabled)
{
  m_spUi->pStartAtSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::on_pEndAtCheckBox_toggled(bool bEnabled)
{
  m_spUi->pEndAtSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayVideo::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetPlayVideo::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    emit SignalChangedProperties();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetPlayAudio::CTimelineSeqeunceInstructionWidgetPlayAudio(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetPlayAudio>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetPlayAudio::EmitPropertiesChanged);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, &CSlidingStackedWidget::SlideInNext);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetPlayAudio::on_pResourceSelectTree_doubleClicked);

  connect(m_spUi->pNameLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetPlayAudio::EmitPropertiesChanged);
  connect(m_spUi->pLoopsCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::on_pLoopsCheckBox_toggled);
  connect(m_spUi->pLoopsSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::EmitPropertiesChanged);
  connect(m_spUi->pStartAtCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::on_pStartAtCheckBox_toggled);
  connect(m_spUi->pStartAtSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::EmitPropertiesChanged);
  connect(m_spUi->pEndAtCheckBox, &QCheckBox::toggled, this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::on_pEndAtCheckBox_toggled);
  connect(m_spUi->pEndAtSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), this,
          &CTimelineSeqeunceInstructionWidgetPlayAudio::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetPlayAudio::~CTimelineSeqeunceInstructionWidgetPlayAudio()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);


  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SPlayAudioInstruction>(spInstr);
  m_spUi->pResourceLineEdit->setText(spInstrCasted->m_sResource);
  m_spUi->pNameLineEdit->setText(spInstrCasted->m_sName.value_or(QString()));
  m_spUi->pLoopsCheckBox->setChecked(spInstrCasted->m_iLoops.has_value());
  m_spUi->pLoopsSpinBox->setValue(spInstrCasted->m_iLoops.value_or(0));
  m_spUi->pStartAtCheckBox->setChecked(spInstrCasted->m_iStartAt.has_value());
  m_spUi->pStartAtSpinBox->setValue(spInstrCasted->m_iStartAt.value_or(0));
  m_spUi->pEndAtCheckBox->setChecked(spInstrCasted->m_iEndAt.has_value());
  m_spUi->pEndAtSpinBox->setValue(spInstrCasted->m_iEndAt.value_or(0));

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetPlayAudio::Properties() const
{
  std::shared_ptr<SPlayAudioInstruction> spInstr = std::make_shared<SPlayAudioInstruction>();
  spInstr->m_sInstructionType = sequence::c_sInstructionIdPlayAudio;
  spInstr->m_sResource = m_spUi->pResourceLineEdit->text();
  if (!m_spUi->pNameLineEdit->text().isEmpty())
  {
    spInstr->m_sName = m_spUi->pNameLineEdit->text();
  }
  if (m_spUi->pLoopsCheckBox->isChecked())
  {
    spInstr->m_iLoops = m_spUi->pLoopsSpinBox->value();
  }
  if (m_spUi->pStartAtCheckBox->isChecked())
  {
    spInstr->m_iStartAt = m_spUi->pStartAtSpinBox->value();
  }
  if (m_spUi->pEndAtCheckBox->isChecked())
  {
    spInstr->m_iEndAt = m_spUi->pEndAtSpinBox->value();
  }
  return spInstr;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::on_pLoopsCheckBox_toggled(bool bEnabled)
{
  m_spUi->pLoopsSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::on_pStartAtCheckBox_toggled(bool bEnabled)
{
  m_spUi->pStartAtSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::on_pEndAtCheckBox_toggled(bool bEnabled)
{
  m_spUi->pEndAtSpinBox->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPlayAudio::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetPlayAudio::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    emit SignalChangedProperties();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetPauseAudio::CTimelineSeqeunceInstructionWidgetPauseAudio(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetPauseAudio>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pNameLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetPauseAudio::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetPauseAudio::~CTimelineSeqeunceInstructionWidgetPauseAudio() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetPauseAudio::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SPauseAudioInstruction>(spInstr);
  m_spUi->pNameLineEdit->setText(spInstrCasted->m_sName);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetPauseAudio::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pNameLineEdit->text());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetStopAudio::CTimelineSeqeunceInstructionWidgetStopAudio(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetStopAudio>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pNameLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetStopAudio::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetStopAudio::~CTimelineSeqeunceInstructionWidgetStopAudio() = default;
//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetStopAudio::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SStopAudioInstruction>(spInstr);
  m_spUi->pNameLineEdit->setText(spInstrCasted->m_sName);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetStopAudio::Properties() const
{
  QVariantList args;
  args << QVariant::fromValue(m_spUi->pNameLineEdit->text());
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetShowText::CTimelineSeqeunceInstructionWidgetShowText(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetShowText>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetShowText::EmitPropertiesChanged);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, &CSlidingStackedWidget::SlideInNext);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pResourceSelectTree_doubleClicked);

  connect(m_spUi->pIconCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pIconCheckBox_toggled);
  connect(m_spUi->pSetSleepTimeCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pSetSleepTimeCheckBox_toggled);
  connect(m_spUi->pAutoTimeCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pAutoTimeCheckBox_toggled);
  connect(m_spUi->pSleepSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          this, &CTimelineSeqeunceInstructionWidgetShowText::EmitPropertiesChanged);
  connect(m_spUi->pSkippableCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::EmitPropertiesChanged);
  connect(m_spUi->pEnableTextColorCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pEnableTextColorCheckBox_toggled);
  connect(m_spUi->pTextColorPicker, &CColorPicker::SignalColorChanged,
          this, &CTimelineSeqeunceInstructionWidgetShowText::EmitPropertiesChanged);
  connect(m_spUi->pEnableBgColorCheckBox, &QCheckBox::toggled,
          this, &CTimelineSeqeunceInstructionWidgetShowText::on_pEnableBgColorCheckBox_toggled);
  connect(m_spUi->pBgColorPicker, &CColorPicker::SignalColorChanged,
          this, &CTimelineSeqeunceInstructionWidgetShowText::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetShowText::~CTimelineSeqeunceInstructionWidgetShowText()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);


  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SShowTextInstruction>(spInstr);

  m_spUi->pIconCheckBox->setChecked(spInstrCasted->m_sPortrait.has_value());
  m_spUi->pResourceLineEdit->setText(spInstrCasted->m_sPortrait.value_or(QString()));
  m_spUi->pSetSleepTimeCheckBox->setChecked(spInstrCasted->m_dSkippableWaitS.has_value());
  m_spUi->pAutoTimeCheckBox->setChecked(qFuzzyCompare(spInstrCasted->m_dSkippableWaitS.value_or(1.0), -1));
  m_spUi->pSleepSpinBox->setValue(spInstrCasted->m_dSkippableWaitS.value_or(0.0));
  m_spUi->pSkippableCheckBox->setChecked(spInstrCasted->m_bSkippable.value_or(false));
  m_spUi->pTextEdit->setPlainText(spInstrCasted->m_sText.value_or(QString()));

  m_spUi->pEnableTextColorCheckBox->setChecked(spInstrCasted->m_textColor.has_value());
  m_spUi->pTextColorPicker->SetColor(spInstrCasted->m_textColor.value_or(QColor()));
  m_spUi->pEnableBgColorCheckBox->setChecked(spInstrCasted->m_bgColor.has_value());
  m_spUi->pBgColorPicker->SetColor(spInstrCasted->m_bgColor.value_or(QColor()));

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetShowText::Properties() const
{
  std::shared_ptr<SShowTextInstruction> spInstr = std::make_shared<SShowTextInstruction>();
  spInstr->m_sInstructionType = sequence::c_sInstructionIdShowText;
  if (m_spUi->pIconCheckBox->isChecked())
  {
    spInstr->m_sPortrait = m_spUi->pResourceLineEdit->text();
  }
  if (m_spUi->pSetSleepTimeCheckBox->isChecked())
  {
    spInstr->m_dSkippableWaitS = m_spUi->pAutoTimeCheckBox->isChecked() ? -1.0 : m_spUi->pSleepSpinBox->value();
  }
  spInstr->m_bSkippable = m_spUi->pSkippableCheckBox->isChecked();
  QString sText = m_spUi->pTextEdit->toPlainText();
  if (!sText.isEmpty())
  {
    spInstr->m_sText = sText;;
  }
  if (m_spUi->pEnableTextColorCheckBox->isChecked())
  {
    spInstr->m_textColor = m_spUi->pTextColorPicker->Color();
  }
  if (m_spUi->pEnableBgColorCheckBox->isChecked())
  {
    spInstr->m_bgColor = m_spUi->pBgColorPicker->Color();
  }
  return spInstr;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pIconCheckBox_toggled(bool bEnabled)
{
  m_spUi->pResourceLineEdit->setEnabled(bEnabled);
  m_spUi->SearchButton->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pSetSleepTimeCheckBox_toggled(bool bEnabled)
{
  m_spUi->pAutoTimeCheckBox->setEnabled(bEnabled);
  m_spUi->pSleepSpinBox->setEnabled(bEnabled && m_spUi->pAutoTimeCheckBox->isChecked());
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pAutoTimeCheckBox_toggled(bool bEnabled)
{
  m_spUi->pSleepSpinBox->setEnabled(bEnabled && m_spUi->pSetSleepTimeCheckBox->isChecked());
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pEnableTextColorCheckBox_toggled(bool bEnabled)
{
  m_spUi->pTextColorPicker->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pEnableBgColorCheckBox_toggled(bool bEnabled)
{
  m_spUi->pBgColorPicker->setEnabled(bEnabled);
  EmitPropertiesChanged();
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetShowText::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetShowText::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    emit SignalChangedProperties();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetRunScript::CTimelineSeqeunceInstructionWidgetRunScript(QWidget* pParent) :
    CTimelineSeqeunceInstructionWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetRunScript>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CTimelineSeqeunceInstructionWidgetRunScript::EmitPropertiesChanged);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          m_spUi->pStackedWidget, &CSlidingStackedWidget::SlideInNext);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CTimelineSeqeunceInstructionWidgetRunScript::on_pResourceSelectTree_doubleClicked);
}
CTimelineSeqeunceInstructionWidgetRunScript::~CTimelineSeqeunceInstructionWidgetRunScript()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetRunScript::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eScript, EResourceType::eSequence});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  connect(m_spUi->pFilter, &CSearchWidget::SignalFilterChanged,
          pProxyModel, [pProxyModel](const QString sFilter) { pProxyModel->setFilterRegExp(sFilter);} );

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pResourceSelectTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);


  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetRunScript::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SRunScriptInstruction>(spInstr);
  m_spUi->pResourceLineEdit->setText(spInstrCasted->m_sResource);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetRunScript::Properties() const
{
  QVariantList args;
  args << m_spUi->pResourceLineEdit->text();
  return sequence::CreateInstruction(m_sType, args);
}

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetRunScript::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  bool bFiltered = ResourceClickHandler(index, m_spUi->pResourceSelectTree,
                                        m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    EmitPropertiesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CTimelineSeqeunceInstructionWidgetRunScript::eventFilter(QObject* pObj, QEvent* pEvt)
{
  bool bFiltered = ResourceTreeEventFilter(pObj, pEvt, m_spUi->pResourceSelectTree,
                                           m_spUi->pStackedWidget, [this](const QString& sResource) {
    m_spUi->pResourceLineEdit->setText(sResource);
  });
  if (bFiltered)
  {
    emit SignalChangedProperties();
  }
  return bFiltered;
}

//----------------------------------------------------------------------------------------
//
CTimelineSeqeunceInstructionWidgetEval::CTimelineSeqeunceInstructionWidgetEval(QWidget* pParent) :
  CTimelineSeqeunceInstructionWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CTimelineSeqeunceInstructionWidgetEval>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pScriptEditor, &CScriptEditorWidget::textChanged,
          this, &CTimelineSeqeunceInstructionWidgetEval::EmitPropertiesChanged);
}
CTimelineSeqeunceInstructionWidgetEval::~CTimelineSeqeunceInstructionWidgetEval() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineSeqeunceInstructionWidgetEval::SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  m_bIsInitializing = true;

  m_sType = spInstr->m_sInstructionType;

  auto spInstrCasted = std::dynamic_pointer_cast<SEvalInstruction>(spInstr);
  m_spUi->pScriptEditor->setPlainText(spInstrCasted->m_sScript);

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineSeqeunceInstructionWidgetEval::Properties() const
{
  QVariantList args;
  args << m_spUi->pScriptEditor->toPlainText();
  return sequence::CreateInstruction(m_sType, args);
}
