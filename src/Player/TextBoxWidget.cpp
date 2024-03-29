#include "TextBoxWidget.h"
#include "Application.h"
#include "Constants.h"
#include "Settings.h"
#include "Widgets/FlowLayout.h"
#include "ui_TextBoxWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScrollBar>

namespace
{
  const char* c_sPropertyButtonIndex = "Index";
}

//----------------------------------------------------------------------------------------
//
CTextBoxWidget::CTextBoxWidget(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CTextBoxWidget>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_vCurrentBackgroundColor(),
  m_vCurrentTextColor(),
  m_bSceneSelection(false)
{
  m_vCurrentBackgroundColor.push_back(Qt::black);
  m_vCurrentTextColor.push_back(Qt::white);

  m_spUi->setupUi(this);
  m_spUi->pScrollArea->setStyleSheet("background-color:transparent;");

  m_pTextSliderValueAnimation =
      new QPropertyAnimation(m_spUi->pScrollArea->verticalScrollBar(), "value",
                             m_spUi->pScrollArea->verticalScrollBar());
  m_pTextSliderValueAnimation->setDuration(500);

  QLinearGradient alphaGradient(rect().topLeft(),
                                rect().bottomLeft());
  alphaGradient.setColorAt(0.0, Qt::transparent);
  alphaGradient.setColorAt(0.2, Qt::black);
  alphaGradient.setColorAt(0.8, Qt::black);
  alphaGradient.setColorAt(1.0, Qt::black);
  QGraphicsOpacityEffect* pOpacityEffect = new QGraphicsOpacityEffect(this);
  pOpacityEffect->setOpacity(1.0);
  pOpacityEffect->setOpacityMask(alphaGradient);
  setGraphicsEffect(pOpacityEffect);
}

CTextBoxWidget::~CTextBoxWidget()
{
  if (!m_pTextSliderValueAnimation.isNull())
  {
    m_pTextSliderValueAnimation->stop();
  }

  m_vCurrentBackgroundColor.clear();
  m_vCurrentTextColor.clear();
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::Initialize()
{
  m_bInitialized = false;

  m_spUi->pScrollArea->horizontalScrollBar()->hide();
  m_spUi->pScrollArea->verticalScrollBar()->hide();
  m_spUi->pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  m_spUi->pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);


  connect(m_spUi->pScrollArea->verticalScrollBar(), &QAbstractSlider::rangeChanged,
          this, &CTextBoxWidget::SlotSliderRangeChanged, Qt::QueuedConnection);

  QLayout* pLayout = m_spUi->pContent->layout();
  if (nullptr != pLayout)
  {
    QSpacerItem* pHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    pLayout->addItem(pHorizontalSpacer);
  }

  m_bSceneSelection = false;

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotClearText()
{
  QLayoutItem* pItem;
  while ((pItem = m_spUi->pContent->layout()->takeAt(0)))
  {
    delete pItem->widget();
    delete pItem;
  }
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotOnButtonPromptClicked()
{
  QPushButton* pButton = qobject_cast<QPushButton*>(sender());
  pButton->disconnect();
  if (nullptr != pButton->parent())
  {
    auto vpChildren = pButton->parent()->findChildren<QPushButton*>();
    for (QPushButton* pFoundButton : qAsConst(vpChildren))
    {
      pFoundButton->disconnect();
    }
  }

  qint32 iIndex = pButton->property(c_sPropertyButtonIndex).toInt();

  if (!SceneSelection())
  {
    emit SignalShowButtonReturnValue(iIndex);
  }
  else
  {
    emit SignalShowSceneSelectReturnValue(iIndex);
    SetSceneSelection(false);
  }
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotOnInputEditingFinished()
{
  QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
  pEdit->disconnect();

  emit SignalShowInputReturnValue(pEdit->text());

  pEdit->setText("> " + pEdit->text());
  pEdit->setReadOnly(true);
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotShowButtonPrompts(QStringList vsLabels)
{
  QLayout* pLayout = m_spUi->pContent->layout();
  if (nullptr != pLayout)
  {
    QWidget* pRoot = new QWidget(m_spUi->pContent);
    pRoot->setFixedWidth(m_spUi->pContent->width());
    CFlowLayout* pFlowLayout = new CFlowLayout(pRoot, 10, 10, 10);

    size_t iIndex = 0;
    for (const QString& sLabel : qAsConst(vsLabels))
    {
      QPushButton* pButton = new QPushButton(sLabel, pRoot);
      pButton->setProperty(c_sPropertyButtonIndex, static_cast<qint32>(iIndex));
      pButton->setStyleSheet(QString("QPushButton { background-color: rgb(%1, %2, %3); "
                                     "color : rgb(%4, %5, %6); "
                                     "border-image: none;"
                                     "border: 1px solid " SILVER_QML "; "
                                     "border-radius: 10px; "
                                     "padding: 6px;}")
          .arg(iIndex < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[iIndex].red() : 0)
          .arg(iIndex < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[iIndex].green() : 0)
          .arg(iIndex < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[iIndex].blue() : 0)
          .arg(iIndex < m_vCurrentTextColor.size() ? m_vCurrentTextColor[iIndex].red() : 255)
          .arg(iIndex < m_vCurrentTextColor.size() ? m_vCurrentTextColor[iIndex].green() : 255)
          .arg(iIndex < m_vCurrentTextColor.size() ? m_vCurrentTextColor[iIndex].blue() : 255));
      pButton->setContentsMargins(6,6,6,6);
      QFont wFont = pButton->font();
      wFont.setPixelSize(16);
      wFont.setFamily(m_spSettings->Font());
      pButton->setFont(wFont);
      AddDropShadow(pButton);

      connect(pButton, &QAbstractButton::clicked,
              this, &CTextBoxWidget::SlotOnButtonPromptClicked);

      pFlowLayout->addWidget(pButton);

      ++iIndex;
    }

    pRoot->setLayout(pFlowLayout);
    pLayout->addWidget(pRoot);
  }

  ScrollToBottom();
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotShowInput()
{
  QLayout* pLayout = m_spUi->pContent->layout();
  if (nullptr != pLayout)
  {
    QWidget* pRoot = new QWidget(m_spUi->pContent);
    pRoot->setFixedWidth(m_spUi->pContent->width());
    QHBoxLayout* pHorizontalLayout = new QHBoxLayout(pRoot);

    QSpacerItem* pHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    pHorizontalLayout->addItem(pHorizontalSpacer);

    QGroupBox* pGroupBox = new QGroupBox(pRoot);
    pGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pGroupBox->setStyleSheet(QString("QGroupBox { "
                                     "background-color: rgb(%1, %2, %3); "
                                     "border: 1px solid " SILVER_QML "; "
                                     "border-radius: 5px; }")
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].red() : 0)
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].green() : 0)
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].blue() : 0));
    AddDropShadow(pGroupBox);

    QHBoxLayout* pHorizontalLayout2 = new QHBoxLayout(pGroupBox);
    QLineEdit* pEdit = new QLineEdit(pGroupBox);
    pEdit->setMinimumSize({1, 1});
    pEdit->setAlignment(Qt::AlignCenter);
    pEdit->setFrame(false);
    pEdit->setPlaceholderText(".....");
    pEdit->setStyleSheet(QString("QLineEdit { "
                                 "background-color: rgb(%1, %2, %3); "
                                 "color : rgb(%4, %5, %6); }")
         .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].red() : 0)
         .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].green() : 0)
         .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].blue() : 0)
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].red() : 255)
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].green() : 255)
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].blue() : 255));
    QFont wFont = pEdit->font();
    wFont.setPixelSize(16);
    wFont.setFamily(m_spSettings->Font());
    pEdit->setFont(wFont);
    pHorizontalLayout2->addWidget(pEdit);

    connect(pEdit, &QLineEdit::editingFinished,
            this, &CTextBoxWidget::SlotOnInputEditingFinished);

    pHorizontalLayout->addWidget(pGroupBox);

    QSpacerItem* pHorizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    pHorizontalLayout->addItem(pHorizontalSpacer2);

    pLayout->addWidget(pRoot);
  }

  ScrollToBottom();
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotShowText(QString sText)
{
  QLayout* pLayout = m_spUi->pContent->layout();
  if (nullptr != pLayout)
  {
    QWidget* pRoot = new QWidget(m_spUi->pContent);
    pRoot->setFixedWidth(m_spUi->pContent->width());
    QHBoxLayout* pHorizontalLayout = new QHBoxLayout(pRoot);

    QSpacerItem* pHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    pHorizontalLayout->addItem(pHorizontalSpacer);

    QGroupBox* pGroupBox = new QGroupBox(pRoot);
    pGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pGroupBox->setStyleSheet(QString("QGroupBox { "
                                     "background-color: rgb(%1, %2, %3); "
                                     "border: 1px solid " SILVER_QML "; "
                                     "border-radius: 10px; }")
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].red() : 0)
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].green() : 0)
        .arg(0 < m_vCurrentBackgroundColor.size() ? m_vCurrentBackgroundColor[0].blue() : 0));
    AddDropShadow(pGroupBox);

    QHBoxLayout* pHorizontalLayout2 = new QHBoxLayout(pGroupBox);
    QLabel* pLabel = new QLabel(pGroupBox);
    pLabel->setMinimumSize({1, 1});
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setTextFormat(Qt::RichText);
    pLabel->setText(sText);
    pLabel->setStyleSheet(QString("QLabel { color : rgb(%1, %2, %3); }")
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].red() : 255)
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].green() : 255)
         .arg(0 < m_vCurrentTextColor.size() ? m_vCurrentTextColor[0].blue() : 255));
    QFont wFont = pLabel->font();
    wFont.setPixelSize(16);
    wFont.setFamily(m_spSettings->Font());
    pLabel->setFont(wFont);
    pHorizontalLayout2->addWidget(pLabel);


    pHorizontalLayout->addWidget(pGroupBox);

    QSpacerItem* pHorizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    pHorizontalLayout->addItem(pHorizontalSpacer2);

    pLayout->addWidget(pRoot);
  }

  ScrollToBottom();
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotTextBackgroundColorsChanged(std::vector<QColor> vColors)
{
  m_vCurrentBackgroundColor = vColors;
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotTextColorsChanged(std::vector<QColor> vColors)
{
  m_vCurrentTextColor = vColors;
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::SlotSliderRangeChanged()
{
  ScrollToBottom();
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::AddDropShadow(QWidget* pWidget)
{
  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pWidget);
  pShadow->setBlurRadius(5);
  pShadow->setXOffset(5);
  pShadow->setYOffset(5);
  pShadow->setColor(Qt::black);
  pWidget->setGraphicsEffect(pShadow);
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::ScrollToBottom()
{
  if (!m_pTextSliderValueAnimation.isNull())
  {
    m_pTextSliderValueAnimation->stop();

    m_pTextSliderValueAnimation->setStartValue(
          m_spUi->pScrollArea->verticalScrollBar()->value());
    m_pTextSliderValueAnimation->setEndValue(
          m_spUi->pScrollArea->verticalScrollBar()->maximum());

    m_pTextSliderValueAnimation->start();
  }
}
