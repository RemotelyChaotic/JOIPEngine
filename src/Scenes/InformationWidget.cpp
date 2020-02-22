#include "InformationWidget.h"
#include "Application.h"
#include "FlowLayout.h"
#include "Settings.h"
#include "Backend/Resource.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_InformationWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPointer>
#include <QPropertyAnimation>

namespace  {
  const qint32 c_iIconWidth = 96;
  const qint32 c_iIconHeight = 96;
  const qint32 c_iHeaderFooterIconWidth = 64;
  const qint32 c_iHeaderFooterIconHeight = 64;
  const qint32 c_iFixedIconWidth = 48;
  const qint32 c_iFixedIconHeight = 48;
  const char* c_sMapKeyProperty = "sName";

  const QString c_sQuitIcon = "~|quit";
  const QString c_sSkipIcon = "~|skip";
}

CInformationWidget::CInformationWidget(QWidget *parent) :
  QWidget(parent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CInformationWidget>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_pExitWidget(nullptr),
  m_pSkipWidget(nullptr),
  m_iconMap(),
  m_skipTimer(),
  m_bSkippable(false),
  m_iconMask()
{
  m_spUi->setupUi(this);
  m_spUi->pIcons->setLayout(new CFlowLayout(m_spUi->pIcons, 10, 10, 10));

  m_pExitWidget = CreateHeaderIcon(m_spUi->pHeader, c_sQuitIcon, "ExitButton");
  m_pSkipWidget = CreateHeaderIcon(m_spUi->pFooter, c_sSkipIcon, "PlayButton");

  m_iconMask = QPixmap("://resources/img/IconMask.svg")
      .scaled(c_iIconWidth, c_iIconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
      .mask();

  QGraphicsOpacityEffect* pOpacity = new QGraphicsOpacityEffect(m_pSkipWidget);
  pOpacity->setOpacity(0.00);
  m_pSkipWidget->setGraphicsEffect(pOpacity);
  m_pSkipAnimation = new QPropertyAnimation(pOpacity, "opacity", m_pSkipWidget);
  m_pSkipAnimation->setDuration(200);

  connect(&m_skipTimer, &QTimer::timeout, this, &CInformationWidget::SlotSkipTimeout);
}

CInformationWidget::~CInformationWidget()
{
  SlotSkipTimeout();
  m_pSkipAnimation->stop();
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::Initialize()
{
  m_bInitialized = false;

  m_skipTimer.setSingleShot(true);
  SlotSkipTimeout();

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotHideIcon(QString sIconIdentifier)
{
  if (!m_bInitialized) { return; }

  if (sIconIdentifier.isNull() || sIconIdentifier.isEmpty())
  {
    RemoveAllIcons();
  }
  else
  {
    RemoveIcon(sIconIdentifier);
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotShowIcon(tspResource spResource)
{
  if (!m_bInitialized) { return; }
  if (nullptr == spResource) { return; }

  spResource->m_rwLock.lockForRead();
  EResourceType resourceType = spResource->m_type;
  QString sName = spResource->m_sName;
  spResource->m_rwLock.unlock();

  if (resourceType._to_integral() != EResourceType::eSound &&
      resourceType._to_integral() != EResourceType::eOther)
  {
    AddIcon(sName, spResource);
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotShowSkipIcon(qint32 iTimeS)
{
  m_bSkippable = true;
  m_skipTimer.setInterval(iTimeS * 1000);
  m_skipTimer.start();
  if (nullptr != m_pSkipWidget)
  {
    QGraphicsOpacityEffect* pOpacity =
      dynamic_cast<QGraphicsOpacityEffect*>(m_pSkipWidget->graphicsEffect());

    m_pSkipAnimation->stop();
    m_pSkipAnimation->setStartValue((nullptr != pOpacity) ? pOpacity->opacity() : 0.0);
    m_pSkipAnimation->setEndValue(1.0);
    m_pSkipWidget->show();
    m_pSkipAnimation->start();
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotSkipTimeout()
{
  m_bSkippable = false;
  m_skipTimer.stop();
  if (nullptr != m_pSkipWidget)
  {
    QGraphicsOpacityEffect* pOpacity =
      dynamic_cast<QGraphicsOpacityEffect*>(m_pSkipWidget->graphicsEffect());

    m_pSkipAnimation->stop();
    m_pSkipAnimation->setStartValue((nullptr != pOpacity) ? pOpacity->opacity() : 1.0);
    m_pSkipAnimation->setEndValue(0.0);
    m_pSkipAnimation->start();
  }
}

//----------------------------------------------------------------------------------------
//
bool CInformationWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if(pObject == nullptr || pEvent == nullptr) { return false; }

  if (pEvent->type() == QEvent::MouseButtonRelease)
  {
    const QString sName = pObject->property(c_sMapKeyProperty).toString();

    if (c_sQuitIcon == sName)
    {
      emit SignalQuit();
    }
    else if (m_bSkippable || c_sSkipIcon == sName)
    {
      emit SignalWaitSkipped();
      SlotSkipTimeout();
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotFadeoutAnimationFinished()
{
  QPropertyAnimation* pFadeOutAnim = dynamic_cast<QPropertyAnimation*>(sender());
  if (nullptr != pFadeOutAnim)
  {
    QWidget* pWidget = dynamic_cast<QWidget*>(pFadeOutAnim->parent());
    if (nullptr != pWidget)
    {
      m_spUi->pIcons->layout()->removeWidget(pWidget);
      pWidget->deleteLater();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotSkipAnimationFinished()
{
  if (m_bSkippable)
  {
    m_pSkipWidget->hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::AddIcon(const QString& sName, const tspResource& spResource)
{
  CFlowLayout* pFlowLayout = dynamic_cast<CFlowLayout*>(m_spUi->pIcons->layout());
  if (nullptr != pFlowLayout)
  {
    QWidget* pRoot = new QWidget(m_spUi->pIcons);
    pRoot->setFixedSize(c_iIconWidth, c_iIconHeight);
    pRoot->setStyleSheet("background-color: transparent;");

    QLabel* pBackground = new QLabel("", pRoot);
    pBackground->setFixedSize(c_iIconWidth, c_iIconHeight);
    //pBackground->setPixmap(QPixmap("://resources/img/IconBg.svg"));
    pBackground->setObjectName("IconBackground");
    pBackground->setScaledContents(true);
    pBackground->setStyleSheet("background-color: transparent;");

    CResourceDisplayWidget* pResourceDisplay = new CResourceDisplayWidget(pRoot);
    pResourceDisplay->setFixedSize(c_iIconWidth, c_iIconHeight);
    pResourceDisplay->LoadResource(spResource);
    pResourceDisplay->setMask(m_iconMask);
    pBackground->setStyleSheet("background-color: transparent;");

    pRoot->installEventFilter(this);
    pRoot->setProperty(c_sMapKeyProperty, sName);

    QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pRoot);
    pShadow->setBlurRadius(5);
    pShadow->setXOffset(5);
    pShadow->setYOffset(5);
    pShadow->setColor(Qt::black);
    pRoot->setGraphicsEffect(pShadow);

    pFlowLayout->addWidget(pRoot);
    m_iconMap.insert({sName, QPointer<QWidget>(pRoot)});
  }
}

//----------------------------------------------------------------------------------------
//
QWidget* CInformationWidget::CreateHeaderIcon(QWidget* pParent, const QString& sName,
                                              const QString sObjName)
{
  QLayout* pLayout = pParent->layout();
  if (nullptr != pLayout)
  {
    QWidget* pRoot = new QWidget(pParent);
    pRoot->setFixedSize(c_iIconWidth, c_iIconHeight);
    pRoot->setStyleSheet("background-color: transparent;");

    QLabel* pBackground = new QLabel("", pRoot);
    pBackground->setFixedSize(c_iHeaderFooterIconWidth, c_iHeaderFooterIconHeight);
    //pBackground->setPixmap(QPixmap("://resources/img/IconBg.svg"));
    pBackground->setObjectName("IconBackground");
    pBackground->setScaledContents(true);
    pRoot->setStyleSheet("background-color: transparent;");

    QLabel* pIcon = new QLabel("", pRoot);
    pIcon->setFixedSize(c_iFixedIconWidth, c_iFixedIconHeight);
    pIcon->setGeometry((c_iHeaderFooterIconWidth - c_iFixedIconWidth) / 2,
                       (c_iHeaderFooterIconHeight - c_iFixedIconHeight) / 2,
                       c_iFixedIconWidth, c_iFixedIconHeight);
    pIcon->setObjectName(sObjName);
    pIcon->setScaledContents(true);
    pIcon->setMask(m_iconMask);
    pRoot->setStyleSheet("background-color: transparent;");

    pRoot->installEventFilter(this);
    pRoot->setProperty(c_sMapKeyProperty, sName);

    QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pRoot);
    pShadow->setBlurRadius(5);
    pShadow->setXOffset(5);
    pShadow->setYOffset(5);
    pShadow->setColor(Qt::black);
    pRoot->setGraphicsEffect(pShadow);

    pLayout->addWidget(pRoot);

    return pRoot;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::RemoveAllIcons()
{
  while(0 < m_iconMap.size())
  {
    RemoveIcon(m_iconMap.begin()->first);
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::RemoveIcon(const QString& sName)
{
  auto it = m_iconMap.find(sName);
  if (m_iconMap.end() != it)
  {
    auto pWidget = it->second;
    m_iconMap.erase(it);

    QGraphicsOpacityEffect* pOpacity = new QGraphicsOpacityEffect(pWidget);
    pOpacity->setOpacity(1.00);
    pWidget->setGraphicsEffect(pOpacity);
    QPropertyAnimation* pFadeOutAnimation = new QPropertyAnimation(pOpacity, "opacity", pWidget);
    pFadeOutAnimation->setStartValue(1);
    pFadeOutAnimation->setEndValue(0);
    pFadeOutAnimation->setDuration(500);
    connect(pFadeOutAnimation, &QPropertyAnimation::finished,
            this, &CInformationWidget::SlotFadeoutAnimationFinished);
    pFadeOutAnimation->start();
  }
}
