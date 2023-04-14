#include "SettingsScreen.h"
#include "Application.h"
#include "Settings.h"
#include "Style.h"
#include "WindowContext.h"

#include "Systems/DLJobs/DownloadJobRegistry.h"
#include "Systems/HelpFactory.h"
#include "Utils/WidgetHelpers.h"
#include "Widgets/DownloadButtonOverlay.h"
#include "Widgets/HelpOverlay.h"
#include "ui_SettingsScreen.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequenceEdit>
#include <QListView>
#include <QScreen>
#include <QSize>
#include <cassert>
#include <map>

namespace  {
  const double c_dSliderScaling = 10000;

  const QString c_sWindowModeHelpId = "Settings/WindowMode";
  const QString c_sDataFolderHelpId = "Settings/DataFolder";
  const QString c_sFontHelpId = "Settings/Font";
  const QString c_sStyleHelpId = "Settings/Style";
  const QString c_sResolutionHelpId = "Settings/Resolution";
  const QString c_sMuteHelpId = "Settings/Mute";
  const QString c_sVolumeHelpId = "Settings/Volume";
  const QString c_sOfflineHelpId = "Settings/Offline";
  const QString c_sPauseWhenInactiveHelpId = "Settings/PauseWhenInactive";
  const QString c_sPushNotificationsHelpId = "Settings/PushNotifications";
  const QString c_sHotloadStyleHelpId = "Settings/HotloadStyle";
  const QString c_sCancelHelpId = "MainScreen/Cancel";

  const char* c_sPropertyKeySequence = "KeyBinding";

  std::map<QString, QSize> c_possibleDimensionsMap =
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

  std::map<CSettings::EditorType, QString> c_editorTypeStrings =
  {
    { CSettings::eNone,     QString("None (Not Configured)")},
    { CSettings::eClassic,  QString("Classic (2 Views)")},
    { CSettings::eModern ,  QString("Modern (3 Views)")},
    { CSettings::eCompact , QString("Compact (Mobile Layout)")}
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
    m_spUi->pWindowModeContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sWindowModeHelpId);
    wpHelpFactory->RegisterHelp(c_sWindowModeHelpId, ":/resources/help/settings/window_mode_setting_help.html");
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
    m_spUi->pOfflineContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sOfflineHelpId);
    wpHelpFactory->RegisterHelp(c_sOfflineHelpId, ":/resources/help/settings/offline_setting_help.html");
    m_spUi->pPauseWhenNotActive->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sPauseWhenInactiveHelpId);
    wpHelpFactory->RegisterHelp(c_sPauseWhenInactiveHelpId, ":/resources/help/settings/pausewheninactive_setting_help.html");
    m_spUi->pShowPushNotifications->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sPushNotificationsHelpId);
    wpHelpFactory->RegisterHelp(c_sPushNotificationsHelpId, ":/resources/help/settings/push_notification_setting_help.html");
    m_spUi->pStyleHotoadContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sHotloadStyleHelpId);
    wpHelpFactory->RegisterHelp(c_sHotloadStyleHelpId, ":/resources/help/settings/hotloadstyle_setting_help.html");
    m_spUi->pBackButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/player/cancel_button_help.html");
  }

  m_pHelpButtonOverlay = window()->findChild<CHelpButtonOverlay*>();

  m_spUi->WarningIcon->hide();
  m_spUi->BrowseButton->setProperty("styleSmall", true);

  m_spUi->pEditorLayoutComboBox->clear();
  for (auto it = c_editorTypeStrings.begin(); c_editorTypeStrings.end() != it; ++it)
  {
    m_spUi->pEditorLayoutComboBox->addItem(it->second, static_cast<qint32>(it->first));
  }
  qobject_cast<QListView*>(m_spUi->pEditorLayoutComboBox->view())
      ->setRowHidden(CSettings::eNone, true);

  // Dynamically create job settings. The jobs define the behavior of these.
  for (const auto& itJob : CDownloadJobFactory::Instance().GetHostSettingMap())
  {
    QWidget* pWidget = CDownloadJobFactory::Instance().GetJobWidget(m_spUi->pAdvancedContent,
                                                                    itJob.second.m_sClassType);
    if (nullptr != pWidget)
    {
      QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(m_spUi->pAdvancedContent->layout());
      if (nullptr != pLayout)
      {
        pLayout->insertWidget(pLayout->count()-1, pWidget);
      }
    }
  }

  m_spUi->tabWidget->setCurrentIndex(0);

#if defined (Q_OS_ANDROID)
  // disable settings pages that do not make sense on mobile
  m_spUi->tabWidget->setTabEnabled(0, false);
  m_spUi->tabWidget->setTabEnabled(3, false);
  m_spUi->tabWidget->setTabEnabled(5, false);
  m_spUi->tabWidget->setCurrentIndex(1);
  m_spUi->pWindowModeContainer->hide();

  widget_helpers::RetainSizeAndHide(m_spUi->pBackButton);
#endif

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Load()
{
  // ensure settings tab-bar is visible
  m_spUi->pTopSpacer->changeSize(20, m_pHelpButtonOverlay->y() + m_pHelpButtonOverlay->height() + 10,
                                 QSizePolicy::Fixed, QSizePolicy::Fixed);
  layout()->invalidate();

  emit m_spWindowContext->SignalSetDownloadButtonVisible(false);

#ifndef Q_OS_ANDROID
  qint32 iThisScreen = QApplication::desktop()->screenNumber(this);
  auto vpScreens = QApplication::screens();
  QScreen* pThisScreen = vpScreens[iThisScreen];
#else
  QScreen* pThisScreen = QApplication::screens()[0];
#endif

  assert(nullptr != pThisScreen);
  if (nullptr == pThisScreen) { return; }
  QSize screenSize = pThisScreen->availableSize();

  QSize currentResolution = m_spSettings->Resolution();

  // block signals to prevent settings change
  m_spUi->pWindowModeComboBox->blockSignals(true);
  m_spUi->pFolderLineEdit->blockSignals(true);
  m_spUi->pFontComboBox->blockSignals(true);
  m_spUi->pStyleComboBox->blockSignals(true);
  m_spUi->pEditorLayoutComboBox->blockSignals(true);
  m_spUi->pResolutionComboBox->blockSignals(true);
  m_spUi->pMuteCheckBox->blockSignals(true);
  m_spUi->pVolumeSlider->blockSignals(true);
  m_spUi->pOfflineModeCheckBox->blockSignals(true);
  m_spUi->pPauseWhenNotActiveCheckBox->blockSignals(true);
  m_spUi->pShowPushNotificationsCeckBox->blockSignals(true);
  m_spUi->pStyleHotoadCheckBox->blockSignals(true);

  // set fullscreen
  m_spUi->pWindowModeComboBox->setCurrentIndex(static_cast<qint32>(m_spSettings->GetWindowMode()));

  // find available screen dimensions
  m_spUi->pResolutionComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  qint32 iIndex = 0;
  bool bFoundResolution = false;
  for (auto it = c_possibleDimensionsMap.begin(); c_possibleDimensionsMap.end() != it; ++it)
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

  m_spUi->pResolutionContainer->layout()->invalidate();

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
    pKeyLabel->setMinimumSize({1, 0});
    pLayout->addWidget(pKeyLabel);

    QKeySequenceEdit* pKeySequenceEdit = new QKeySequenceEdit(m_spSettings->keyBinding(sKeyBinding),
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

  // editor layout
  iIndex = m_spUi->pEditorLayoutComboBox->findData(
        static_cast<qint32>(m_spSettings->PreferedEditorLayout()));
  m_spUi->pEditorLayoutComboBox->setCurrentIndex(iIndex);

  // set volume
  m_spUi->pMuteCheckBox->setCheckState(m_spSettings->Muted() ? Qt::Checked : Qt::Unchecked);
  m_spUi->pVolumeSlider->setValue(static_cast<qint32>(m_spSettings->Volume() * c_dSliderScaling));

  // set lineedit
  m_spUi->pFolderLineEdit->setText(m_spSettings->ContentFolder());

  // set offline mode
  m_spUi->pOfflineModeCheckBox->setChecked(m_spSettings->Offline());

  // set pause when app is inactive
  m_spUi->pPauseWhenNotActiveCheckBox->setChecked(m_spSettings->PauseWhenInactive());

  // push notifications
  m_spUi->pShowPushNotificationsCeckBox->setChecked(m_spSettings->PushNotifications());

  // hot-loading of style
  m_spUi->pStyleHotoadCheckBox->setChecked(m_spSettings->StyleHotLoad());

  // unblock signals
  m_spUi->pWindowModeComboBox->blockSignals(false);
  m_spUi->pResolutionComboBox->blockSignals(false);
  m_spUi->pFontComboBox->blockSignals(false);
  m_spUi->pStyleComboBox->blockSignals(false);
  m_spUi->pEditorLayoutComboBox->blockSignals(false);
  m_spUi->pFolderLineEdit->blockSignals(false);
  m_spUi->pMuteCheckBox->blockSignals(false);
  m_spUi->pVolumeSlider->blockSignals(false);
  m_spUi->pOfflineModeCheckBox->blockSignals(false);
  m_spUi->pPauseWhenNotActiveCheckBox->blockSignals(false);
  m_spUi->pShowPushNotificationsCeckBox->blockSignals(false);
  m_spUi->pStyleHotoadCheckBox->blockSignals(false);
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

  emit UnloadFinished();
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pWindowModeComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetWindowMode(CSettings::WindowMode(iIndex));
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
  m_spUi->WarningIcon->show();
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pEditorLayoutComboBox_currentIndexChanged(qint32 iIndex)
{
  Q_UNUSED(iIndex)
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetPreferedEditorLayout(
        static_cast<CSettings::EditorType>(
          m_spUi->pEditorLayoutComboBox->currentData().toInt()));
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
void CSettingsScreen::on_BrowseButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  QPointer<CSettingsScreen> pThis(this);
  QString dir =
      widget_helpers::GetExistingDirectory(this, tr("Open Directory"),
        m_spSettings->ContentFolder(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (nullptr == pThis) { return; }

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
void CSettingsScreen::on_pPauseWhenNotActiveCheckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetPauseWhenInactive(iState == Qt::Checked);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pOfflineModeCheckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetOffline(iState == Qt::Checked);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pShowPushNotificationsCeckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetPushNotifications(iState == Qt::Checked);
}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::on_pStyleHotoadCheckBox_stateChanged(qint32 iState)
{
  WIDGET_INITIALIZED_GUARD
  assert(nullptr != m_spSettings);
  if (nullptr == m_spSettings) { return; }

  m_spSettings->SetStyleHotLoad(iState == Qt::Checked);
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
  m_spSettings->setKeyBinding(keySequence, sKeyBinding);
}
