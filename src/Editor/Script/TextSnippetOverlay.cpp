#include "TextSnippetOverlay.h"
#include "Widgets/ColorPicker.h"
#include "ui_TextSnippetOverlay.h"

#include <QLineEdit>

namespace  {
  const char* c_sIndexProperty = "Index";
}

//----------------------------------------------------------------------------------------
//
CTextSnippetOverlay::CTextSnippetOverlay(QWidget* pParent) :
  COverlayBase(pParent),
  m_spUi(new Ui::CTextSnippetOverlay),
  m_bInitialized(true),
  m_data()
{
  m_spUi->setupUi(this);
  connect(m_spUi->pButtonsList->itemDelegate(), &QAbstractItemDelegate::commitData,
          this, &CTextSnippetOverlay::SlotItemListCommitData);
}

CTextSnippetOverlay::~CTextSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTarget->geometry().width() / 2, m_pTarget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
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
void CTextSnippetOverlay::on_pTextEdit_textChanged()
{
  if (!m_bInitialized) { return; }
  m_data.m_sText = m_spUi->pTextEdit->toPlainText();
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
void CTextSnippetOverlay::on_pAddButtonButton_clicked()
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
void CTextSnippetOverlay::on_pRemoveButtonButton_clicked()
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
void CTextSnippetOverlay::on_pAddTextColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  CColorPicker* pItem = new CColorPicker(m_spUi->pButtonsList);
  pItem->SetAlphaVisible(false);
  connect(pItem, &CColorPicker::SignalColorChanged,
          this, &CTextSnippetOverlay::SlotTextColorChanged);

  m_data.m_vTextColors.push_back(QColor(0, 0, 0, 255));

  qint32 iIndex = m_spUi->pTextColorsList->rowCount();
  m_spUi->pTextColorsList->insertRow(iIndex);
  m_spUi->pTextColorsList->setCellWidget(iIndex, 0, pItem);
  pItem->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pRemoveTextColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  QModelIndex index = m_spUi->pTextColorsList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vTextColors.size() > static_cast<size_t>(index.row()))
  {
    m_data.m_vTextColors.erase(m_data.m_vTextColors.begin() + index.row());
    m_spUi->pTextColorsList->removeRow(index.row());
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
void CTextSnippetOverlay::on_pAddBGColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  CColorPicker* pItem = new CColorPicker(m_spUi->pBGList);
  pItem->SetAlphaVisible(false);
  connect(pItem, &CColorPicker::SignalColorChanged,
          this, &CTextSnippetOverlay::SlotBGColorChanged);

  m_data.m_vBGColors.push_back(QColor(0, 0, 0, 255));

  qint32 iIndex = m_spUi->pBGList->rowCount();
  m_spUi->pBGList->insertRow(iIndex);
  m_spUi->pBGList->setCellWidget(iIndex, 0, pItem);
  pItem->setProperty(c_sIndexProperty, iIndex);
}

//----------------------------------------------------------------------------------------
//
void CTextSnippetOverlay::on_pRemoveBGColorButton_clicked()
{
  if (!m_bInitialized) { return; }

  QModelIndex index = m_spUi->pBGList->currentIndex();
  if (index.row() >= 0 &&
      m_data.m_vBGColors.size() > static_cast<size_t>(index.row()))
  {
    m_data.m_vBGColors.erase(m_data.m_vBGColors.begin() + index.row());
    m_spUi->pBGList->removeRow(index.row());
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
  QString sCode;
  if (m_data.m_bSetTextColors)
  {
    QString sText("textBox.setTextColors([%1]);\n");
    QStringList vsColors;
    for (auto color : m_data.m_vTextColors)
    {
      QString sColor = "[" + QString::number(color.red()) + "," +
          QString::number(color.green()) + "," +
          QString::number(color.blue()) + "]";
      vsColors << sColor;
    }
    sCode += sText.arg(vsColors.join(","));
  }
  if (m_data.m_bSetBGColors)
  {
    QString sText("textBox.setBackgroundColors([%1]);\n");
    QStringList vsColors;
    for (auto color : m_data.m_vBGColors)
    {
      QString sColor = "[" + QString::number(color.red()) + "," +
          QString::number(color.green()) + "," +
          QString::number(color.blue()) + "]";
      vsColors << sColor;
    }
    sCode += sText.arg(vsColors.join(","));
  }
  if (m_data.m_bShowText)
  {
    QString sText("textBox.showText(\"%1\");\n");
    sCode += sText.arg(m_data.m_sText);
  }
  if (m_data.m_bShowUserInput)
  {
    QString sText("var sInput = textBox.showInput(); // TODO: change variable name\n");
    sCode += sText;
  }
  if (m_data.m_bShowButtons)
  {
    QString sText("var iSelection = textBox.showButtonPrompts([%1]);  // TODO: change variable name\n");
    QStringList vsPrompts;
    for (auto sButton : m_data.m_vsButtons)
    {
      vsPrompts << "\"" + sButton + "\"";
    }
    sCode += sText.arg(vsPrompts.join(","));
  }

  emit SignalTextSnippetCode(sCode);
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
  m_spUi->pTextEdit->clear();
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

