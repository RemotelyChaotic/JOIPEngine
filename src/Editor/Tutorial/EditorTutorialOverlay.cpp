#include "EditorTutorialOverlay.h"
#include "Application.h"
#include "Settings.h"
#include "Editor/EditorModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/NodeEditor/FlowView.h"
#include "Widgets/ShortcutButton.h"

#include <nodes/Node>

#include <QDialogButtonBox>
#include <QEvent>
#include <QGraphicsProxyWidget>
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

CEditorTutorialOverlay* CEditorTutorialOverlay::m_pInstance = nullptr;

//----------------------------------------------------------------------------------------
//
CEditorTutorialOverlay::CEditorTutorialOverlay(QWidget* pParent) :
  COverlayBase(1, pParent),
  m_highlightedImages(),
  m_vpClickFilterWidgets(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_pTutorialTextBox(nullptr),
  m_pTutorialTextLabel(nullptr),
  m_pTutorialpButtonBox(nullptr),
  m_pNextButton(nullptr),
  m_pEditorModel(nullptr),
  m_pAlphaAnimation(nullptr),
  m_updateTimer(),
  m_bInitialized(false),
  m_bClickToAdvanceEnabled(true),
  m_bTriggerNextAfterWidgetClick(true),
  m_iAnimatedAlpha(0)
{
  m_pInstance = this;
  setAttribute(Qt::WA_TranslucentBackground);
  connect(m_spSettings.get(), &CSettings::keyBindingsChanged,
          this, &CEditorTutorialOverlay::SlotKeyBindingsChanged);

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

  // filter
  installEventFilter(this);

  // Overlay Logic
  Climb();
  Resize();

  // create message dialog
  m_pTutorialTextBox = new COverlayBase(ZOrder()+1, this);
  m_pTutorialTextBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  QGridLayout* pLayout = new QGridLayout(m_pTutorialTextBox);
  m_pTutorialTextBox->setLayout(pLayout);

  m_pTutorialTextLabel = new QLabel(m_pTutorialTextBox);
  m_pTutorialTextLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  pLayout->addWidget(m_pTutorialTextLabel, 0, 0);

  m_pNextButton = new CShortcutButton(m_pTutorialTextBox);
  m_pNextButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  m_pNextButton->setObjectName("ForardButton");
  m_pNextButton->SetShortcut(m_spSettings->keyBinding("Skip"));
  m_pNextButton->setAutoDefault(true);
  connect(m_pNextButton, &QPushButton::clicked,
          this, &CEditorTutorialOverlay::SlotTriggerNextInstruction);
  m_pTutorialpButtonBox = new QDialogButtonBox(Qt::Horizontal, m_pTutorialTextBox);
  m_pTutorialpButtonBox->addButton(m_pNextButton, QDialogButtonBox::ActionRole);
  pLayout->addWidget(m_pTutorialpButtonBox, 1, 0);

  m_pTutorialTextBox->hide();
}

CEditorTutorialOverlay::~CEditorTutorialOverlay()
{
  m_pInstance = nullptr;
  Reset();
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorTutorialOverlay> CEditorTutorialOverlay::Instance()
{
  return m_pInstance;
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
void CEditorTutorialOverlay::SetClickFilterWidgets(const QStringList& vsWidgetNames,
                                                   bool bTriggerNextAfterClick)
{
  m_vpClickFilterWidgets.clear();
  m_vpClickFilterWidgets = FindWidgetsByName(vsWidgetNames);
  m_bTriggerNextAfterWidgetClick = bTriggerNextAfterClick;
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SetHighlightedWidgets(const QStringList& vsWidgetNames, bool bAlwaysOnTop)
{
  m_updateTimer.stop();
  m_highlightedImages.clear();

  QList<QPointer<QWidget>> vpList = FindWidgetsByName(vsWidgetNames);

  for (QWidget* pWidget : qAsConst(vpList))
  {
    QRect widgetGeometry = pWidget->geometry();
    QPoint tlGlobal =  MapPosToGlobal(pWidget, widgetGeometry.topLeft());
    widgetGeometry.moveTopLeft(mapFromGlobal(tlGlobal));

    QImage bitmap(pWidget->size(), QImage::Format_ARGB32);
    bitmap.fill(Qt::transparent);
    QPainter painter(&bitmap);
    pWidget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    painter.end();

    m_highlightedImages.insert({pWidget, { widgetGeometry, bitmap}});
  }

  repaint();

  if (bAlwaysOnTop)
  {
    m_updateTimer.start();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SetMouseTransparecny(bool bEnabled)
{
  setAttribute(Qt::WA_TransparentForMouseEvents, bEnabled);
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::ShowTutorialText(int anchor, double dPosX, double dPosY,
                                              bool bHideButtons, QString sText)
{
  if (nullptr != m_pTutorialTextBox)
  {
    m_pTutorialTextLabel->setText(sText);
    m_pTutorialpButtonBox->setVisible(!bHideButtons);

    m_pTutorialTextBox->show();
    m_pTutorialTextBox->updateGeometry();
    QSize thisSize = size();

    switch(anchor)
    {
    case EAnchors::eCenter:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width() / 2,
                               dPosY * thisSize.height() - m_pTutorialTextBox->height() / 2);
      break;
    case EAnchors::eLeft:
      m_pTutorialTextBox->move(dPosX * thisSize.width(),
                               dPosY * thisSize.height() - m_pTutorialTextBox->height() / 2);
      break;
    case EAnchors::eRight:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width(),
                               dPosY * thisSize.height() - m_pTutorialTextBox->height() / 2);
      break;
    case EAnchors::eTop:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width() / 2,
                               dPosY * thisSize.height());
      break;
    case EAnchors::eBottom:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width() / 2,
                               dPosY * thisSize.height() - m_pTutorialTextBox->height());
      break;
    case EAnchors::eTopLeft:
      m_pTutorialTextBox->move(dPosX * thisSize.width(),
                               dPosY * thisSize.height());
      break;
    case EAnchors::eTopRight:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width(),
                               dPosY * thisSize.height());
      break;
    case EAnchors::eBottomLeft:
      m_pTutorialTextBox->move(dPosX * thisSize.width(),
                               dPosY * thisSize.height() - m_pTutorialTextBox->height());
      break;
    case EAnchors::eBottomRight:
      m_pTutorialTextBox->move(dPosX * thisSize.width() - m_pTutorialTextBox->width(),
                               dPosY * thisSize.height() - m_pTutorialTextBox->height());
      break;
    default: break;
    }

    m_pTutorialTextBox->setParent(m_pTargetWidget);
    m_pTutorialTextBox->raise();
    m_pTutorialTextBox->show();
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
  if (!isVisible())
  {
    COverlayBase::Show();

    // start animation
    m_pAlphaAnimation->setStartValue(0.0);
    m_pAlphaAnimation->setEndValue(200.0);
    m_pAlphaAnimation->start();
    m_updateTimer.start();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::SlotTriggerNextInstruction()
{
  m_pTutorialTextBox->setParent(this);
  m_pTutorialTextBox->hide();
  emit SignalOverlayNextInstructionTriggered();
}

//----------------------------------------------------------------------------------------
//
bool CEditorTutorialOverlay::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt && isVisible())
  {
    if (this == pObj)
    {
      switch (pEvt->type())
      {
        case QEvent::MouseButtonRelease:
        {
          QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
          if (m_bClickToAdvanceEnabled && this == pObj)
          {
            SlotTriggerNextInstruction();
          }
          else if (auto pWidget = ClickedFilterChild(pMouseEvt->globalPos()))
          {
            QMouseEvent mouseEvent(QEvent::MouseButtonPress,
                                   pWidget->mapFromGlobal(pMouseEvt->globalPos()),
                                   pMouseEvt->windowPos(), pMouseEvt->screenPos(),
                                   Qt::LeftButton, Qt::LeftButton,
                                   Qt::NoModifier);
            QApplication::sendEvent(pWidget, &mouseEvent);
            QMouseEvent mouseEvent2(QEvent::MouseButtonRelease,
                                   pWidget->mapFromGlobal(pMouseEvt->globalPos()),
                                   pMouseEvt->windowPos(), pMouseEvt->screenPos(),
                                   Qt::LeftButton, Qt::LeftButton,
                                   Qt::NoModifier);
            QApplication::sendEvent(pWidget, &mouseEvent2);
            if (m_bTriggerNextAfterWidgetClick)
            {
              SlotTriggerNextInstruction();
            }
          }
        } break;
        default: break;
      }
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

  SlotTriggerNextInstruction();

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
  for (auto it = m_highlightedImages.begin(); m_highlightedImages.end() != it; ++it)
  {
    if (nullptr != it->first)
    {
      QRect widgetGeometry = it->first->geometry();
      QPoint tlGlobal =  MapPosToGlobal(it->first, widgetGeometry.topLeft());
      widgetGeometry.moveTopLeft(mapFromGlobal(tlGlobal));

      QImage bitmap(it->first->size(), QImage::Format_ARGB32);
      bitmap.fill(Qt::transparent);
      QPainter painter(&bitmap);
      it->first->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
      painter.end();

      it->second.second = bitmap;
    }
  }

  repaint();
}

//----------------------------------------------------------------------------------------
//
QPointer<QWidget> CEditorTutorialOverlay::ClickedFilterChild(const QPoint& locClickedGlobal)
{
  for (const QPointer<QWidget>& pWidget : qAsConst(m_vpClickFilterWidgets))
  {
    QPoint tl = MapPosToGlobal(pWidget, pWidget->geometry().topLeft());
    QPoint br = MapPosToGlobal(pWidget, pWidget->geometry().bottomRight());
    if (QRect(tl, br).contains(locClickedGlobal))
    {
      return pWidget;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QList<QPointer<QWidget>> CEditorTutorialOverlay::FindWidgetsByName(const QStringList& vsWidgetNames)
{
  QList<QPointer<QWidget>> vpList;
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

      // if this is a flow view check all node widgets as well
      CFlowView* pFlowWidget = parentWidget()->findChild<CFlowView*>();
      if (nullptr != pFlowWidget)
      {
        if (nullptr != pFlowWidget->Scene())
        {
          const auto& nodeMap = pFlowWidget->Scene()->nodes();
          for (auto it = nodeMap.begin(); nodeMap.end() != it; ++it)
          {
            if (it->second->nodeDataModel())
            {
              QWidget* pEmbedWidget = it->second->nodeDataModel()->embeddedWidget();
              if (nullptr != pEmbedWidget)
              {
                if (pEmbedWidget->isVisibleTo(this))
                {
                  if (pEmbedWidget->objectName() == sName && !vpList.contains(pEmbedWidget))
                  {
                    vpList.push_back(pEmbedWidget);
                  }
                  QList<QWidget*> vpFoundSubList =
                      pEmbedWidget->findChildren<QWidget*>(sName);
                  for (QWidget* pFoundWidget : qAsConst(vpFoundSubList))
                  {
                    if (pFoundWidget->isVisibleTo(this) && !vpList.contains(pFoundWidget))
                    {
                      vpList.push_back(pFoundWidget);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return vpList;
}

//----------------------------------------------------------------------------------------
//
// see <https://bugreports.qt.io/browse/QTBUG-41135> for why this is needed
QPoint CEditorTutorialOverlay::MapPosToGlobal(const QPointer<QWidget> pWidget,
                                              const QPoint& widgetPos)
{
  QWidget* pWindow = pWidget->window();
  QGraphicsProxyWidget* pGpw = pWindow->graphicsProxyWidget ();

  if (nullptr != pGpw)
  {
    QGraphicsScene* pScene = pGpw->scene();
    QList<QGraphicsView *> vpViews = pScene->views();
    if (vpViews.count() != 1)
    {
      return pWidget->mapToGlobal(widgetPos);
    }

    QGraphicsView* pView = vpViews[0];

    QPoint windowPos;
    if (nullptr != pWidget->parentWidget())
    {
      windowPos = pWidget->parentWidget()->mapTo(pWindow, widgetPos);
    }
    else
    {
      windowPos = pWidget->mapTo(pWindow, widgetPos);
    }
    QPointF proxyPosF = QPointF(windowPos.x(), windowPos.y());
    QPointF scenePosF = pGpw->mapToScene(proxyPosF);
    QPoint viewPos = pView->mapFromScene(scenePosF);

    return pView->mapToGlobal(viewPos);
  }
  else
  {
    if (nullptr == pWidget->parentWidget())
    {
      return pWidget->mapToGlobal(widgetPos - pWidget->geometry().topLeft());
    }
    else
    {
      return pWidget->parentWidget()->mapToGlobal(widgetPos);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorTutorialOverlay::Reset()
{
  m_pAlphaAnimation->stop();
  m_updateTimer.stop();
  m_iAnimatedAlpha = 0;

  SetClickFilterWidgets(QStringList());
  SetHighlightedWidgets(QStringList(), false);
}
