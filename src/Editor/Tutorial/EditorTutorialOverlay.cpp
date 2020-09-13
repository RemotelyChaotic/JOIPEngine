#include "EditorTutorialOverlay.h"
#include "Editor/EditorModel.h"

#include <QPainter>
#include <QPropertyAnimation>

namespace {
  const qint32 c_iUpdateInterval = 5;
  const qint32 c_iAnimationTime = 1000;

  const QColor c_backgroundColor = QColor(30, 30, 30);
}

//----------------------------------------------------------------------------------------
//
CEditorTutorialOverlay::CEditorTutorialOverlay(QWidget* pParent) :
  COverlayBase(1, pParent),
  m_pEditorModel(nullptr),
  m_pAlphaAnimation(nullptr),
  m_updateTimer(),
  m_bInitialized(false),
  m_iAnimatedAlpha(0)
{
  setAttribute(Qt::WA_TranslucentBackground);

  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CEditorTutorialOverlay::SlotUpdate, Qt::DirectConnection);

  m_pAlphaAnimation = new QPropertyAnimation(this, "alpha");
  m_pAlphaAnimation->setDuration(c_iAnimationTime);
  m_pAlphaAnimation->setEasingCurve(QEasingCurve::Linear);
  connect(m_pAlphaAnimation.data(), &QPropertyAnimation::finished,
          this, &CEditorTutorialOverlay::SlotAlphaAnimationFinished, Qt::DirectConnection);
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
void CEditorTutorialOverlay::paintEvent(QPaintEvent* pEvent)
{
  Q_UNUSED(pEvent);
  QPainter painter(this);
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
