#include "NotificationSnippetOverlay.h"
#include "Application.h"
#include "ui_NotificationSnippetOverlay.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Widgets/ColorPicker.h"

namespace  {
  const qint32 c_iAlignLeftIndex = 0;
  const qint32 c_iAlignRightIndex = 1;
  const qint32 c_iAlignHCenterIndex = 2;
}

//----------------------------------------------------------------------------------------
//
CNotificationSnippetOverlay::CNotificationSnippetOverlay(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CNotificationSnippetOverlay>())
{
  m_spUi->setupUi(this);
}

CNotificationSnippetOverlay::~CNotificationSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pPortraitResourceSelectTree->model())
      ->setSourceModel(nullptr);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pOnClickResourceSelectTree->model())
      ->setSourceModel(nullptr);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pOnTimeoutResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  m_bInitialized = false;

  CResourceTreeItemSortFilterProxyModel* pPortraitProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pPortraitResourceSelectTree);
  pPortraitProxyModel->setSourceModel(pResourceTreeModel);
  m_spUi->pPortraitResourceSelectTree->setModel(pPortraitProxyModel);
  CResourceTreeItemSortFilterProxyModel* pOnClickProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pOnClickResourceSelectTree);
  pOnClickProxyModel->setSourceModel(pResourceTreeModel);
  m_spUi->pOnClickResourceSelectTree->setModel(pOnClickProxyModel);
  CResourceTreeItemSortFilterProxyModel* pOnTimeoutProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pOnTimeoutResourceSelectTree);
  pOnTimeoutProxyModel->setSourceModel(pResourceTreeModel);
  m_spUi->pOnTimeoutResourceSelectTree->setModel(pOnTimeoutProxyModel);

  QItemSelectionModel* pIconSelectionModel = m_spUi->pPortraitResourceSelectTree->selectionModel();
  connect(pIconSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CNotificationSnippetOverlay::SlotIconCurrentChanged);
  QItemSelectionModel* pOnClickSelectionModel = m_spUi->pOnClickResourceSelectTree->selectionModel();
  connect(pOnClickSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CNotificationSnippetOverlay::SlotOnClickCurrentChanged);
  QItemSelectionModel* pOnTimeoutSelectionModel = m_spUi->pOnTimeoutResourceSelectTree->selectionModel();
  connect(pOnTimeoutSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CNotificationSnippetOverlay::SlotOnTimeoutCurrentChanged);

  pPortraitProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pPortraitProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  pOnClickProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pOnClickProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  pOnTimeoutProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pOnTimeoutProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pPortraitResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pPortraitResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pPortraitResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pOnClickResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pOnClickResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pOnClickResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  m_spUi->pOnTimeoutResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pOnTimeoutResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pOnTimeoutResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::UnloadProject()
{
  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::Show()
{
  COverlayBase::Show();
  Initialize();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pSetTextOrientation_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetAlignment = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOrientationComboBox_currentIndexChanged(qint32 iIndex)
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
void CNotificationSnippetOverlay::on_pSetTimeoutTimeCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetTimeoutTime = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pTimeSpinBox_valueChanged(double dValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_dTimeoutTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pIdLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sId = m_spUi->pIdLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pShowRadioButton_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  if (bStatus)
  {
    m_data.m_displayStatus = EDisplayStatus::eShow;
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pHideRadioButton_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  if (bStatus)
  {
    m_data.m_displayStatus = EDisplayStatus::eHide;
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pClearRadioButton_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  if (bStatus)
  {
    m_data.m_displayStatus = EDisplayStatus::eClear;
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  m_data.m_sText = m_spUi->pTextEdit->toPlainText();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pWidgetTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  m_data.m_sWidgetText = m_spUi->pWidgetTextEdit->toPlainText();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pShowIconCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShowIcon = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sIcon = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pPortraitResourceSelectTree->model());

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
void CNotificationSnippetOverlay::SlotIconCurrentChanged(const QModelIndex& current,
                                                         const QModelIndex& previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pPortraitResourceSelectTree->model());
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
void CNotificationSnippetOverlay::on_pOnClickCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bOnButton = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOnClickResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sOnButton = m_spUi->pOnClickResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOnClickFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pOnClickResourceSelectTree->model());

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
void CNotificationSnippetOverlay::SlotOnClickCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pOnClickResourceSelectTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    m_spUi->pOnClickResourceLineEdit->setText(sName);
    on_pOnClickResourceLineEdit_editingFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOnTimeoutCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bOnTimeout = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOnTimeoutResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sOnTimeout = m_spUi->pOnTimeoutResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pOnTimeoutFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pOnTimeoutResourceSelectTree->model());

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
void CNotificationSnippetOverlay::SlotOnTimeoutCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pOnTimeoutResourceSelectTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    m_spUi->pOnTimeoutResourceLineEdit->setText(sName);
    on_pOnTimeoutResourceLineEdit_editingFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pSetTextColorCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetTextColor = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pSetTextBackgroundColorCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetTextBackgroundColor = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pSetWidgetColorCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetWidgetTextColors = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pSetWidgetBackgrounColorCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSetTextBackgroundColor = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pTextColor_SignalColorChanged(const QColor& color)
{
  if (!m_bInitialized) { return; }
  m_data.m_textColor = color;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pTextBackgroundColor_SignalColorChanged(const QColor& color)
{
  if (!m_bInitialized) { return; }
  m_data.m_textBackgroundColor = color;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pWidgetTextColor_SignalColorChanged(const QColor& color)
{
  if (!m_bInitialized) { return; }
  m_data.m_widgetTextColor = color;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pWidgetBackgroundColor_SignalColorChanged(const QColor& color)
{
  if (!m_bInitialized) { return; }
  m_data.m_widgettextBackgroundColor = color;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pConfirmButton_clicked()
{
  QString sCode;
  if (m_data.m_bSetTextColor)
  {
    QString sText("notification.setTextColor([%1]);\n");
      QString sColor = "[" +
          QString::number(m_data.m_textColor.red()) + "," +
          QString::number(m_data.m_textColor.green()) + "," +
          QString::number(m_data.m_textColor.blue()) + "," +
          QString::number(m_data.m_textColor.alpha()) + "]";
    sCode += sText.arg(sColor);
  }
  if (m_data.m_bSetTextBackgroundColor)
  {
    QString sText("notification.setTextBackgroundColor([%1]);\n");
      QString sColor = "[" +
          QString::number(m_data.m_textBackgroundColor.red()) + "," +
          QString::number(m_data.m_textBackgroundColor.green()) + "," +
          QString::number(m_data.m_textBackgroundColor.blue()) + "," +
          QString::number(m_data.m_textBackgroundColor.alpha()) + "]";
    sCode += sText.arg(sColor);
  }
  if (m_data.m_bSetWidgetTextColors)
  {
    QString sText("notification.setWidgetColor([%1]);\n");
      QString sColor = "[" +
          QString::number(m_data.m_widgetTextColor.red()) + "," +
          QString::number(m_data.m_widgetTextColor.green()) + "," +
          QString::number(m_data.m_widgetTextColor.blue()) + "," +
          QString::number(m_data.m_widgetTextColor.alpha()) + "]";
    sCode += sText.arg(sColor);
  }
  if (m_data.m_bSetWidgetTextBackgroundColor)
  {
    QString sText("notification.setWidgetBackgroundColor([%1]);\n");
      QString sColor = "[" +
          QString::number(m_data.m_widgettextBackgroundColor.red()) + "," +
          QString::number(m_data.m_widgettextBackgroundColor.green()) + "," +
          QString::number(m_data.m_widgettextBackgroundColor.blue()) + "," +
          QString::number(m_data.m_widgettextBackgroundColor.alpha()) + "]";
    sCode += sText.arg(sColor);
  }
  if (m_data.m_bSetAlignment)
  {
    QString sText("notification.setIconAlignment(IconAlignment.%1);\n");
    switch (m_data.m_textAlignment)
    {
    case Qt::AlignLeft: sText = sText.arg("AlignLeft"); break;
    case Qt::AlignRight: sText = sText.arg("AlignRight"); break;
    case Qt::AlignHCenter: sText = sText.arg("AlignCenter"); break;
    default: break;
    }
    sCode += sText;
  }
  if (m_data.m_bShowIcon)
  {
    EResourceType type = EResourceType::eImage;
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager && nullptr != m_spCurrentProject)
    {
      tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_data.m_sIcon);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        type = spResource->m_type;
      }
    }

    QString sText("notification.setPortrait(%2);\n");
    switch (type)
    {
      case EResourceType::eMovie: // fallthrough
      case EResourceType::eImage:
        sText = sText.arg(m_data.m_sIcon.isEmpty() ? "null" :
                                                     ("\"" + m_data.m_sIcon + "\""));
        break;
      case EResourceType::eSound: // fallthrough
    default: sText = ""; break;
    }

    sCode += sText;
  }
  QString sText;
  switch(m_data.m_displayStatus)
  {
    case EDisplayStatus::eShow:
    {
      QString sOptional;
      if (m_data.m_bSetTimeoutTime)
      {
        sOptional += QString(",%1").arg(m_data.m_dTimeoutTimeS);
      }
      if (m_data.m_bOnButton)
      {
        sOptional += QString(",\"%1\"").arg(m_data.m_sOnButton);
      }
      if (m_data.m_bOnTimeout)
      {
        if (!m_data.m_bSetTimeoutTime) { sOptional += ",-1"; }
        if (!m_data.m_bOnButton) { sOptional += ",\"\""; }
        sOptional += QString(",\"%1\"").arg(m_data.m_sOnTimeout);
      }
      sText = QString("notification.show(\"%1\",\"%2\",\"%3\"%4);\n")
          .arg(m_data.m_sId).arg(m_data.m_sText).arg(m_data.m_sWidgetText).arg(sOptional);
    } break;
    case EDisplayStatus::eHide:
      sText = QString("notification.hide(\"%1\");\n").arg(m_data.m_sId);
      break;
    case EDisplayStatus::eClear:
      sText = QString("notification.clear();\n");
      break;
    default: break;
  }
  sCode += sText;

  emit SignalNotificationSnippetCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSnippetOverlay::Initialize()
{
  m_bInitialized = false;
  m_data = SNotificationSnippetCode();
  m_spUi->pSetTextOrientation->setChecked(false);
  m_spUi->pOrientationComboBox->setCurrentIndex(c_iAlignHCenterIndex);
  m_spUi->pSetTimeoutTimeCheckBox->setChecked(true);
  m_spUi->pTimeSpinBox->setValue(0.0);
  m_spUi->pIdLineEdit->setText(QString());
  m_spUi->pShowRadioButton->setChecked(true);
  m_spUi->pTextEdit->clear();
  m_spUi->pWidgetTextEdit->clear();
  m_spUi->pShowIconCheckBox->setChecked(false);
  m_spUi->pResourceLineEdit->clear();
  m_spUi->pOnClickCheckBox->setChecked(false);
  m_spUi->pOnClickResourceLineEdit->clear();
  m_spUi->pOnTimeoutCheckBox->setChecked(false);
  m_spUi->pOnTimeoutResourceLineEdit->clear();
  m_spUi->pSetTextColorCheckBox->setChecked(false);
  m_spUi->pTextColor->SetColor(QColor());
  m_spUi->pSetTextBackgroundColorCheckBox->setChecked(false);
  m_spUi->pTextBackgroundColor->SetColor(QColor());
  m_spUi->pSetWidgetColorCheckBox->setChecked(false);
  m_spUi->pWidgetTextColor->SetColor(QColor());
  m_spUi->pSetWidgetBackgrounColorCheckBox->setChecked(false);
  m_spUi->pWidgetBackgroundColor->SetColor(QColor());
  m_bInitialized = true;
}
