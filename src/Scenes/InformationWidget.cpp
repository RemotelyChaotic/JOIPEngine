#include "InformationWidget.h"
#include "Application.h"
#include "FlowLayout.h"
#include "Settings.h"
#include "Backend/Resource.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_InformationWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPointer>

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
  m_iconMask()
{
  m_spUi->setupUi(this);
  m_spUi->pIcons->setLayout(new CFlowLayout(m_spUi->pIcons, 10, 10, 10));

  m_pExitWidget = CreateHeaderIcon(m_spUi->pHeader, c_sQuitIcon, "://resources/img/ButtonExit.png");
  m_pSkipWidget = CreateHeaderIcon(m_spUi->pFooter, c_sSkipIcon, "://resources/img/ButtonPlay.png");

  m_iconMask = QPixmap("://resources/img/IconMask.svg")
      .scaled(c_iIconWidth, c_iIconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
      .mask();

  connect(&m_skipTimer, &QTimer::timeout, this, &CInformationWidget::SlotSkipTimeout);
}

CInformationWidget::~CInformationWidget()
{
  SlotSkipTimeout();
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
  m_skipTimer.setInterval(iTimeS * 1000);
  m_skipTimer.start();
  if (nullptr != m_pSkipWidget)
  {
    m_pSkipWidget->show();
  }
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::SlotSkipTimeout()
{
  m_skipTimer.stop();
  if (nullptr != m_pSkipWidget)
  {
    m_pSkipWidget->hide();
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
    else if (c_sSkipIcon == sName)
    {
      emit SignalWaitSkipped();
      SlotSkipTimeout();
    }
  }

  return false;
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
    pBackground->setPixmap(QPixmap("://resources/img/IconBg.svg"));
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
                                              const QString sPath)
{
  QLayout* pLayout = pParent->layout();
  if (nullptr != pLayout)
  {
    QWidget* pRoot = new QWidget(pParent);
    pRoot->setFixedSize(c_iIconWidth, c_iIconHeight);
    pRoot->setStyleSheet("background-color: transparent;");

    QLabel* pBackground = new QLabel("", pRoot);
    pBackground->setFixedSize(c_iHeaderFooterIconWidth, c_iHeaderFooterIconHeight);
    pBackground->setPixmap(QPixmap("://resources/img/IconBg.svg"));
    pBackground->setScaledContents(true);
    pRoot->setStyleSheet("background-color: transparent;");

    QLabel* pIcon = new QLabel("", pRoot);
    pIcon->setFixedSize(c_iFixedIconWidth, c_iFixedIconHeight);
    pIcon->setGeometry((c_iHeaderFooterIconWidth - c_iFixedIconWidth) / 2,
                       (c_iHeaderFooterIconHeight - c_iFixedIconHeight) / 2,
                       c_iFixedIconWidth, c_iFixedIconHeight);
    pIcon->setPixmap(QPixmap(sPath));
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
    m_spUi->pIcons->layout()->removeWidget(pWidget);
    delete pWidget;
  }
}
