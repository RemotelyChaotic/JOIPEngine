#include "Splash.h"
#include "Style.h"
#include "Updater.h"

#include "Widgets/TitleLabel.h"

#if defined(Q_OS_WIN)
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#include <QDirIterator>
#include <QGuiApplication>
#include <QGridLayout>
#include <QPainter>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QStyle>

#include <chrono>

namespace
{
  // windows convention is 620 x 300 for spash screens
  const QSize c_SplashSize = QSize(620, 300);
  const qint32 c_iMessageHeight = 20;
  const qint32 c_iAnimationTimerMs = 1000;
  const qint32 c_iNbrMaxDots = 3;
}

//----------------------------------------------------------------------------------------
//
CSplash::CSplash(const SSettingsData& settings, QWidget *parent) :
  QWidget{parent},
  m_settings(settings),
  m_pUpdater(new CUpdater(&m_settings, this)),
  m_backgroundPixmap()
{
  setWindowFlag(Qt::FramelessWindowHint, true);
  setWindowIcon(QIcon(":/resouces/Icon.ico")); // TODO: replce with updater icon

#if defined(Q_OS_WIN)
  m_pTaskButton = new QWinTaskbarButton(this);
  m_pTaskButton->setWindow(windowHandle());
  m_pTaskProgress = m_pTaskButton->progress();
  m_pTaskProgress->setVisible(false);
#endif

  QPoint globalCursorPos = QCursor::pos();
  QRect availableGeometry =
      QGuiApplication::screenAt(globalCursorPos)->geometry();

  setGeometry(
      QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
                          c_SplashSize, availableGeometry));

  QDirIterator iter(joip_style::StyleFolder() + "/" + settings.sStyle,
                    QStringList() << "Background.png",
                    QDir::NoDotAndDotDot | QDir::Files,
                    QDirIterator::Subdirectories);
  if (iter.hasNext())
  {
    m_backgroundPixmap = QPixmap(iter.next());
  }

  QGridLayout* pLayout = new QGridLayout(this);
  pLayout->setMargin(0);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->setSpacing(0);

  QWidget* pContainer = new QWidget(this);
  QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(pContainer->sizePolicy().hasHeightForWidth());
  pContainer->setSizePolicy(sizePolicy);
  pContainer->setFixedHeight(c_SplashSize.height() - c_iMessageHeight);
  pLayout->addWidget(pContainer, 0, 0, 1, 1);

  QGridLayout* pInner = new QGridLayout(pContainer);
  pInner->setMargin(0);
  pInner->setContentsMargins(0, 0, 0, 0);
  pInner->setSpacing(0);

  m_pSplashText = new CTitleLabel(pContainer);
  SlotReloadText(true);
  m_pSplashText->setObjectName(QString::fromUtf8("pSplashText"));
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_pSplashText->sizePolicy().hasHeightForWidth());
  m_pSplashText->setSizePolicy(sizePolicy);
  m_pSplashText->setMinimumSize(QSize(1, 0));
  m_pSplashText->setAlignment(Qt::AlignCenter);
  pInner->addWidget(m_pSplashText, 0, 0, 1, 1);

  connect(m_pSplashText, &CTitleLabel::SignalAnimationFinished, this,
          &CSplash::SlotAnimationFinished);

  m_pMessageBar = new QProgressBar(this);
  m_pMessageBar->setRange(0, 0);
  m_pMessageBar->setValue(0);
  m_pMessageBar->setFormat(QString(""));
  m_pMessageBar->setObjectName(QString::fromUtf8("pMessageLabel"));
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_pMessageBar->sizePolicy().hasHeightForWidth());
  m_pMessageBar->setSizePolicy(sizePolicy);
  m_pMessageBar->setMinimumSize(QSize(1, 0));
  m_pMessageBar->setFixedHeight(c_iMessageHeight);
  m_pMessageBar->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(m_pMessageBar, 1, 0, 1, 1);

  m_pMessageText = new QLabel(this);
  m_pMessageText->setText(tr("Initializing..."));
  m_pMessageText->setObjectName(QString::fromUtf8("pMessageLabel"));
  sizePolicy = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_pMessageText->sizePolicy().hasHeightForWidth());
  m_pMessageText->setSizePolicy(sizePolicy);
  m_pMessageText->setFixedSize(width(), c_iMessageHeight);
  m_pMessageText->setAlignment(Qt::AlignCenter);
  QFont font = m_pMessageBar->font();
  font.setPixelSize(12);
  m_pMessageText->setFont(font);
  m_pMessageText->setStyleSheet("QLabel {"
                               "  color: silver;"
                               "}");
  m_pMessageText->move(0, height() - c_iMessageHeight);
  m_pMessageText->raise();

  m_pPopupBox = new QGroupBox(this);
  m_pPopupBox->setObjectName(QString::fromUtf8("pPopupBox"));
  m_pPopupBox->setCheckable(false);
  m_pPopupBox->setTitle(QString());
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_pPopupBox->sizePolicy().hasHeightForWidth());
  m_pPopupBox->setSizePolicy(sizePolicy);
  m_pPopupBox->resize(width() / 2, 100);
  m_pPopupBox->move(width() / 4, height() - c_iMessageHeight/2 - m_pPopupBox->height());
  m_pPopupBox->raise();
  m_pPopupBox->setVisible(!m_settings.bAutoUpdateFound);

  QGridLayout* pGridQuestion = new QGridLayout(m_pPopupBox);

  QLabel* pTitle = new QLabel(m_pPopupBox);
  pTitle->setText(tr("Do you want to enable\nauto updates going forward?"));
  pTitle->setObjectName(QString::fromUtf8("pTitle"));
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(pTitle->sizePolicy().hasHeightForWidth());
  pTitle->setSizePolicy(sizePolicy);
  pTitle->setMinimumSize(QSize(1, 0));
  pTitle->setAlignment(Qt::AlignCenter);
  pGridQuestion->addWidget(pTitle, 0, 0, 1, 3);

  QPushButton* pYes = new QPushButton(m_pPopupBox);
  pYes->setText(tr("Yes"));
  pYes->setObjectName(QString::fromUtf8("pMessageLabel"));
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(pYes->sizePolicy().hasHeightForWidth());
  pYes->setSizePolicy(sizePolicy);
  pGridQuestion->addWidget(pYes, 1, 0, 1, 1);
  connect(pYes, &QPushButton::clicked, this, [this]() {
    m_settings.spSettings->setValue(c_sSettingAutoUpdate, true);
    m_settings.spSettings->sync();
    m_settings.bAutoUpdate = true;
    m_pPopupBox->hide();
    StartUpdate();
  });

  pGridQuestion->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 1, 1, 1);

  QPushButton* pNo = new QPushButton(m_pPopupBox);
  pNo->setText(tr("No"));
  pNo->setObjectName(QString::fromUtf8("pMessageLabel"));
  sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(pNo->sizePolicy().hasHeightForWidth());
  pNo->setSizePolicy(sizePolicy);
  pGridQuestion->addWidget(pNo, 1, 2, 1, 1);
  connect(pNo, &QPushButton::clicked, this, [this]() {
    m_settings.spSettings->setValue(c_sSettingAutoUpdate, false);
    m_settings.spSettings->sync();
    m_settings.bAutoUpdate = false;
    m_pPopupBox->hide();
    StartMainExe();
  });

  connect(m_pUpdater, &CUpdater::SignalMessage, this, [this](const QString& sText) {
    m_pMessageText->setText(sText);
  });
  connect(m_pUpdater, &CUpdater::SignalProgress, this, &CSplash::SlotSetProgress);
  connect(m_pUpdater, &CUpdater::SignalStartExe, this, &CSplash::StartMainExe);

  m_animationTimer.setSingleShot(true);
  m_animationTimer.setInterval(c_iAnimationTimerMs);
  connect(&m_animationTimer, &QTimer::timeout, this,
          [this] {
            SlotReloadText(false);
          });

  if (m_settings.bAutoUpdateFound)
  {
    using namespace std::chrono_literals;
    QTimer::singleShot(1s, this, [this]() {
      if (m_settings.bAutoUpdate)
      {
        StartUpdate();
      }
      else
      {
        StartMainExe();
      }
    });
  }
}

CSplash::~CSplash() = default;

//----------------------------------------------------------------------------------------
//
void CSplash::StartUpdate()
{
  m_pMessageText->setText(tr("Updating..."));
  SlotSetProgress(0, -1);

  if (m_settings.bContinueUpdate)
  {
    m_pUpdater->ContinueUpdate();
  }
  else
  {
    m_pUpdater->RunUpdate();
  }
}

//----------------------------------------------------------------------------------------
//
void CSplash::StartMainExe()
{
  m_pMessageText->setText(tr("Starting engine..."));
  SlotSetProgress(0, -1);
  QProcess proc;
  bool bOk =
      proc.startDetached(QCoreApplication::applicationDirPath() + "/../bin/JOIPEngine.exe",
                              qApp->arguments());

  if (bOk)
  {
    using namespace std::chrono_literals;
    QTimer::singleShot(2s, this, []() {
      qApp->quit();
    });
  }
  else
  {
    m_pMessageText->setText(tr("Error launching JOIPEngine.exe: %1").arg(proc.errorString()));
    SlotSetProgress(0, -1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSplash::SlotAnimationFinished()
{
  m_animationTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CSplash::SlotReloadText(bool bSkipUpdate)
{
  m_iNbrDots++;
  if (c_iNbrMaxDots + 1 <= m_iNbrDots)
  {
    m_iNbrDots = 1;
  }
  m_pSplashText->setText(tr("Launching") + QString(".").repeated(m_iNbrDots));
  if (!bSkipUpdate)
  {
    m_pSplashText->Invalidate();
  }
}

//----------------------------------------------------------------------------------------
//
void CSplash::SlotSetProgress(qint32 iCurrent, qint32 iMax)
{
  if (0 > iMax)
  {
    m_pMessageBar->setRange(0, 0);
    m_pMessageBar->setValue(0);
#if defined(Q_OS_WIN)
    m_pTaskProgress->setVisible(false);
#endif
  }
  else
  {
    m_pMessageBar->setRange(0, iMax);
    m_pMessageBar->setValue(iCurrent);
#if defined(Q_OS_WIN)
    m_pTaskProgress->setVisible(true);
    m_pTaskProgress->setRange(0, iMax);
    m_pTaskProgress->setValue(iCurrent);
#endif
  }
}

//----------------------------------------------------------------------------------------
//
void CSplash::paintEvent(QPaintEvent* pEvt)
{
  Q_UNUSED(pEvt)
  QPainter painter(this);
  QPainter::CompositionMode oldCompositionMode = painter.compositionMode();
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                             QPainter::TextAntialiasing, true);

  QBrush bgBrush;
  bgBrush.setTexture(m_backgroundPixmap);
  painter.fillRect(rect(), bgBrush);

  painter.setCompositionMode(QPainter::CompositionMode_Screen);

  painter.setBrush(m_backgroundColor);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect());

  painter.setCompositionMode(oldCompositionMode);

  QLinearGradient linGrad(0, 0, width(), 0);
  linGrad.setColorAt(0.0, QColor(0, 0, 0, 150));
  linGrad.setColorAt(0.4, QColor(0, 0, 0, 50));
  linGrad.setColorAt(0.6, QColor(0, 0, 0, 50));
  linGrad.setColorAt(1.0, QColor(0, 0, 0, 150));

  painter.setBrush(linGrad);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect());

  painter.setBrush(QColor(200,200,200,128));
  painter.setPen(Qt::NoPen);
  painter.drawRect(QRect(0, height() - c_iMessageHeight, width(), c_iMessageHeight));
}
