#include "SettingsScreen.h"
#include "Application.h"
#include "Settings.h"
#include "WindowContext.h"
#include "ui_SettingsScreen.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QScreen>
#include <QSize>
#include <cassert>
#include <map>

namespace  {
  std::map<QString, QSize> possibleDimensionsMap =
  {
    { "16:10 1920x1200", QSize(1920, 1200)},
    { "16:10 1680x1050", QSize(1680, 1050)},
    { "16:10 1440x900" , QSize(1440, 900)},
    { "16:10 1280x800" , QSize(1280, 800)},
    { "16:9 2560x1440" , QSize(2560, 1440)},
    { "16:9 1920x1080" , QSize(1920, 1080)},
    { "16:9 1366x768"  , QSize(1366, 768)},
    { "16:9 1280x720"  , QSize(1280, 720)},
    { "4:3 1024x768"   , QSize(1024, 768)},
    { "4:3 800x600"    , QSize(800, 600)},
    { "4:3 640x480"    , QSize(640, 480)}
  };
}

//----------------------------------------------------------------------------------------
//
CSettingsScreen::CSettingsScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                 QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CSettingsScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CSettingsScreen::~CSettingsScreen()
{

}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Initialize()
{
  m_bInitialized = false;

  m_spSettings = CApplication::Instance()->Settings();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Load()
{
  qint32 iThisScreen = QApplication::desktop()->screenNumber(this);
  QScreen* pThisScreen = QApplication::screens()[iThisScreen];
  assert(nullptr != pThisScreen);
  if (nullptr == pThisScreen) { return; }
  QSize screenSize = pThisScreen->availableSize();

  QSize currentResolution = m_spSettings->Resolution();

  // block signals to prevent settings change
  m_spUi->pFolderLineEdit->blockSignals(true);
  m_spUi->pResolutionComboBox->blockSignals(true);

  // set lineedit
  m_spUi->pFolderLineEdit->setText(m_spSettings->ContentFolder());

  // find available screen dimensions
  qint32 iIndex = 0;
  bool bFoundResolution = false;
  for (auto it = possibleDimensionsMap.begin(); possibleDimensionsMap.end() != it; ++it)
  {
    if (it->second.width() <= screenSize.width() &&
        it->second.height() <= screenSize.height())
    {
      QString sPrefix = "";
      if (it->second.width() == screenSize.width() &&
          it->second.height() == screenSize.height())
      {
        sPrefix = "*";
      }
      m_spUi->pResolutionComboBox->addItem(sPrefix + it->first, it->second);

      // set start Value
      if (it->second == currentResolution)
      {
        bFoundResolution = true;
        m_spUi->pResolutionComboBox->setCurrentIndex(iIndex);
      }

      ++iIndex;
    }
  }

  // no default dimension -> add custom dimension
  if (!bFoundResolution)
  {
    m_spUi->pResolutionComboBox->addItem(QString("Custom %L1x%L2")
                                         .arg(currentResolution.width())
                                         .arg(currentResolution.height()),
                                         currentResolution);
    m_spUi->pResolutionComboBox->setCurrentIndex(m_spUi->pResolutionComboBox->count() - 1);
  }

  // unblock signals
  m_spUi->pResolutionComboBox->blockSignals(false);
  m_spUi->pFolderLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Unload()
{
  m_spUi->pResolutionComboBox->blockSignals(true);
  m_spUi->pResolutionComboBox->clear();
  m_spUi->pResolutionComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pResolutionComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  QSize size = m_spUi->pResolutionComboBox->itemData(iIndex, Qt::UserRole).toSize();
  m_spSettings->SetResolution(size);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pFolderLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  QFileInfo path(m_spUi->pFolderLineEdit->text());
  if (!path.exists()) { return; }
  m_spSettings->SetContentFolder(path.canonicalFilePath());
  m_spUi->pFolderLineEdit->setText(path.canonicalFilePath());
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pBrowseButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  QString dir =
      QFileDialog::getExistingDirectory(this, tr("Open Directory"),
        m_spSettings->ContentFolder(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  QFileInfo path(dir);
  if (!path.exists()) { return; }
  const QString sResultingPath = path.canonicalFilePath();
  m_spUi->pFolderLineEdit->setText(sResultingPath);
  m_spSettings->SetContentFolder(sResultingPath);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pBackButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}
