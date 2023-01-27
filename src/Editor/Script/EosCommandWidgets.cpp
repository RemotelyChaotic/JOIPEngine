#include "EosCommandWidgets.h"

#include "ui_EosCommandWidgetAudio.h"
#include "ui_EosCommandWidgetChoice.h"
#include "ui_EosCommandWidgetEval.h"
#include "ui_EosCommandWidgetGoto.h"
#include "ui_EosCommandWidgetIf.h"
#include "ui_EosCommandWidgetImage.h"
#include "ui_EosCommandWidgetNotificationClose.h"
#include "ui_EosCommandWidgetNotificationCreate.h"
#include "ui_EosCommandWidgetPrompt.h"
#include "ui_EosCommandWidgetSay.h"
#include "ui_EosCommandWidgetScene.h"
#include "ui_EosCommandWidgetTimer.h"

#include "Editor/Resources/ResourceToolTip.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"
#include "Widgets/ColorPicker.h"

#include <QCompleter>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QToolTip>

namespace
{
  template<typename T>
  std::function<CEosCommandWidgetBase*(QWidget* /*pParent*/)> Creator()
  {
    return [](QWidget* pParent) { return new T(pParent); };
  }
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetBase* eos::CreateWidgetFromType(const QString& sType, QWidget* pParent)
{
  static const std::map<QString, std::function<CEosCommandWidgetBase*(QWidget* /*pParent*/)>> m_creatorMap =
  {
    {eos::c_sCommandAudioPlay, Creator<CEosCommandWidgetAudio>()},
    {eos::c_sCommandChoice, Creator<CEosCommandWidgetChoice>()},
    {eos::c_sCommandDisableScreen, Creator<CEosCommandWidgetScene>()},
    {eos::c_sCommandDisableScreen, Creator<CEosCommandWidgetScene>()},
    {eos::c_sCommandEnd, Creator<CEosCommandWidgetEnd>()},
    {eos::c_sCommandEval, Creator<CEosCommandWidgetEval>()},
    {eos::c_sCommandGoto, Creator<CEosCommandWidgetGoto>()},
    {eos::c_sCommandIf, Creator<CEosCommandWidgetIf>()},
    {eos::c_sCommandImage, Creator<CEosCommandWidgetImage>()},
    {eos::c_sCommandNotificationClose, Creator<CEosCommandWidgetNotificationClose>()},
    {eos::c_sCommandNotificationCreate, Creator<CEosCommandWidgetNotificationCreate>()},
    {eos::c_sCommandPrompt, Creator<CEosCommandWidgetPrompt>()},
    {eos::c_sCommandSay, Creator<CEosCommandWidgetSay>()},
    {eos::c_sCommandTimer, Creator<CEosCommandWidgetTimer>()},
  };
  auto it = m_creatorMap.find(sType);
  if (m_creatorMap.end() != it)
  {
    return it->second(pParent);
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetBase::CEosCommandWidgetBase(QWidget* pParent) :
  QWidget(pParent),
  m_bIsInitializing(false)
{
  setAttribute(Qt::WA_TranslucentBackground);
}

CEosCommandWidgetBase::~CEosCommandWidgetBase()
{
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetAudio::CEosCommandWidgetAudio(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetAudio>())
{
  m_spUi->setupUi(this);

  m_pValidator = new QRegularExpressionValidator(m_spUi->pLocatorLineEdit);
  m_pValidator->setRegularExpression(QRegularExpression(eos::c_sMatcherGallery));

  connect(m_spUi->pLocatorLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetAudio::on_pLocatorLineEdit_editingFinished);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          this, &CEosCommandWidgetAudio::on_SearchButton_clicked);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CEosCommandWidgetAudio::on_pResourceSelectTree_doubleClicked);

  connect(m_spUi->pIdLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetAudio::on_pIdLineEdit_editingFinished);
  connect(m_spUi->pLoopsSpinBox, &CLongLongSpinBox::valueChanged,
          this, &CEosCommandWidgetAudio::on_pLoopsSpinBox_valueChanged);
  connect(m_spUi->pSeekSpinBox, &CLongLongSpinBox::valueChanged,
          this, &CEosCommandWidgetAudio::on_pSeekSpinBox_valueChanged);
  connect(m_spUi->pStartSpinBox, &CLongLongSpinBox::valueChanged,
          this, &CEosCommandWidgetAudio::on_pStartSpinBox_valueChanged);
  connect(m_spUi->pVolumeSlider, &QSlider::valueChanged,
          this, &CEosCommandWidgetAudio::on_pVolumeSlider_valueChanged);
}

CEosCommandWidgetAudio::~CEosCommandWidgetAudio()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itLocator = GetValue<EArgumentType::eString>(*props, "locator");
  const auto& itId = GetValue<EArgumentType::eString>(*props, "id");
  const auto& itLoops = GetValue<EArgumentType::eInt64>(*props, "loops");
  const auto& itSeek = GetValue<EArgumentType::eInt64>(*props, "seek");
  const auto& itStartAt = GetValue<EArgumentType::eInt64>(*props, "startAt");
  const auto& itVolume = GetValue<EArgumentType::eDouble>(*props, "volume");
  if (HasValue(*props, "locator") && IsOk<EArgumentType::eString>(itLocator))
  {
    QString sResourceLocator = std::get<QString>(itLocator);
    m_spUi->pLocatorLineEdit->setText(sResourceLocator);
  }
  if (HasValue(*props, "id") && IsOk<EArgumentType::eString>(itId))
  {
    QString sId = std::get<QString>(itId);
    m_spUi->pIdLineEdit->setText(sId);
  }
  if (HasValue(*props, "loops") && IsOk<EArgumentType::eInt64>(itLoops))
  {
    qint64 iLoops = std::get<qint64>(itLoops);
    m_spUi->pLoopsSpinBox->setValue(iLoops);
  }
  if (HasValue(*props, "seek") && IsOk<EArgumentType::eInt64>(itSeek))
  {
    qint64 iSeek = std::get<qint64>(itSeek);
    m_spUi->pSeekSpinBox->setValue(iSeek);
  }
  if (HasValue(*props, "startAt") && IsOk<EArgumentType::eInt64>(itStartAt))
  {
    qint64 iStartAt = std::get<qint64>(itStartAt);
    m_spUi->pStartSpinBox->setValue(iStartAt);
  }
  if (HasValue(*props, "volume") && IsOk<EArgumentType::eDouble>(itVolume))
  {
    double dVolume = std::get<double>(itVolume);
    m_spUi->pVolumeSlider->setValue(static_cast<qint32>(dVolume*100));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eSound});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

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
void CEosCommandWidgetAudio::on_pLocatorLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }

  QString sString = m_spUi->pLocatorLineEdit->text();
  qint32 iPos = -1;
  QValidator::State state =
      m_pValidator->validate(sString, iPos);
  if (QValidator::State::Acceptable != state)
  {
    QToolTip::showText(m_spUi->pResourceSelectTree->pos(),
                       tr("Validation of gallery string failed<br>"
                          "Please use the selector or enter a string of format:<br>"
                          "&quot;gallery:&lt;bundle or name&gt;/&lt;optional rest of name&gt;&quot;"));
  }
  else
  {
    SetValue<EArgumentType::eString>(m_pProps, "locator", sString);
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_SearchButton_clicked()
{
  m_spUi->pStackedWidget->SlideInNext();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  auto pProxy =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(
        m_spUi->pResourceSelectTree->model());
  auto pSource = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
  QModelIndex idxModel = pProxy->mapToSource(index);
  if (idxModel.isValid())
  {
    if (pSource->IsResourceType(idxModel))
    {
      tspResource spResource = pSource->ResourceForIndex(idxModel);
      if (nullptr != spResource)
      {
        m_spUi->pStackedWidget->SlideInPrev();
        m_spUi->pLocatorLineEdit->setText(eos::GetEosLocator(spResource));
        on_pLocatorLineEdit_editingFinished();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pIdLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }

  QString sString = m_spUi->pIdLineEdit->text();
  SetValue<EArgumentType::eString>(m_pProps, "id", sString);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pLoopsSpinBox_valueChanged(qint64 iValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eInt64>(m_pProps, "loops", iValue);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pSeekSpinBox_valueChanged(qint64 iValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eInt64>(m_pProps, "seek", iValue);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pStartSpinBox_valueChanged(qint64 iValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eInt64>(m_pProps, "startAt", iValue);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetAudio::on_pVolumeSlider_valueChanged(qint32 iValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eDouble>(m_pProps, "volume",
                                   static_cast<double>(iValue - m_spUi->pVolumeSlider->minimum()) /
                                   (m_spUi->pVolumeSlider->maximum() - m_spUi->pVolumeSlider->minimum()));
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
bool CEosCommandWidgetAudio::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (pObj == m_spUi->pResourceSelectTree && nullptr != pEvt)
  {
    if (QEvent::KeyPress == pEvt->type())
    {
      QKeyEvent* pKeyEvt = static_cast<QKeyEvent*>(pEvt);
      if (pKeyEvt->key() == Qt::Key_Enter || pKeyEvt->key() == Qt::Key_Return)
      {
        auto pProxy =
            dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(
              m_spUi->pResourceSelectTree->model());
        auto pSource = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
        QModelIndex idxProxy = m_spUi->pResourceSelectTree->currentIndex();
        QModelIndex idxModel = pProxy->mapToSource(idxProxy);
        if (idxModel.isValid())
        {
          if (pSource->IsResourceType(idxModel))
          {
            tspResource spResource = pSource->ResourceForIndex(idxModel);
            if (nullptr != spResource)
            {
              m_spUi->pStackedWidget->SlideInPrev();
              m_spUi->pLocatorLineEdit->setText(eos::GetEosLocator(spResource));
              on_pLocatorLineEdit_editingFinished();
              return true;
            }
          }
        }
      }
    }
  }
  if ((pObj == m_spUi->pResourceSelectTree ||
       m_spUi->pResourceSelectTree->viewport() == pObj) && nullptr != pEvt)
  {
    if (QEvent::ToolTip == pEvt->type())
    {
      QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(pEvt);

      QModelIndex index = m_spUi->pResourceSelectTree->indexAt(
            m_spUi->pResourceSelectTree->viewport()->mapFromGlobal(pHelpEvent->globalPos()));
      if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model()))
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

//----------------------------------------------------------------------------------------
//
namespace eosChoiceColumns
{
  const qint32 c_iColumnLabel = 0;
  const qint32 c_iColumnColor = 1;
}

namespace
{
  const char c_sIndexProperty[] = "Index";
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetChoice::CEosCommandWidgetChoice(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetChoice>())
{
  m_spUi->setupUi(this);
  m_spUi->pTableWidget->verticalHeader()->setVisible(false);
  m_spUi->pTableWidget->horizontalHeader()->setSectionResizeMode(eosChoiceColumns::c_iColumnLabel, QHeaderView::Stretch);
  m_spUi->pTableWidget->horizontalHeader()->setSectionResizeMode(eosChoiceColumns::c_iColumnColor, QHeaderView::ResizeToContents);

  connect(m_spUi->AddButtonButton, &QPushButton::clicked,
          this, &CEosCommandWidgetChoice::on_AddButtonButton_clicked);
  connect(m_spUi->RemoveButtonButton, &QPushButton::clicked,
          this, &CEosCommandWidgetChoice::on_RemoveButtonButton_clicked);
  connect(m_spUi->pTableWidget, &QTableWidget::itemChanged,
          this, &CEosCommandWidgetChoice::on_pTableWidget_itemChanged);
}

CEosCommandWidgetChoice::~CEosCommandWidgetChoice()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  while (m_spUi->pTableWidget->rowCount() > 0)
  {
    m_spUi->pTableWidget->removeRow(0);
  }

  const auto& itOptions = GetValue<EArgumentType::eArray>(*props, "options");
  if (HasValue(*props, "options") && IsOk<EArgumentType::eArray>(itOptions))
  {
    const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itOptions);
    for (size_t i = 0; arrOptions.size() > i; ++i)
    {
      const auto& itOption = GetValue<EArgumentType::eMap>(arrOptions, i);
      if (IsOk<EArgumentType::eMap>(itOption))
      {
        const tInstructionMapValue& optionsArg = std::get<tInstructionMapValue>(itOption);
        const auto& itLabel = GetValue<EArgumentType::eString>(optionsArg, "label");
        const auto& itColor = GetValue<EArgumentType::eString>(optionsArg, "color");
        const auto& itVisible = GetValue<EArgumentType::eBool>(optionsArg, "visible");

        QString sLabel;
        if (HasValue(optionsArg, "label") && IsOk<EArgumentType::eString>(itLabel))
        {
          sLabel = std::get<QString>(itLabel);
        }
        QColor col = Qt::black;
        if (HasValue(optionsArg, "color") && IsOk<EArgumentType::eString>(itColor))
        {
          col = std::get<QString>(itColor);
        }
        bool bVisible = true;
        if (HasValue(optionsArg, "visible") && IsOk<EArgumentType::eBool>(itVisible))
        {
          bVisible = std::get<bool>(itVisible);
        }

        AddItemToList(static_cast<qint32>(i), sLabel, col, bVisible);
      }
    }
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::on_AddButtonButton_clicked()
{
  if (m_bIsInitializing) { return; }
  const auto& itOptions = GetValue<EArgumentType::eArray>(*m_pProps, "options");
  if (HasValue(*m_pProps, "options") && IsOk<EArgumentType::eArray>(itOptions))
  {
    tInstructionArrayValue arrOptions = std::get<tInstructionArrayValue>(itOptions);
    arrOptions.push_back(
          { EArgumentType::eMap,
            tInstructionMapValue{{ "label", { EArgumentType::eString, "Button" } },
                                 { "commands", { EArgumentType::eArray,
                                                 tInstructionArrayValue{}} },
                                 { "color", { EArgumentType::eString, "#000000" } },
                                 { "visible", { EArgumentType::eBool, true } }}});

    m_pProps->insert_or_assign("options", SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
    AddItemToList(m_spUi->pTableWidget->rowCount(), QString("Button"), Qt::black, true);
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::on_RemoveButtonButton_clicked()
{
  if (m_bIsInitializing) { return; }

  const auto& itOptions = GetValue<EArgumentType::eArray>(*m_pProps, "options");
  QModelIndex index = m_spUi->pTableWidget->currentIndex();
  if (index.row() >= 0 &&
      HasValue(*m_pProps, "options") && IsOk<EArgumentType::eArray>(itOptions))
  {
    tInstructionArrayValue arrOptions = std::get<tInstructionArrayValue>(itOptions);
    arrOptions.erase(arrOptions.begin() + index.row());
    m_pProps->insert_or_assign("options", SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
    m_spUi->pTableWidget->removeRow(index.row());
    emit SignalChangedProperties();
    emit SignalInvalidateItemChildren();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::on_pTableWidget_itemChanged(QTableWidgetItem* pItem)
{
  if (m_bIsInitializing) { return; }
   qint32 iIndex = pItem->row();
   const auto& itOptions = GetValue<EArgumentType::eArray>(*m_pProps, "options");
   if (HasValue(*m_pProps, "options") && IsOk<EArgumentType::eArray>(itOptions))
   {
     tInstructionArrayValue arrOptions = std::get<tInstructionArrayValue>(itOptions);
     if (arrOptions.size() > iIndex && 0 < iIndex &&
         std::holds_alternative<tInstructionMapValue>(arrOptions[iIndex].m_value))
     {
       tInstructionMapValue values = std::get<tInstructionMapValue>(arrOptions[iIndex].m_value);
       Qt::CheckState state = pItem->data(Qt::CheckStateRole).value<Qt::CheckState>();
       SetValue<EArgumentType::eBool>(&values, "visible", Qt::Checked == state);
       SetValue<EArgumentType::eString>(&values, "label",
                                        pItem->data(Qt::EditRole).toString());
       arrOptions[iIndex].m_value = values;
       m_pProps->insert_or_assign("options", SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
       emit SignalChangedProperties();
     }
   }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::SlotColorChanged(const QColor& color)
{
  if (m_bIsInitializing) { return; }
  CColorPicker* pColorPicker = dynamic_cast<CColorPicker*>(sender());
  if (nullptr != pColorPicker)
  {
    m_bForcedOpen = false;
    qint32 iIndex = pColorPicker->property(c_sIndexProperty).toInt();
    const auto& itOptions = GetValue<EArgumentType::eArray>(*m_pProps, "options");
    if (HasValue(*m_pProps, "options") && IsOk<EArgumentType::eArray>(itOptions))
    {
      tInstructionArrayValue arrOptions = std::get<tInstructionArrayValue>(itOptions);
      if (arrOptions.size() > iIndex && 0 < iIndex &&
          std::holds_alternative<tInstructionMapValue>(arrOptions[iIndex].m_value))
      {
        tInstructionMapValue values = std::get<tInstructionMapValue>(arrOptions[iIndex].m_value);
        SetValue<EArgumentType::eString>(&values, "color",
                                         pColorPicker->Color().name(QColor::HexRgb));
        arrOptions[iIndex].m_value = values;
        m_pProps->insert_or_assign("options", SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
        emit SignalChangedProperties();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetChoice::AddItemToList(qint32 iRow, const QString& sText, const QColor& col, bool bVisible)
{
  m_spUi->pTableWidget->insertRow(iRow);
  QTableWidgetItem* pLabel = new QTableWidgetItem(sText);
  pLabel->setFlags(pLabel->flags() | Qt::ItemIsUserCheckable);
  pLabel->setCheckState(bVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

  CColorPicker* pColorPicker = new CColorPicker(m_spUi->pTableWidget);
  pColorPicker->SetColor(col);
  pColorPicker->SetAlphaVisible(false);
  connect(pColorPicker, &CColorPicker::SignalColorWheelOpening,
          this, [this]{ m_bForcedOpen = true; });
  connect(pColorPicker, &CColorPicker::SignalColorChanged,
          this, &CEosCommandWidgetChoice::SlotColorChanged);
  pColorPicker->setProperty(c_sIndexProperty, iRow);

  m_spUi->pTableWidget->setItem(iRow, eosChoiceColumns::c_iColumnLabel, pLabel);
  m_spUi->pTableWidget->setCellWidget(iRow, eosChoiceColumns::c_iColumnColor, pColorPicker);

  if (!m_bIsInitializing)
  {
    emit SignalInvalidateItemChildren();
  }
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetEnd::CEosCommandWidgetEnd(QWidget* pParent) :
  CEosCommandWidgetBase(pParent)
{
}

CEosCommandWidgetEnd::~CEosCommandWidgetEnd()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetEnd::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetEval::CEosCommandWidgetEval(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetEval>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pPlainTextEdit->document(), &QTextDocument::contentsChange,
          this, &CEosCommandWidgetEval::on_pPlainTextEdit_contentsChange);
}

CEosCommandWidgetEval::~CEosCommandWidgetEval()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetEval::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itScript = GetValue<EArgumentType::eString>(*props, "script");
  if (HasValue(*props, "script") && IsOk<EArgumentType::eString>(itScript))
  {
    QString sResourceLocator = std::get<QString>(itScript);
    m_spUi->pPlainTextEdit->setPlainText(sResourceLocator);
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetEval::on_pPlainTextEdit_contentsChange()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "script",
                                   m_spUi->pPlainTextEdit->toPlainText());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetGoto::CEosCommandWidgetGoto(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetGoto>())
{
  m_spUi->setupUi(this);
  connect(m_spUi->pLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetGoto::on_pLineEdit_editingFinished);

  m_pCompleter = new QCompleter(m_spUi->pLineEdit);
  m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_pCompleterModel = new QStandardItemModel(m_pCompleter);
  m_pCompleter->setModel(m_pCompleterModel);
  m_spUi->pLineEdit->setCompleter(m_pCompleter);
}

CEosCommandWidgetGoto::~CEosCommandWidgetGoto()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetGoto::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_pCompleterModel->removeRows(0, m_pCompleterModel->rowCount());
  tspProject spProject = pResourceModel->Project();
  QReadLocker locker(&spProject->m_rwLock);
  for (const tspScene& spScene : spProject->m_vspScenes)
  {
    QReadLocker locker(&spScene->m_rwLock);
    m_pCompleterModel->appendRow(new QStandardItem(spScene->m_sName));
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetGoto::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itTarget = GetValue<EArgumentType::eString>(*props, "target");
  if (HasValue(*props, "target") && IsOk<EArgumentType::eString>(itTarget))
  {
    QString sTarget = std::get<QString>(itTarget);
    m_spUi->pLineEdit->setText(sTarget);
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetGoto::on_pLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "target", m_spUi->pLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetIf::CEosCommandWidgetIf(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetIf>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pConditionLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetIf::on_pConditionLineEdit_editingFinished);
}

CEosCommandWidgetIf::~CEosCommandWidgetIf()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetIf::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itCondition = GetValue<EArgumentType::eString>(*props, "condition");
  if (HasValue(*props, "condition") && IsOk<EArgumentType::eString>(itCondition))
  {
    QString sCondition = std::get<QString>(itCondition);
    m_spUi->pConditionLineEdit->setText(sCondition);
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetIf::on_pConditionLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "condition", m_spUi->pConditionLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetImage::CEosCommandWidgetImage(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetImage>())
{
  m_spUi->setupUi(this);

  m_pValidator = new QRegularExpressionValidator(m_spUi->pResourceLineEdit);
  m_pValidator->setRegularExpression(QRegularExpression(eos::c_sMatcherGallery));

  connect(m_spUi->pResourceLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetImage::on_pResourceLineEdit_editingFinished);
  connect(m_spUi->SearchButton, &QPushButton::clicked,
          this, &CEosCommandWidgetImage::on_SearchButton_clicked);

  m_spUi->pResourceSelectTree->installEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->installEventFilter(this);
  connect(m_spUi->pResourceSelectTree, &QTreeView::doubleClicked,
          this, &CEosCommandWidgetImage::on_pResourceSelectTree_doubleClicked);
}

CEosCommandWidgetImage::~CEosCommandWidgetImage()
{
  m_spUi->pResourceSelectTree->removeEventFilter(this);
  m_spUi->pResourceSelectTree->viewport()->removeEventFilter(this);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetImage::SetProperties(tInstructionMapValue* props)
{
  if (nullptr == props) { return; }

  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itLocator = GetValue<EArgumentType::eString>(*props, "locator");
  if (HasValue(*props, "locator") && IsOk<EArgumentType::eString>(itLocator))
  {
    QString sResourceLocator = std::get<QString>(itLocator);
    m_spUi->pResourceLineEdit->setText(sResourceLocator);
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetImage::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_bIsInitializing = true;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  pProxyModel->setSourceModel(pResourceModel);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

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
void CEosCommandWidgetImage::on_pResourceLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }

  QString sString = m_spUi->pResourceLineEdit->text();
  qint32 iPos = -1;
  QValidator::State state =
      m_pValidator->validate(sString, iPos);
  if (QValidator::State::Acceptable != state)
  {
    QToolTip::showText(m_spUi->pResourceLineEdit->pos(),
                       tr("Validation of gallery string failed<br>"
                          "Please use the selector or enter a string of format:<br>"
                          "&quot;gallery:&lt;bundle or name&gt;/&lt;optional rest of name&gt;&quot;"));
  }
  else
  {
    SetValue<EArgumentType::eString>(m_pProps, "locator", m_spUi->pResourceLineEdit->text());
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetImage::on_SearchButton_clicked()
{
  m_spUi->pStackedWidget->SlideInNext();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetImage::on_pResourceSelectTree_doubleClicked(const QModelIndex& index)
{
  auto pProxy =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(
        m_spUi->pResourceSelectTree->model());
  auto pSource = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
  QModelIndex idxModel = pProxy->mapToSource(index);
  if (idxModel.isValid())
  {
    if (pSource->IsResourceType(idxModel))
    {
      tspResource spResource = pSource->ResourceForIndex(idxModel);
      if (nullptr != spResource)
      {
        m_spUi->pStackedWidget->SlideInPrev();
        m_spUi->pResourceLineEdit->setText(eos::GetEosLocator(spResource));
        on_pResourceLineEdit_editingFinished();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CEosCommandWidgetImage::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (pObj == m_spUi->pResourceSelectTree && nullptr != pEvt)
  {
    if (QEvent::KeyPress == pEvt->type())
    {
      QKeyEvent* pKeyEvt = static_cast<QKeyEvent*>(pEvt);
      if (pKeyEvt->key() == Qt::Key_Enter || pKeyEvt->key() == Qt::Key_Return)
      {
        auto pProxy =
            dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(
              m_spUi->pResourceSelectTree->model());
        auto pSource = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
        QModelIndex idxProxy = m_spUi->pResourceSelectTree->currentIndex();
        QModelIndex idxModel = pProxy->mapToSource(idxProxy);
        if (idxModel.isValid())
        {
          if (pSource->IsResourceType(idxModel))
          {
            tspResource spResource = pSource->ResourceForIndex(idxModel);
            if (nullptr != spResource)
            {
              m_spUi->pStackedWidget->SlideInPrev();
              m_spUi->pResourceLineEdit->setText(eos::GetEosLocator(spResource));
              on_pResourceLineEdit_editingFinished();
              return true;
            }
          }
        }
      }
    }
  }
  if ((pObj == m_spUi->pResourceSelectTree ||
       m_spUi->pResourceSelectTree->viewport() == pObj) && nullptr != pEvt)
  {
    if (QEvent::ToolTip == pEvt->type())
    {
      QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(pEvt);

      QModelIndex index = m_spUi->pResourceSelectTree->indexAt(
            m_spUi->pResourceSelectTree->viewport()->mapFromGlobal(pHelpEvent->globalPos()));
      if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model()))
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

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetNotificationClose::CEosCommandWidgetNotificationClose(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetNotificationClose>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetNotificationClose::on_pLineEdit_editingFinished);
}

CEosCommandWidgetNotificationClose::~CEosCommandWidgetNotificationClose()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationClose::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itId = GetValue<EArgumentType::eString>(*props, "id");
  if (HasValue(*props, "id") && IsOk<EArgumentType::eString>(itId))
  {
    QString sId = std::get<QString>(itId);
    m_spUi->pLineEdit->setText(sId);
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationClose::on_pLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "id", m_spUi->pLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetNotificationCreate::CEosCommandWidgetNotificationCreate(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetNotificationCreate>())
{
  m_spUi->setupUi(this);

  m_pTimeValidator = new QRegularExpressionValidator(m_spUi->pDurationLineEdit);
  m_pTimeValidator->setRegularExpression(QRegularExpression(eos::c_sMatcherTimeDuration));

  connect(m_spUi->pIdLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetNotificationCreate::on_pIdLineEdit_editingFinished);
  connect(m_spUi->pTitleLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetNotificationCreate::on_pTitleLineEdit_editingFinished);
  connect(m_spUi->pButtonLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetNotificationCreate::on_pButtonLineEdit_editingFinished);
  connect(m_spUi->pDurationLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetNotificationCreate::on_pDurationLineEdit_editingFinished);
}

CEosCommandWidgetNotificationCreate::~CEosCommandWidgetNotificationCreate()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationCreate::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itId = GetValue<EArgumentType::eString>(*props, "id");
  const auto& itTitle = GetValue<EArgumentType::eString>(*props, "title");
  const auto& itButtonLabel = GetValue<EArgumentType::eString>(*props, "buttonLabel");
  const auto& itTimer = GetValue<EArgumentType::eString>(*props, "timerDuration");
  if (HasValue(*props, "id") && IsOk<EArgumentType::eString>(itId))
  {
    m_spUi->pIdLineEdit->setText(std::get<QString>(itId));
  }
  if (HasValue(*props, "title") && IsOk<EArgumentType::eString>(itTitle))
  {
    m_spUi->pTitleLineEdit->setText(std::get<QString>(itTitle));
  }
  if (HasValue(*props, "buttonLabel") && IsOk<EArgumentType::eString>(itButtonLabel))
  {
    m_spUi->pButtonLineEdit->setText(std::get<QString>(itButtonLabel));
  }
  if (HasValue(*props, "timerDuration") && IsOk<EArgumentType::eString>(itTimer))
  {
    m_spUi->pDurationLineEdit->setText(std::get<QString>(itTimer));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationCreate::on_pIdLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "id", m_spUi->pIdLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationCreate::on_pTitleLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "title", m_spUi->pTitleLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationCreate::on_pButtonLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "buttonLabel", m_spUi->pButtonLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetNotificationCreate::on_pDurationLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  QString sString = m_spUi->pDurationLineEdit->text();
  qint32 iPos = -1;
  QValidator::State state =
      m_pTimeValidator->validate(sString, iPos);
  if (QValidator::State::Acceptable != state)
  {
    QToolTip::showText(m_spUi->pDurationLineEdit->pos(),
                       tr("Validation of timer duration failed<br>"
                          "Please use the format [0-9]w[0-9]d[0-9]h[0-9]m[0-9]s[0-9]ms<br>"
                          "or enter a plain number for the number of miliseconds."));
  }
  else
  {
    SetValue<EArgumentType::eString>(m_pProps, "timerDuration", sString);
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetPrompt::CEosCommandWidgetPrompt(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetPrompt>())
{
  m_spUi->setupUi(this);

  connect(m_spUi->pLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetPrompt::on_pLineEdit_editingFinished);
}

CEosCommandWidgetPrompt::~CEosCommandWidgetPrompt()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetPrompt::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itVariable = GetValue<EArgumentType::eString>(*props, "variable");
  if (HasValue(*props, "variable") && IsOk<EArgumentType::eString>(itVariable))
  {
    m_spUi->pLineEdit->setText(std::get<QString>(itVariable));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetPrompt::on_pLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "variable", m_spUi->pLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetSay::CEosCommandWidgetSay(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetSay>())
{
  m_spUi->setupUi(this);
  for (qint32 i = 0; static_cast<qint32>(eos::c_vsAlignStrings.size()) > i; ++i)
  {
    m_spUi->pAlignComboBox->addItem(eos::c_vsAlignStrings[static_cast<size_t>(i)], i);
  }
  for (qint32 i = 0; static_cast<qint32>(eos::c_vsPlayModeStrings.size()) > i; ++i)
  {
    m_spUi->pModeComboBox->addItem(eos::c_vsPlayModeStrings[static_cast<size_t>(i)], i);
  }

  m_pTimeValidator = new QRegularExpressionValidator(m_spUi->pDurationLineEdit);
  m_pTimeValidator->setRegularExpression(QRegularExpression(eos::c_sMatcherTimeDuration));

  connect(m_spUi->pLabelPlainTextEdit->document(), &QTextDocument::contentsChange,
          this, &CEosCommandWidgetSay::on_pLabelPlainTextEdit_contentsChange);
  connect(m_spUi->pAlignComboBox, qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CEosCommandWidgetSay::on_pAlignComboBox_currentIndexChanged);
  connect(m_spUi->pModeComboBox, qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CEosCommandWidgetSay::on_pModeComboBox_currentIndexChanged);
  connect(m_spUi->pAllowSkipCheckBox, &QCheckBox::toggled,
          this, &CEosCommandWidgetSay::on_pAllowSkipCheckBox_toggled);
  connect(m_spUi->pDurationLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetSay::on_pDurationLineEdit_editingFinished);
}

CEosCommandWidgetSay::~CEosCommandWidgetSay()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itLabel = GetValue<EArgumentType::eString>(*props, "label");
  const auto& itAlign = GetValue<EArgumentType::eString>(*props, "align");
  const auto& itMode = GetValue<EArgumentType::eString>(*props, "mode");
  const auto& itAllowSkip = GetValue<EArgumentType::eBool>(*props, "allowSkip");
  const auto& itDuration = GetValue<EArgumentType::eString>(*props, "duration");
  if (HasValue(*props, "label") && IsOk<EArgumentType::eString>(itLabel))
  {
    m_spUi->pLabelPlainTextEdit->setPlainText(std::get<QString>(itLabel));
  }
  if (HasValue(*props, "align") && IsOk<EArgumentType::eString>(itAlign))
  {
    m_spUi->pAlignComboBox->setCurrentText(std::get<QString>(itAlign));
  }
  if (HasValue(*props, "mode") && IsOk<EArgumentType::eString>(itMode))
  {
    m_spUi->pModeComboBox->setCurrentText(std::get<QString>(itMode));
  }
  if (HasValue(*props, "allowSkip") && IsOk<EArgumentType::eBool>(itAllowSkip))
  {
    m_spUi->pAllowSkipCheckBox->setChecked(std::get<bool>(itAllowSkip));
  }
  if (HasValue(*props, "duration") && IsOk<EArgumentType::eString>(itDuration))
  {
    m_spUi->pDurationLineEdit->setText(std::get<QString>(itDuration));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::on_pLabelPlainTextEdit_contentsChange()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "label",
                                   m_spUi->pLabelPlainTextEdit->toPlainText());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::on_pAlignComboBox_currentIndexChanged(qint32)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "align",
                                   m_spUi->pAlignComboBox->currentText());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::on_pModeComboBox_currentIndexChanged(qint32 iIndex)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "mode",
                                   m_spUi->pModeComboBox->currentText());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::on_pAllowSkipCheckBox_toggled(bool bValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eBool>(m_pProps, "allowSkip", bValue);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetSay::on_pDurationLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  QString sString = m_spUi->pDurationLineEdit->text();
  qint32 iPos = -1;
  QValidator::State state =
      m_pTimeValidator->validate(sString, iPos);
  if (QValidator::State::Acceptable != state)
  {
    QToolTip::showText(m_spUi->pDurationLineEdit->pos(),
                       tr("Validation of timer duration failed<br>"
                          "Please use the format [0-9]w[0-9]d[0-9]h[0-9]m[0-9]s[0-9]ms<br>"
                          "or enter a plain number for the number of miliseconds."));
  }
  else
  {
    SetValue<EArgumentType::eString>(m_pProps, "duration", sString);
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetScene::CEosCommandWidgetScene(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetScene>())
{
  m_spUi->setupUi(this);

  m_pCompleter = new QCompleter(m_spUi->pSceneLineEdit);
  m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_pCompleterModel = new QStandardItemModel(m_pCompleter);
  m_pCompleter->setModel(m_pCompleterModel);
  m_spUi->pSceneLineEdit->setCompleter(m_pCompleter);

  connect(m_spUi->pSceneLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetScene::on_pSceneLineEdit_editingFinished);
}

CEosCommandWidgetScene::~CEosCommandWidgetScene()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetScene::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itTarget = GetValue<EArgumentType::eString>(*props, "target");
  if (HasValue(*props, "target") && IsOk<EArgumentType::eString>(itTarget))
  {
    m_spUi->pSceneLineEdit->setText(std::get<QString>(itTarget));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetScene::SetResourceModel(CResourceTreeItemModel* pResourceModel)
{
  m_pCompleterModel->removeRows(0, m_pCompleterModel->rowCount());
  tspProject spProject = pResourceModel->Project();
  QReadLocker locker(&spProject->m_rwLock);
  for (const tspScene& spScene : spProject->m_vspScenes)
  {
    QReadLocker locker(&spScene->m_rwLock);
    m_pCompleterModel->appendRow(new QStandardItem(spScene->m_sName));
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetScene::on_pSceneLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "target", m_spUi->pSceneLineEdit->text());
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
CEosCommandWidgetTimer::CEosCommandWidgetTimer(QWidget* pParent) :
  CEosCommandWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEosCommandWidgetTimer>())
{
  m_spUi->setupUi(this);
  for (qint32 i = 0; static_cast<qint32>(eos::c_vsTimerStyleStrings.size()) > i; ++i)
  {
    m_spUi->pStyleComboBox->addItem(eos::c_vsTimerStyleStrings[static_cast<size_t>(i)], i);
  }

  m_pTimeValidator = new QRegularExpressionValidator(m_spUi->pDurationLineEdit);
  m_pTimeValidator->setRegularExpression(QRegularExpression(eos::c_sMatcherTimeDuration));

  connect(m_spUi->pDurationLineEdit, &QLineEdit::editingFinished,
          this, &CEosCommandWidgetTimer::on_pDurationLineEdit_editingFinished);
  connect(m_spUi->pAsynchCheckBox, &QCheckBox::toggled,
          this, &CEosCommandWidgetTimer::on_pAsynchCheckBox_toggled);
  connect(m_spUi->pStyleComboBox, qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CEosCommandWidgetTimer::on_pStyleComboBox_currentIndexChanged);
}

CEosCommandWidgetTimer::~CEosCommandWidgetTimer()
{
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetTimer::SetProperties(tInstructionMapValue* props)
{
  m_bIsInitializing = true;

  m_pProps = props;

  const auto& itDuration = GetValue<EArgumentType::eString>(*props, "duration");
  const auto& itIsAsync = GetValue<EArgumentType::eBool>(*props, "isAsync");
  const auto& itStyle = GetValue<EArgumentType::eString>(*props, "style");
  if (HasValue(*props, "duration") && IsOk<EArgumentType::eString>(itDuration))
  {
    m_spUi->pDurationLineEdit->setText(std::get<QString>(itDuration));
  }
  if (HasValue(*props, "isAsync") && IsOk<EArgumentType::eBool>(itIsAsync))
  {
    m_spUi->pAsynchCheckBox->setChecked(std::get<bool>(itIsAsync));
  }
  if (HasValue(*props, "style") && IsOk<EArgumentType::eString>(itStyle))
  {
    m_spUi->pStyleComboBox->setCurrentText(std::get<QString>(itStyle));
  }

  m_bIsInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetTimer::on_pDurationLineEdit_editingFinished()
{
  if (m_bIsInitializing) { return; }
  QString sString = m_spUi->pDurationLineEdit->text();
  qint32 iPos = -1;
  QValidator::State state =
      m_pTimeValidator->validate(sString, iPos);
  if (QValidator::State::Acceptable != state)
  {
    QToolTip::showText(m_spUi->pDurationLineEdit->pos(),
                       tr("Validation of timer duration failed<br>"
                          "Please use the format [0-9]w[0-9]d[0-9]h[0-9]m[0-9]s[0-9]ms<br>"
                          "or enter a plain number for the number of miliseconds."));
  }
  else
  {
    SetValue<EArgumentType::eString>(m_pProps, "duration", sString);
    emit SignalChangedProperties();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetTimer::on_pAsynchCheckBox_toggled(bool bValue)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eBool>(m_pProps, "isAsync", bValue);
  emit SignalChangedProperties();
}

//----------------------------------------------------------------------------------------
//
void CEosCommandWidgetTimer::on_pStyleComboBox_currentIndexChanged(qint32)
{
  if (m_bIsInitializing) { return; }
  SetValue<EArgumentType::eString>(m_pProps, "style",
                                   m_spUi->pStyleComboBox->currentText());
  emit SignalChangedProperties();
}
