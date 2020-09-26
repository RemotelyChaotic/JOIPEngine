#include "EditorTutorialOverlay.h"
#include "Application.h"
#include "Settings.h"
#include "Editor/EditorModel.h"
#include "Widgets/ShortcutButton.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOptionFrame>

namespace {
  const qint32 c_iUpdateInterval = 5;
  const qint32 c_iAnimationTime = 1000;

  const QColor c_backgroundColor = QColor(30, 30, 30);
}

//----------------------------------------------------------------------------------------
//
CEditorTutorialOverlay::CEditorTutorialOverlay(QWidget* pParent) :
  COverlayBase(1, pParent),
  m_highlightedImages(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_pTutorialTextBox(nullptr),
  m_pTutorialTextLabel(nullptr),
  m_pNextButton(nullptr),
  m_pEditorModel(nullptr),
  m_pAlphaAnimation(nullptr),
  m_updateTimer(),
  m_bInitialized(false),
  m_bClickToAdvanceEnabled(true),
  m_iAnimatedAlpha(0)
{
  setAttribute(Qt::WA_TranslucentBackground);
  connect(m_spSettings.get(), &CSettings::keyBindingsChanged,
          this, &CEditorTutorialOverlay::SlotKeyBindingsChanged);

  // create message dialog
  m_pTutorialTextBox = new QGroupBox(this);
  QGridLayout* pLayout = new QGridLayout(m_pTutorialTextBox);
  m_pTutorialTextBox->setLayout(pLayout);

  m_pTutorialTextLabel = new QLabel(m_pTutorialTextBox);
  pLayout->addWidget(m_pTutorialTextLabel, 0, 0);

  m_pNextButton = new CShortcutButton(m_pTutorialTextBox);
  m_pNextButton->setObjectName("ForardButton");
  m_pNextButton->SetShortcut(m_spSettings->keyBinding("Skip"));
  m_pNextButton->setAutoDefault(true);
  connect(m_pNextButton, &QPushButton::clicked,
          this, &CEditorTutorialOverlay::SlotTriggerNextInstruction);
  QDialogButtonBox* pButtonBox = new QDialogButtonBox(Qt::Horizontal, m_pTutorialTextBox);
  pButtonBox->addButton(m_pNextButton, QDialogButtonBox::ActionRole);
  pLayout->addWidget(pButtonBox, 1, 0);

  m_pTutorialTextBox->hide();


  // animation
  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CEditorTutorialOverlay::SlotUpdate, Qt::DirectConnection);

  m_pAlphaAnimation = new QPropertyAnimation(this, "alpha");
  m_pAlphaAnimation->setDuration(c_iAnimationTime);
  m_pAlphaAnimation->setEasingCurve(QEasingCurve::Linear);
  connect(m_pAlphaAnimation.data(), &QPropertyAnimation::finished,
          this, &CEditorTutorialOverlay::SlotAlphaAnimationFinished, Qt::DirectConnection);

  installEventFilter(this);
}

CEditorTutorialOverlay::~CEditorTutorialOverlay()
{
  Reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Initialize(QPointer<CEditorModel> pEditorModel)
{
  m_bInitialized = false;

  m_pEditorModel = pEditorModel;

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::NextTutorialState()
{
  if (m_bInitialized)
  {
    m_pEditorModel->NextTutorialState();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SetClickToAdvanceEnabled(bool bEnabled)
{
  m_bClickToAdvanceEnabled = bEnabled;
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SetHighlightedWidgets(const QStringList& vsWidgetNames)
{
  m_highlightedImages.clear();

  QList<QWidget*> vpList;
  if (nullptr != parentWidget())
  {
    for (const QString& sName : qAsConst(vsWidgetNames))
    {
      QList<QWidget*> vpFoundList =
        parentWidget()->findChildren<QWidget*>(sName);
      for (QWidget* pWidget : qAsConst(vpFoundList))
      {
        if (pWidget->isVisibleTo(this) && !vpList.contains(pWidget))
        {
          vpList.push_back(pWidget);
        }
      }
    }
  }

  for (QWidget* pWidget : qAsConst(vpList))
  {
    QRect widgetGeometry = pWidget->geometry();
    QPoint tl = pWidget->parentWidget()->mapToGlobal(widgetGeometry.topLeft());
    widgetGeometry.moveTopLeft(mapFromGlobal(tl));

    QImage bitmap(pWidget->size(), QImage::Format_ARGB32);
    bitmap.fill(Qt::transparent);
    QPainter painter(&bitmap);
    pWidget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    painter.end();

    m_highlightedImages.insert({pWidget, { widgetGeometry, bitmap}});
  }

  repaint();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::ShowTutorialText(double dCenterX, double dCenterY, QString sText)
{
  if (nullptr != m_pTutorialTextLabel)
  {
    m_pTutorialTextLabel->setText(sText);

    m_pTutorialTextBox->show();
    m_pTutorialTextBox->updateGeometry();
    QSize thisSize = size();
    m_pTutorialTextBox->move(dCenterX * thisSize.width() - m_pTutorialTextBox->width() / 2,
                             dCenterY * thisSize.height() - m_pTutorialTextBox->height() / 2);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Hide()
{
  Reset();
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Resize()
{
  move(0, 0);
  resize(m_pTargetWidget->width(), m_pTargetWidget->height());
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Show()
{
  COverlayBase::Show();

  // start animation
  m_pAlphaAnimation->setStartValue(0.0);
  m_pAlphaAnimation->setEndValue(200.0);
  m_pAlphaAnimation->start();
  m_updateTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SlotTriggerNextInstruction()
{
  m_pTutorialTextBox->hide();
  emit SignalOverlayNextInstructionTriggered();
}

//----------------------------------------------------------------------------------------
//
bool CEditorTutorialOverlay::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && this == pObj && nullptr != pEvt && isVisible())
  {
    switch (pEvt->type())
    {
      case QEvent::MouseButtonRelease:
      {
        if (m_bClickToAdvanceEnabled)
        {
          SlotTriggerNextInstruction();
        }
      } break;
      default: break;
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  painter.setClipRegion(pEvent->region());
  painter.setBackgroundMode(Qt::BGMode::TransparentMode);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                         QPainter::TextAntialiasing, true);

  // draw background
  painter.save();
  painter.setBrush(QColor(c_backgroundColor.red(), c_backgroundColor.green(),
                          c_backgroundColor.blue(), m_iAnimatedAlpha));
  painter.setPen(Qt::NoPen);
  painter.drawRect(geometry());
  painter.restore();

  // draw widgets on top
  for (auto it = m_highlightedImages.begin(); m_highlightedImages.end() != it; ++it)
  {
    painter.save();
    painter.setBrush(Qt::transparent);
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.drawImage(it->second.first, it->second.second, it->second.second.rect());
    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SlotAlphaAnimationFinished()
{
  // do a last update
  SlotUpdate();

  m_pAlphaAnimation->stop();
  m_updateTimer.stop();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SlotKeyBindingsChanged()
{
  if (nullptr != m_pNextButton && nullptr != m_spSettings)
  {
    m_pNextButton->SetShortcut(m_spSettings->keyBinding("Skip"));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SlotUpdate()
{
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Reset()
{
  m_pAlphaAnimation->stop();
  m_updateTimer.stop();
  m_iAnimatedAlpha = 0;
}
