#include "SettingsScreen.h"
#include "Application.h"
#include "Settings.h"
#include "Style.h"
#include "WindowContext.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"
#include "ui_SettingsScreen.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequenceEdit>
#include <QScreen>
#include <QSize>
#include <cassert>
#include <map>

namespace  {
  const double c_dSliderScaling = 10000;

  const QString c_sFullscreenHelpId = "Settings/Fullscreen";
  const QString c_sDataFolderHelpId = "Settings/DataFolder";
  const QString c_sFontHelpId = "Settings/Font";
  const QString c_sStyleHelpId = "Settings/Style";
  const QString c_sResolutionHelpId = "Settings/Resolution";
  const QString c_sMuteHelpId = "Settings/Mute";
  const QString c_sVolumeHelpId = "Settings/Volume";
  const QString c_sCancelHelpId = "MainScreen/Cancel";

  const char* c_sPropertyKeySequence = "KeyBinding";

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

  CApplication* pApp = CApplication::Instance();
  m_spSettings = pApp->Settings();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pFullScreenContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sFullscreenHelpId);
    wpHelpFactory->RegisterHelp(c_sFullscreenHelpId, ":/resources/help/settings/fullscreen_setting_help.html");
    m_spUi->pContentContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDataFolderHelpId);
    wpHelpFactory->RegisterHelp(c_sDataFolderHelpId, ":/resources/help/settings/datafolder_setting_help.html");
    m_spUi->pFontContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sFontHelpId);
    wpHelpFactory->RegisterHelp(c_sFontHelpId, ":/resources/help/settings/font_setting_help.html");
    m_spUi->pStyleContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sStyleHelpId);
    wpHelpFactory->RegisterHelp(c_sStyleHelpId, ":/resources/help/settings/style_setting_help.html");
    m_spUi->pResolutionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sResolutionHelpId);
    wpHelpFactory->RegisterHelp(c_sResolutionHelpId, ":/resources/help/settings/resolution_setting_help.html");
    m_spUi->pMuteContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sMuteHelpId);
    wpHelpFactory->RegisterHelp(c_sMuteHelpId, ":/resources/help/settings/mute_setting_help.html");
    m_spUi->pVolumeContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sVolumeHelpId);
    wpHelpFactory->RegisterHelp(c_sVolumeHelpId, ":/resources/help/settings/volume_setting_help.html");
    m_spUi->pBackButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/player/cancel_button_help.html");
  }

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
  m_spUi->pFullscreenCheckBox->blockSignals(true);
  m_spUi->pFolderLineEdit->blockSignals(true);
  m_spUi->pFontComboBox->blockSignals(true);
  m_spUi->pStyleComboBox->blockSignals(true);
  m_spUi->pResolutionComboBox->blockSignals(true);
  m_spUi->pMuteCheckBox->blockSignals(true);
  m_spUi->pVolumeSlider->blockSignals(true);

  // set fullscreen
  m_spUi->pFullscreenCheckBox->setCheckState(m_spSettings->Fullscreen() ? Qt::Checked : Qt::Unchecked);

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

  // set font
  QFont font(m_spSettings->Font());
  m_spUi->pFontComboBox->setCurrentFont(font);

  // set key bindings
  QStringList vsKeyBindings = m_spSettings->KeyBindings();
  for (const QString& sKeyBinding : qAsConst(vsKeyBindings))
  {
    QWidget* pKeyBindingContainer = new QWidget(m_spUi->pInputContainer);
    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);
    pKeyBindingContainer->setLayout(pLayout);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);

    QLabel* pKeyLabel = new QLabel(sKeyBinding, pKeyBindingContainer);
    pKeyLabel->setSizePolicy(sizePolicy);
    pLayout->addWidget(pKeyLabel);

    QKeySequenceEdit* pKeySequenceEdit = new QKeySequenceEdit(m_spSettings->KeyBinding(sKeyBinding),
                                                              pKeyBindingContainer);
    pKeySequenceEdit->setSizePolicy(sizePolicy);
    pKeySequenceEdit->setProperty(c_sPropertyKeySequence, sKeyBinding);
    pKeySequenceEdit->setEnabled(CSettings::IsAllowedToOverwriteKeyBinding(sKeyBinding));
    connect(pKeySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &CSettingsScreen::SlotKeySequenceChanged);
    pLayout->addWidget(pKeySequenceEdit);

    m_spUi->pInputContainer->layout()->addWidget(pKeyBindingContainer);
  }

  // setStyle
  m_spUi->pStyleComboBox->addItems(joip_style::AvailableStyles());
  m_spUi->pStyleComboBox->setCurrentText(m_spSettings->Style());

  // set volume
  m_spUi->pMuteCheckBox->setCheckState(m_spSettings->Muted() ? Qt::Checked : Qt::Unchecked);
  m_spUi->pVolumeSlider->setValue(static_cast<qint32>(m_spSettings->Volume() * c_dSliderScaling));

  // set lineedit
  m_spUi->pFolderLineEdit->setText(m_spSettings->ContentFolder());

  // unblock signals
  m_spUi->pFullscreenCheckBox->blockSignals(false);
  m_spUi->pResolutionComboBox->blockSignals(false);
  m_spUi->pFontComboBox->blockSignals(false);
  m_spUi->pStyleComboBox->blockSignals(false);
  m_spUi->pFolderLineEdit->blockSignals(false);
  m_spUi->pMuteCheckBox->blockSignals(false);
  m_spUi->pVolumeSlider->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Unload()
{
  m_spUi->pResolutionComboBox->blockSignals(true);
  m_spUi->pStyleComboBox->blockSignals(true);

  m_spUi->pResolutionComboBox->clear();
  m_spUi->pStyleComboBox->clear();

  m_spUi->pResolutionComboBox->blockSignals(false);
  m_spUi->pStyleComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pFullscreenCheckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetFullscreen(iState == Qt::Checked);
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
void CSettingsScreen::on_pFontComboBox_currentFontChanged(const QFont& font)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetFont(font.family());
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pStyleComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetStyle(m_spUi->pStyleComboBox->itemText(iIndex));
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
void CSettingsScreen::on_pMuteCheckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetMuted(iState == Qt::Checked);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pVolumeSlider_sliderReleased()
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  double dVolume = static_cast<double>(m_spUi->pVolumeSlider->value()) / c_dSliderScaling;
  m_spSettings->SetVolume(dVolume);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pBackButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::SlotKeySequenceChanged(const QKeySequence& keySequence)
{
  WIDGET_INITIALIZED_GUARD
  QString sKeyBinding = sender()->property(c_sPropertyKeySequence).toString();
  m_spSettings->SetKeyBinding(keySequence, sKeyBinding);
}
