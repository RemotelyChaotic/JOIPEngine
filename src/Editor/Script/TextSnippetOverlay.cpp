#include "TextSnippetOverlay.h"
#include "Application.h"
#include "ScriptEditorWidget.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Widgets/ColorPicker.h"
#include "ui_TextSnippetOverlay.h"

#include <QLineEdit>
#include <QScrollBar>

namespace  {
  const char* c_sIndexProperty = "Index";
  const qint32 c_iAlignLeftIndex = 0;
  const qint32 c_iAlignRightIndex = 1;
  const qint32 c_iAlignHCenterIndex = 2;
}

//----------------------------------------------------------------------------------------
//
CTextSnippetOverlay::CTextSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(new Ui::CTextSnippetOverlay),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_data()
{
  m_spUi->setupUi(this);
  connect(m_spUi->pButtonsList->itemDelegate(), &QAbstractItemDelegate::commitData,
          this, &CTextSnippetOverlay::SlotItemListCommitData);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  m_spUi->pScrollAreaText->setWidgetResizable(true);
  m_spUi->pScrollAreaIcon->setWidgetResizable(true);
  m_spUi->pScrollAreaButtons->setWidgetResizable(true);
  m_spUi->pScrollAreaTextColor->setWidgetResizable(true);
  m_spUi->pScrollAreaBG->setWidgetResizable(true);
  m_preferredSize = size();
  m_vScrollAreas = {
    m_spUi->pScrollAreaText,
    m_spUi->pScrollAreaIcon,
    m_spUi->pScrollAreaButtons,
    m_spUi->pScrollAreaTextColor,
    m_spUi->pScrollAreaBG
  };
}

CTextSnippetOverlay::~CTextSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  SetInitialized(false);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CTextSnippetOverlay::SlotCurrentChanged);

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
void CTextSnippetOverlay::Resize()
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
      QPoint(newSize.width() / 2, newSize.height() / 2);

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
void CTextSnippetOverlay::Show()
{
  COverlayBase::Show();
  Initialize();
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pShowTextCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShowText = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pShowUserInputCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShowUserInput = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSetTextOrientation_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetAlignment = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pOrientationComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!m_bInitialized) { return; }
  switch (iIndex)
  {
  case c_iAlignLeftIndex: m_data.m_textAlignment = Qt::AlignLeft; break;
  case c_iAlignRightIndex: m_data.m_textAlignment = Qt::AlignRight; break;
  case c_iAlignHCenterIndex: m_data.m_textAlignment = Qt::AlignHCenter; break;
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSetSleepTimeCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetSleepTime = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pAutoTimeCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bAutoTime = bStatus;
  m_spUi->pSleepSpinBox->setEnabled(!bStatus);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSleepSpinBox_valueChanged(double dValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_dSleepTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSkippableCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSkippable = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  m_data.m_sText = m_spUi->pTextEdit->toPlainText();
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pShowIconCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShowIcon = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sTextIcon = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CTextSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
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
void CTextSnippetOverlay::on_pShowButtonsCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShowButtons = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_AddButtonButton_clicked()
{
  if (!m_bInitialized) { return; }
  const QString sButtonText = "New Button";
  m_data.m_vsButtons.push_back(sButtonText);
  m_spUi->pButtonsList->addItem(sButtonText);
  auto pItem = m_spUi->pButtonsList->item(static_cast<qint32>(m_data.m_vsButtons.size()) - 1);
  pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_RemoveButtonButton_clicked()
{
  if (!m_bInitialized) { return; }
  QModelIndex index = m_spUi->pButtonsList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vsButtons.size() > static_cast<size_t>(index.row()))
  {
    m_data.m_vsButtons.erase(m_data.m_vsButtons.begin() + index.row());
    auto pItem = m_spUi->pButtonsList->takeItem(index.row());
    if (nullptr != pItem)
    {
      delete pItem;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::SlotItemListCommitData(QWidget* pLineEdit)
{
  if (!m_bInitialized) { return; }

  QLineEdit* pCastLineEdit = qobject_cast<QLineEdit*>(pLineEdit);
  QModelIndex index = m_spUi->pButtonsList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vsButtons.size() > static_cast<size_t>(index.row()) &&
      nullptr != pCastLineEdit)
  {
    m_data.m_vsButtons[static_cast<size_t>(index.row())] = pCastLineEdit->text();
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSetTextColorsCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetTextColors = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_AddTextColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  CColorPicker* pItem = new CColorPicker(m_spUi->pButtonsList);
  pItem->SetAlphaVisible(false);
  connect(pItem, &CColorPicker::SignalColorChanged,
          this, &CTextSnippetOverlay::SlotTextColorChanged);

  qint32 iIndex = 0;
  if (m_data.m_vTextColors.size() == 0)
  {
    iIndex = 0;
  }
  else
  {
    iIndex = std::next(m_data.m_vTextColors.end(), -1)->first + 1;
  }
  m_data.m_vTextColors.insert({iIndex, QColor(0, 0, 0, 255)});

  m_spUi->pTextColorsList->insertRow(iIndex);
  m_spUi->pTextColorsList->setCellWidget(iIndex, 0, pItem);
  pItem->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_RemoveTextColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  QModelIndex index = m_spUi->pTextColorsList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vTextColors.size() > static_cast<size_t>(index.row()))
  {
    CColorPicker* pItem = dynamic_cast<CColorPicker*>(
      m_spUi->pTextColorsList->cellWidget(index.row(), index.column()));

    if (nullptr != pItem)
    {
      qint32 iIndex = pItem->property(c_sIndexProperty).toInt();
      auto it = m_data.m_vTextColors.find(iIndex);
      if (m_data.m_vTextColors.end() != it)
      {
        m_data.m_vTextColors.erase(it);
        m_spUi->pTextColorsList->removeRow(index.row());
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::SlotTextColorChanged(const QColor& color)
{
  qint32 iIndex = sender()->property(c_sIndexProperty).toInt();
  if (0 <= iIndex && m_data.m_vTextColors.size() > static_cast<size_t>(iIndex))
  {
    m_data.m_vTextColors[static_cast<size_t>(iIndex)] = color;
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pSetBGCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetBGColors = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_AddBGColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  CColorPicker* pItem = new CColorPicker(m_spUi->pBGList);
  pItem->SetAlphaVisible(false);
  connect(pItem, &CColorPicker::SignalColorChanged,
          this, &CTextSnippetOverlay::SlotBGColorChanged);

  qint32 iIndex = 0;
  if (m_data.m_vBGColors.size() == 0)
  {
    iIndex = 0;
  }
  else
  {
    iIndex = std::next(m_data.m_vBGColors.end(), -1)->first + 1;
  }
  m_data.m_vBGColors.insert({iIndex, QColor(0, 0, 0, 255)});

  m_spUi->pBGList->insertRow(iIndex);
  m_spUi->pBGList->setCellWidget(iIndex, 0, pItem);
  pItem->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_RemoveBGColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  QModelIndex index = m_spUi->pBGList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vBGColors.size() > static_cast<size_t>(index.row()))
  {
    CColorPicker* pItem = dynamic_cast<CColorPicker*>(
      m_spUi->pBGList->cellWidget(index.row(), index.column()));

    if (nullptr != pItem)
    {
      qint32 iIndex = pItem->property(c_sIndexProperty).toInt();
      auto it = m_data.m_vBGColors.find(iIndex);
      if (m_data.m_vBGColors.end() != it)
      {
        m_data.m_vBGColors.erase(it);
        m_spUi->pBGList->removeRow(index.row());
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::SlotBGColorChanged(const QColor& color)
{
  qint32 iIndex = sender()->property(c_sIndexProperty).toInt();
  if (0 <= iIndex && m_data.m_vBGColors.size() > static_cast<size_t>(iIndex))
  {
    m_data.m_vBGColors[static_cast<size_t>(iIndex)] = color;
  }
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pConfirmButton_clicked()
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
void CTextSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::Initialize()
{
  m_bInitialized = false;
  m_data = STextSnippetCode();
  m_spUi->pShowTextCheckBox->setChecked(false);
  m_spUi->pShowUserInputCheckBox->setChecked(false);
  m_spUi->pSetTextOrientation->setChecked(false);
  m_spUi->pOrientationComboBox->setCurrentIndex(c_iAlignHCenterIndex);
  m_spUi->pSetSleepTimeCheckBox->setChecked(false);
  m_spUi->pAutoTimeCheckBox->setChecked(true);
  m_spUi->pSleepSpinBox->setValue(0.0);
  m_spUi->pSleepSpinBox->setEnabled(false);
  m_spUi->pSkippableCheckBox->setChecked(false);
  m_spUi->pTextEdit->clear();
  m_spUi->pShowIconCheckBox->setChecked(false);
  m_spUi->pResourceLineEdit->clear();
  m_spUi->pShowButtonsCheckBox->setChecked(false);
  m_spUi->pButtonsList->clear();
  m_spUi->pSetTextColorsCheckBox->setChecked(false);
  m_spUi->pTextColorsList->clear();
  while (m_spUi->pTextColorsList->rowCount() > 0)
  {
    m_spUi->pTextColorsList->removeRow(0);
  }
  m_spUi->pSetBGCheckBox->setChecked(false);
  m_spUi->pBGList->clear();
  while (m_spUi->pBGList->rowCount() > 0)
  {
    m_spUi->pBGList->removeRow(0);
  }
  m_bInitialized = true;
}

