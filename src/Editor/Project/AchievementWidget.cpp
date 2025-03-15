#include "AchievementWidget.h"

#include "Editor/Project/SelectableResourceLabel.h"

#include "Systems/Project.h"

#include "Utils/UndoRedoFilter.h"

#include "Widgets/ShortcutButton.h"

#include <QGridLayout>
#include <QSignalBlocker>

namespace
{
  const qint32 c_iAchievementTypeBool = ESaveDataType::eBool;
  const qint32 c_iAchievementTypeInt = ESaveDataType::eInt;

  const char c_sNameFileIcon[] = "FileIcon";
}

const char CAchievementWidget::c_sAchievementItemRootWidgetProperty[] = "AchievementItemRoot";

//----------------------------------------------------------------------------------------
//
CAchievementWidget::CAchievementWidget(const tspProject& spProj,
                                       QPointer<CResourceTreeItemModel> pResourceModel,
                                       QWidget *parent) :
  QFrame{parent},
  m_spCurrentProject(spProj),
  m_pResourceModel(pResourceModel)
{
  setFrameStyle(QFrame::Raised);
  setLineWidth(1);
  setFrameShape(QFrame::StyledPanel);
  setFixedHeight(120);

  QGridLayout* pBoxLayout = new QGridLayout(this);
  pBoxLayout->setMargin(0);
  pBoxLayout->setContentsMargins({0,0,0,0});

  m_pButtonIcon = new CSelectableResourceLabel(this);
  m_pButtonIcon->setObjectName(c_sNameFileIcon);
  m_pButtonIcon->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_pButtonIcon->setToolTip(tr("Add resource icon"));
  m_pButtonIcon->SetCurrentProject(spProj);
  m_pButtonIcon->SetResourceModel(pResourceModel);
  connect(m_pButtonIcon, &CSelectableResourceLabel::SignalResourcePicked, this,
          [this](const QString& /*sOld*/, const QString& sNew){
            m_pButtonIcon->SetCurrentResource(sNew);
            EmitParamsChanged();
          });
  pBoxLayout->addWidget(m_pButtonIcon, 0, 0, 2, 1);

  QWidget* pHeader = new QWidget(this);
  pHeader->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  QHBoxLayout* pHeaderLayout = new QHBoxLayout(pHeader);
  pHeaderLayout->setMargin(0);
  pHeaderLayout->setContentsMargins({0,0,0,0});
  m_pLineEditHeader = new QLineEdit(pHeader);
  m_pLineEditHeader->setPlaceholderText(tr("Achievement name / id"));
  m_pLineEditHeader->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  connect(m_pLineEditHeader, &QLineEdit::textChanged, this,
          [this]() {
            EmitParamsChanged();
          });
  new CUndoRedoFilter(m_pLineEditHeader, nullptr);
  m_pRemoveButton = new CShortcutButton(pHeader);
  m_pRemoveButton->setObjectName("CloseButton");
  m_pRemoveButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_pRemoveButton->setToolTip(tr("Remove achievement"));
  connect(m_pRemoveButton, &QPushButton::clicked, this, &CAchievementWidget::SignalRemove);
  pHeaderLayout->addWidget(m_pLineEditHeader);
  pHeaderLayout->addWidget(m_pRemoveButton);
  pBoxLayout->addWidget(pHeader, 0, 1, 1, 1);

  QWidget* pBody = new QWidget(this);
  pBody->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
  QGridLayout* pBodyLayout = new QGridLayout(pBody);
  pBodyLayout->setMargin(0);
  pBodyLayout->setContentsMargins({0,0,0,0});
  m_pComboType = new QComboBox(pBody);
  m_pComboType->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_pComboType->addItem(tr("Type: Locked / Unlock"), c_iAchievementTypeBool);
  m_pComboType->addItem(tr("Type: Counting"), c_iAchievementTypeInt);
  m_pComboType->setCurrentIndex(1);
  pBodyLayout->addWidget(m_pComboType, 0, 0, 1, 1);
  m_pSpinCounter = new QSpinBox(pBody);
  m_pSpinCounter->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_pSpinCounter->setRange(0, INT_MAX);
  connect(m_pSpinCounter, qOverload<qint32>(&QSpinBox::valueChanged), this,
          [this](qint32 /*iVal*/){
            EmitParamsChanged();
          });
  pBodyLayout->addWidget(m_pSpinCounter, 0, 1, 1, 1);
  connect(m_pComboType, qOverload<qint32>(&QComboBox::currentIndexChanged), m_pComboType,
          [this](qint32 iIdx) {
            m_pSpinCounter->setVisible(m_pComboType->itemData(c_iAchievementTypeInt).toInt() == iIdx);
            EmitParamsChanged();
          });
  m_pTextEdit = new QTextEdit(pBody);
  m_pTextEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
  m_pTextEdit->setPlaceholderText(tr("No description set"));
  new CUndoRedoFilter(m_pTextEdit, nullptr);
  connect(m_pTextEdit, &QTextEdit::textChanged, this, &CAchievementWidget::EmitParamsChanged);
  pBodyLayout->addWidget(m_pTextEdit, 1, 0, 1, 2);
  pBoxLayout->addWidget(pBody, 1, 1, 1, 1);
}
CAchievementWidget::~CAchievementWidget() = default;

//----------------------------------------------------------------------------------------
//
void CAchievementWidget::SetAchievementData(const SSaveDataData& saveData)
{
  m_saveData = saveData;
  QSignalBlocker b1(m_pLineEditHeader);
  m_pLineEditHeader->setText(m_saveData.m_sName);
  QSignalBlocker b2(m_pComboType);
  qint32 iIdx = m_pComboType->findData(saveData.m_type._to_integral());
  m_pComboType->setCurrentIndex(iIdx);
  QSignalBlocker b3(m_pSpinCounter);
  m_pSpinCounter->setVisible(m_pComboType->itemData(c_iAchievementTypeInt).toInt() == iIdx);
  switch (saveData.m_type)
  {
    case ESaveDataType::eInt:
      m_pSpinCounter->setValue(saveData.m_data.toInt());
      break;
    default: break;
  }
  QSignalBlocker b4(m_pTextEdit);
  const qint32 iCurrentCursorPosition = m_pTextEdit->textCursor().position();
  m_pTextEdit->setPlainText(saveData.m_sDescribtion);
  QTextCursor cursor = m_pTextEdit->textCursor();
  cursor.setPosition(iCurrentCursorPosition);
  m_pTextEdit->setTextCursor(cursor);
  QSignalBlocker b5(m_pButtonIcon);
  m_pButtonIcon->SetCurrentResource(saveData.m_sResource);
}

//----------------------------------------------------------------------------------------
//
const SSaveDataData& CAchievementWidget::AchievementData() const
{
  return m_saveData;
}

//----------------------------------------------------------------------------------------
//
void CAchievementWidget::EmitParamsChanged()
{
  SSaveDataData oldData = m_saveData;
  m_saveData.m_sName = m_pLineEditHeader->text();
  m_saveData.m_sDescribtion = m_pTextEdit->toPlainText();
  m_saveData.m_sResource = m_pButtonIcon->CurrentResource();
  m_saveData.m_type = ESaveDataType::_from_integral(m_pComboType->currentData().toInt());
  switch (m_saveData.m_type)
  {
    case ESaveDataType::eBool:
      m_saveData.m_data = false;
      break;
    case ESaveDataType::eInt:
      m_saveData.m_data = m_pSpinCounter->value();
      break;
    default: break;
  }
  emit SignalAchievementChanged(oldData, m_saveData);
}
