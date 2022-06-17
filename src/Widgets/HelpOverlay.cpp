#include "HelpOverlay.h"
#include "Application.h"
#include "Settings.h"
#include "ui_HelpOverlay.h"
#include "Systems/HelpFactory.h"
#include "Widgets/Editor/HighlightedSearchableTextEdit.h"
#include "Widgets/Editor/TextEditZoomEnabler.h"
#include "Widgets/ShortcutButton.h"

//#include <private/qpixmapfilter_p.h>

#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QPainter>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScroller>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QVBoxLayout>
#include <QTextBrowser>

namespace  {
  const qint32 c_iOffsetOfHelpWindow = 40;
  const qint32 c_iAnimationTime = 1500;
  const qint32 c_iUpdateInterval = 5;

  const QColor c_backgroundColor = QColor(30, 30, 30, 200);

  //--------------------------------------------------------------------------------------
  //
  void FindWidgetsForHelp(QWidget* pRootToSearch, std::vector<QPointer<QWidget>>& vpWidgetMap)
  {
    if (nullptr != pRootToSearch)
    {
      // get children
      auto vpChildren = pRootToSearch->children();
      for (auto pChild : qAsConst(vpChildren))
      {
        QWidget* pWidget = dynamic_cast<QWidget*>(pChild);
        if (nullptr != pWidget)
        {
          // only search for visible widgets
          if (pWidget->isVisible())
          {
            if (pWidget->property(helpOverlay::c_sHelpPagePropertyName).isValid())
            {
              vpWidgetMap.push_back(pWidget);
            }
            // search it's children
            FindWidgetsForHelp(pWidget, vpWidgetMap);
          }
        }
      }
    }
  }
}

const char* const helpOverlay::c_sHelpPagePropertyName = "HelpPageId";
QPointer<CHelpOverlay> CHelpOverlay::m_pHelpOverlay = nullptr;


//----------------------------------------------------------------------------------------
//
CHelpOverlayBackGround::CHelpOverlayBackGround(QWidget* pParent) :
  QWidget(pParent),
  m_bOpened(false),
  m_circleOrigin(0, 0),
  m_iCircleRadius(0),
  //m_pDropShadowFilter(new QPixmapDropShadowFilter(this)),
  m_shownIconImages(),
  m_cursor(-1, -1),
  m_updateTimer()
{
  //m_pDropShadowFilter->setOffset(0, 0);
  //m_pDropShadowFilter->setBlurRadius(8);
  //m_pDropShadowFilter->setColor(QColor(255, 255, 255, 100));

  m_pCircleAnimation = new QPropertyAnimation(this, "circleRadius");
  m_pCircleAnimation->setDuration(c_iAnimationTime);
  m_pCircleAnimation->setEasingCurve(QEasingCurve::InQuart);
  connect(m_pCircleAnimation.data(), &QPropertyAnimation::finished,
          this, &CHelpOverlayBackGround::SlotCircleAnimationFinished, Qt::DirectConnection);
  connect(m_pCircleAnimation.data(), &QPropertyAnimation::finished,
          this, &CHelpOverlayBackGround::SignalCircleAnimationFinished, Qt::DirectConnection);

  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CHelpOverlayBackGround::SlotUpdate, Qt::DirectConnection);

  m_circleOrigin = QPoint(width() / 2, height() / 2);
  setMouseTracking(true);
}
CHelpOverlayBackGround::~CHelpOverlayBackGround()
{
  Reset();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::Reset()
{
  m_pCircleAnimation->stop();
  m_updateTimer.stop();
  m_shownIconImages.clear();
  m_iCircleRadius = 0;
  m_bOpened = false;
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::StartAnimation(const QPoint& origin, qint32 iEndValue)
{
  m_circleOrigin = origin;
  m_pCircleAnimation->setStartValue(m_iCircleRadius);
  m_pCircleAnimation->setEndValue(iEndValue);
  m_pCircleAnimation->start();
  m_updateTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::SlotCircleAnimationFinished()
{
  // do a last update
  SlotUpdate();

  m_pCircleAnimation->stop();
  m_updateTimer.stop();
  m_bOpened = m_iCircleRadius != 0;
  if (!m_bOpened)
  {
    m_shownIconImages.clear();
  }

  MousePositionCheck();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::SlotUpdate()
{
  // get "pinged" widgets
  QVector2D center(m_circleOrigin);
  auto pHelpwidget = CHelpOverlay::Instance();
  if (nullptr != pHelpwidget)
  {
    for (auto pWidget : pHelpwidget->m_vpHelpWidgets)
    {
      auto it = m_shownIconImages.find(pWidget.data());
      QRect widgetGeometry = pWidget->geometry();
      widgetGeometry.moveTopLeft(parentWidget()->mapFromGlobal(
            pWidget->parentWidget()->mapToGlobal(
              widgetGeometry.topLeft())));
      float fCircleRadius = static_cast<float>(m_iCircleRadius);
      QVector2D tl(widgetGeometry.topLeft());
      QVector2D tr(widgetGeometry.topRight());
      QVector2D bl(widgetGeometry.bottomLeft());
      QVector2D br(widgetGeometry.bottomRight());
      if (center.distanceToPoint(tl) < fCircleRadius &&
          center.distanceToPoint(tr) < fCircleRadius &&
          center.distanceToPoint(bl) < fCircleRadius &&
          center.distanceToPoint(br) < fCircleRadius)
      {
        if (m_shownIconImages.end() == it)
        {
          QImage bitmap(pWidget->size(), QImage::Format_ARGB32);
          bitmap.fill(Qt::transparent);
          QPainter painter(&bitmap);
          pWidget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
          painter.end();

          m_shownIconImages.insert({pWidget.data(),
                                    { QRect(tl.toPoint(), br.toPoint()),
                                      bitmap}});
        }
      }
      else
      {
        if (m_shownIconImages.end() != it)
        {
          m_shownIconImages.erase(it);
        }
      }
    }
  }

  // repaint
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::mouseMoveEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    m_cursor = parentWidget()->mapFromGlobal(pEvt->globalPos());
    MousePositionCheck();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::paintEvent(QPaintEvent* pEvt)
{
  Q_UNUSED(pEvt)

  QPainter painter(this);
  painter.setBackgroundMode(Qt::BGMode::TransparentMode);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                         QPainter::TextAntialiasing, true);

  // draw background
  painter.save();
  painter.setBrush(c_backgroundColor);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(m_circleOrigin, m_iCircleRadius, m_iCircleRadius);
  painter.restore();

  bool bSomethingSelected = false;
  QRect selection;
  QPixmap selectionImage;

  // draw widgets on top with border to highlight them
  QStyleOptionFrame option;
  QPalette originalpalette = option.palette;
  option.initFrom(this);
  option.state = QStyle::State_Sunken;
  option.frameShape = QFrame::HLine;
  for (auto it = m_shownIconImages.begin(); m_shownIconImages.end() != it; ++it)
  {
    painter.save();
    painter.setBrush(Qt::transparent);
    painter.setBackgroundMode(Qt::OpaqueMode);

    QRect rectOfItem = it->second.first;

    if (it->second.first.contains(m_cursor))
    {
      bSomethingSelected = true;
      selection = it->second.first;

      double dRad =
          static_cast<double>(static_cast<qint32>(static_cast<double>
             (QDateTime::currentDateTime().toMSecsSinceEpoch() % 1000000) / 5.0) % 360) / 180.0 * 3.1415;
      double dMultiplyer = std::sin(dRad);
      double dWToHeight = static_cast<double>(rectOfItem.width()) / rectOfItem.height();
      rectOfItem = selection.adjusted(static_cast<qint32>(-20 -10 * dMultiplyer),
                                      static_cast<qint32>((-20 -10 * dMultiplyer) / dWToHeight),
                                      static_cast<qint32>(20 + 10 * dMultiplyer),
                                      static_cast<qint32>((20 + 10 * dMultiplyer) / dWToHeight));
      selection = rectOfItem;
      selectionImage = QPixmap(rectOfItem.width(), rectOfItem.height());
    }

    // draw selection
    //if (bSomethingSelected)
    //{
    //  painter.save();
    //  m_pDropShadowFilter->draw(&painter, selection.topLeft(), selectionImage, selection);
    //  painter.restore();
    //}

    painter.drawImage(rectOfItem, it->second.second, it->second.second.rect());
    option.rect = rectOfItem;
    style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::MousePositionCheck()
{
   bool bStartTimer = false;
  for (auto it = m_shownIconImages.begin(); m_shownIconImages.end() != it; ++it)
  {
    if (it->second.first.contains(m_cursor))
    {
      bStartTimer = true;
      break;
    }
  }

  if (!bStartTimer)
  {
    m_cursor = QPoint(-1, -1);
  }

  if (bStartTimer)
  {
    m_updateTimer.start();
  }
  else if (m_pCircleAnimation->state() != QAbstractAnimation::State::Running)
  {
    m_updateTimer.stop();
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
CHelpOverlay::CHelpOverlay(QPointer<CHelpButtonOverlay> pHelpButton, QWidget* pParent) :
  COverlayBase(100, pParent),
  m_vpHelpWidgets(),
  m_spUi(std::make_unique<Ui::CHelpOverlay>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_wpHelpFactory(CApplication::Instance()->System<CHelpFactory>()),
  m_pHighlightedSearchableEdit(nullptr),
  m_pHelpButton(pHelpButton)
{
  setAttribute(Qt::WA_TranslucentBackground);

  m_spUi->setupUi(this);
  m_pHelpOverlay = this;

  m_spUi->pHtmlBrowser->setUndoRedoEnabled(false);

#if defined(Q_OS_ANDROID)
  QScroller::grabGesture(m_spUi->pHtmlBrowser, QScroller::LeftMouseButtonGesture);
#endif

  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(m_spUi->pHtmlBrowser);
  m_pHighlightedSearchableEdit->SetSyntaxHighlightingEnabled(false);
  m_pZoomEnabler = new CTextEditZoomEnabler(m_spUi->pHtmlBrowser);

  m_pBackground = new CHelpOverlayBackGround(this);
  m_pBackground->installEventFilter(this);
  m_spUi->pHtmlBrowserBox->hide();
  m_spUi->pHtmlBrowserBox->raise();

  QStandardItemModel* pModel = new QStandardItemModel(m_spUi->pHelpSearchTree);
  QSortFilterProxyModel* pProxy = new QSortFilterProxyModel(m_spUi->pHelpSearchTree);
  pProxy->setSourceModel(pModel);
  m_spUi->pHelpSearchTree->setModel(pProxy);
  pModel->setColumnCount(1);
  pModel->setHorizontalHeaderLabels(QStringList() << "Topic");
  connect(m_spUi->pHelpSearchTree->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &CHelpOverlay::SlotCurrentIndexChanged);

  connect(m_pBackground, &CHelpOverlayBackGround::SignalCircleAnimationFinished,
          this, &CHelpOverlay::SlotCircleAnimationFinished, Qt::DirectConnection);
  connect(m_spSettings.get(), &CSettings::keyBindingsChanged,
          this, &CHelpOverlay::SlotKeyBindingsChanged);

  SlotKeyBindingsChanged();
}

CHelpOverlay::~CHelpOverlay()
{
  m_pHelpOverlay = nullptr;
}

//----------------------------------------------------------------------------------------
//
QPointer<CHelpOverlay> CHelpOverlay::Instance()
{
  return m_pHelpOverlay;
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Hide()
{
  COverlayBase::Hide();
  m_spUi->pHtmlBrowserBox->hide();
  m_pBackground->Reset();
  m_vpHelpWidgets.clear();

  emit SignalOverlayClosed();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Resize()
{
  COverlayBase::Resize();
  QSize parentSize = m_pTargetWidget->size();
  m_pBackground->resize(parentSize);
  m_spUi->pHtmlBrowserBox->resize(parentSize.width() - c_iOffsetOfHelpWindow * 2,
                                  parentSize.height() - c_iOffsetOfHelpWindow * 2);
  m_spUi->pHtmlBrowserBox->move(c_iOffsetOfHelpWindow, c_iOffsetOfHelpWindow);

#if defined(Q_OS_ANDROID)
  if (parentSize.width() > parentSize.height())
  {
    m_spUi->pSplitter->setSizes({m_spUi->pHtmlBrowserBox->width() / 5,
                                 m_spUi->pHtmlBrowserBox->width() * 4 / 5});
  }
  else
  {
    m_spUi->pSplitter->setSizes({0, m_spUi->pHtmlBrowserBox->width()});
  }
#else
  m_spUi->pSplitter->setSizes({m_spUi->pHtmlBrowserBox->width() / 5,
                               m_spUi->pHtmlBrowserBox->width() * 4 / 5});
#endif
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Show()
{
  COverlayBase::Show();
  m_spUi->pHtmlBrowserBox->hide();
  m_spUi->pHtmlBrowserBox->raise();

  emit SignalOverlayOpened();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Show(const QPoint& animationOrigin, QWidget* pRootToSearch)
{
  Show();

  QSize parentSize = m_pTargetWidget->size();
  m_pBackground->StartAnimation(animationOrigin,
        static_cast<qint32>(std::sqrt(parentSize.width() * parentSize.width() +
                                      parentSize.height() * parentSize.height())));

  m_vpHelpWidgets.clear();
  FindWidgetsForHelp(pRootToSearch, m_vpHelpWidgets);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::SlotCircleAnimationFinished()
{
  if (!m_pBackground->m_bOpened)
  {
    Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::SlotKeyBindingsChanged()
{
  m_spUi->CloseButton->SetShortcut(m_spSettings->keyBinding("Exit"));
  m_spUi->BackButton->SetShortcut(m_spSettings->keyBinding("Backward"));
  m_spUi->HomeButton->SetShortcut(m_spSettings->keyBinding("Help"));
  m_spUi->ForardButton->SetShortcut(m_spSettings->keyBinding("Foreward"));
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_pHtmlBrowser_backwardAvailable(bool bAvailable)
{
  m_spUi->BackButton->setEnabled(bAvailable);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_pHtmlBrowser_forwardAvailable(bool bAvailable)
{
  m_spUi->ForardButton->setEnabled(bAvailable);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_BackButton_clicked()
{
  m_spUi->pHtmlBrowser->backward();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_HomeButton_clicked()
{
  m_spUi->pHtmlBrowser->home();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_ForardButton_clicked()
{
  m_spUi->pHtmlBrowser->forward();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_CloseButton_clicked()
{
  m_spUi->pHtmlBrowser->clearHistory();
  m_spUi->pHtmlBrowserBox->hide();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_pHtmlBrowser_sourceChanged(const QUrl& source)
{
  if (auto spFactory = m_wpHelpFactory.lock())
  {
    QString sSource = source.toLocalFile();
    if (sSource.startsWith("qrc")) { sSource = sSource.mid(3); }

    for (const auto& pair : spFactory->HelpMap())
    {
      if (pair.second == sSource)
      {
        QStringList vsList = pair.first.split("/");
        QSortFilterProxyModel* pProxy =
            dynamic_cast<QSortFilterProxyModel*>(m_spUi->pHelpSearchTree->model());
        if (nullptr != pProxy)
        {
          QStandardItemModel* pModel = dynamic_cast<QStandardItemModel*>(pProxy->sourceModel());
          if (nullptr != pModel)
          {
            QList<QStandardItem*> vpItems = pModel->findItems(vsList.last(),
                                                              Qt::MatchRecursive | Qt::MatchFixedString);

            QStandardItem* pItemFound = nullptr;
            for (QStandardItem* pItem : qAsConst(vpItems))
            {
              QStandardItem* pItemParent = pItem->parent();
              for (qint32 i = vsList.size() - 2; -1 < i; --i)
              {
                if (nullptr != pItemParent)
                {
                  if (pItemParent->data(Qt::DisplayRole) != vsList[i])
                  {
                    break;
                  }
                  else
                  {
                    pItemParent = pItemParent->parent();
                    if (nullptr == pItemParent || pItemParent == pModel->invisibleRootItem())
                    {
                      pItemFound = pItem;
                      goto loopEnd;
                    }
                  }
                }
              }
            }
            loopEnd:

            if (nullptr != pItemFound)
            {
              QModelIndex index = pProxy->mapFromSource(pModel->indexFromItem(pItemFound));
              m_spUi->pHelpSearchTree->selectionModel()->blockSignals(true);
              m_spUi->pHelpSearchTree->selectionModel()->select(index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
              m_spUi->pHelpSearchTree->scrollTo(index);
              m_spUi->pHelpSearchTree->selectionModel()->blockSignals(false);
            }
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::SlotCurrentIndexChanged(const QModelIndex& current,
                                           const QModelIndex& previous)
{
  Q_UNUSED(previous)
  QSortFilterProxyModel* pProxy =
      dynamic_cast<QSortFilterProxyModel*>(m_spUi->pHelpSearchTree->model());
  if (nullptr != pProxy)
  {
    QStandardItemModel* pModel = dynamic_cast<QStandardItemModel*>(pProxy->sourceModel());
    if (nullptr != pModel)
    {
      QStandardItem* pItem = pModel->itemFromIndex(pProxy->mapToSource(current));
      if (nullptr != pItem)
      {
        const QString sPath = pItem->data().toString();
        m_spUi->pHtmlBrowser->setSource(QUrl::fromLocalFile(sPath));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CHelpOverlay::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    switch (pEvt->type())
    {
      case QEvent::MouseButtonPress:
      {
        QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
        // if help window is not open, hide overlay
        if (!m_spUi->pHtmlBrowserBox->isVisible())
        {
          // find clicked widget
          for (auto pWidget : m_vpHelpWidgets)
          {
            QVector2D center(m_pBackground->mapToGlobal(m_pBackground->m_circleOrigin));
            QRect widgetGeometry = pWidget->geometry();
            widgetGeometry.moveTopLeft(pWidget->parentWidget()->mapToGlobal(widgetGeometry.topLeft()));
            float fCircleRadius = static_cast<float>(m_pBackground->m_iCircleRadius);
            QVector2D tl(widgetGeometry.topLeft());
            QVector2D tr(widgetGeometry.topRight());
            QVector2D bl(widgetGeometry.bottomLeft());
            QVector2D br(widgetGeometry.bottomRight());
            QPoint mousePos = pMouseEvt->globalPos();
            if (center.distanceToPoint(tl) < fCircleRadius &&
                center.distanceToPoint(tr) < fCircleRadius &&
                center.distanceToPoint(bl) < fCircleRadius &&
                center.distanceToPoint(br) < fCircleRadius &&
                widgetGeometry.contains(mousePos))
            {
              // found -> open ui
              ShowHelp(pWidget->property(helpOverlay::c_sHelpPagePropertyName).toString());
              return COverlayBase::eventFilter(pObj, pEvt);
            }
          }

          m_pBackground->StartAnimation(m_pBackground->m_circleOrigin, 0);
        }
      } break;
    default: break;
    }
  }

  return COverlayBase::eventFilter(pObj, pEvt);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::InitTree()
{
  if (auto spFactory = m_wpHelpFactory.lock())
  {
    QSortFilterProxyModel* pProxy =
        dynamic_cast<QSortFilterProxyModel*>(m_spUi->pHelpSearchTree->model());
    if (nullptr != pProxy)
    {
      QStandardItemModel* pModel = dynamic_cast<QStandardItemModel*>(pProxy->sourceModel());
      if (nullptr != pModel)
      {
        QStandardItem* pRoot = pModel->invisibleRootItem();
        if (nullptr != pRoot)
        {
          // clear model
          pModel->removeRows(0, pRoot->rowCount(), pModel->indexFromItem(pRoot));
          m_spUi->pHelpSearchTree->setUpdatesEnabled(false);
          auto map = spFactory->HelpMap();
          for (const auto& pair : map)
          {
            QStringList vsCategorySplit = pair.first.split("/");
            InitTreeBranch(pRoot, vsCategorySplit, pair.second);
          }
          m_spUi->pHelpSearchTree->setUpdatesEnabled(true);
        }
      }
    }
  }
  m_spUi->pHelpSearchTree->expandAll();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::InitTreeBranch(QStandardItem* pParent, const QStringList& vsData,
                                  const QString& sPath)
{
  if (nullptr != pParent && vsData.size() > 0)
  {
    QStandardItem* pChild = nullptr;
    for (qint32 i = 0; pParent->rowCount() > i; ++i)
    {
      if (pParent->child(i)->data(Qt::DisplayRole).toString() == vsData.first())
      {
        pChild = pParent->child(i);
      }
    }
    if (nullptr == pChild)
    {
      pChild = new QStandardItem(vsData.first());
      pChild->setData(sPath);
      pParent->appendRow(pChild);
    }
    InitTreeBranch(pChild, vsData.mid(1), sPath);
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::ShowHelp(const QString sKey)
{
  if (auto spFactory = m_wpHelpFactory.lock())
  {
    InitTree();
    m_spUi->pHtmlBrowser->setSource(QUrl::fromLocalFile(spFactory->GetHelp(sKey)));
    m_spUi->pHtmlBrowser->clearHistory();
    m_spUi->BackButton->setEnabled(m_spUi->pHtmlBrowser->isBackwardAvailable());
    m_spUi->ForardButton->setEnabled(m_spUi->pHtmlBrowser->isForwardAvailable());
    m_spUi->pHtmlBrowserBox->show();
  }
}

//----------------------------------------------------------------------------------------
//
CHelpButtonOverlay::CHelpButtonOverlay(QWidget* pParent) :
  COverlayButton(QStringLiteral("HelpButtonOverlay"),
                 QStringLiteral("HelpButton"),
                 QStringLiteral("Open help interface"),
                 QStringLiteral("Help"), pParent)
{

}

//----------------------------------------------------------------------------------------
//
CHelpButtonOverlay::~CHelpButtonOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CHelpButtonOverlay::Resize()
{
  QSize parentSize = m_pTargetWidget->size();
  move(parentSize.width() - 50 - sizeHint().width(), 50);
}
