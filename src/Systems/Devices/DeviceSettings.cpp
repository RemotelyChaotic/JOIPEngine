#include "DeviceSettings.h"
#include "Application.h"
#include "Settings.h"

#include <QLayout>

namespace
{
  static const char* c_sDeviceSettingsBase = "Devices/";
}

//----------------------------------------------------------------------------------------
//
QVariant SDeviceSettingBase::GetValue() const
{
  const QString sSettingName = QString(c_sDeviceSettingsBase) + m_sName;
  return CApplication::Instance()->Settings()->ReadRaw(sSettingName, GetDefault());
}

//----------------------------------------------------------------------------------------
//
void SDeviceSettingBase::Store(QVariant value)
{
  const QString sSettingName = QString(c_sDeviceSettingsBase) + m_sName;
  CApplication::Instance()->Settings()->WriteRaw(sSettingName, value);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSettingFactory::CreateSettingsWidgets(QWidget* pContainer)
{
  QLayout* pLayout = pContainer->layout();
  while (auto pItem = pLayout->takeAt(0))
  {
    if (nullptr != pItem->widget()) delete pItem->widget();
    delete pItem;
  }

  for (const auto& [_, spSetting] : Instance().m_settings)
  {
    QWidget* pWidget = spSetting->m_fnCreatorFunction(pContainer);
    if (nullptr != pWidget)
    {
      pLayout->addWidget(pWidget);
    }
  }
}

//----------------------------------------------------------------------------------------
//
CDeviceSettingFactory& CDeviceSettingFactory::Instance()
{
  // we don't like static initialisation fiaskos
  static CDeviceSettingFactory instance;
  return instance;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSettingFactory::InitializeSettings()
{
  auto spSettings = CApplication::Instance()->Settings();
  if (nullptr != spSettings)
  {
    for (const auto& [sName, spSetting] : Instance().m_settings)
    {
      const QString sSettingName = QString(c_sDeviceSettingsBase) + sName;
      if (!spSettings->HasRaw(sSettingName))
      {
        spSettings->WriteRaw(sSettingName, spSetting->GetDefault());
      }
    }
  }
}
