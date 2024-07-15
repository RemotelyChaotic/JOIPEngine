#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include <QString>
#include <QVariant>
#include <QWidget>

#include <functional>
#include <map>
#include <type_traits>
#include <typeinfo>

namespace detail
{
  typedef QWidget* (*tFnCreateWidget)(QWidget*);

  template<typename T>
  struct SDeviceRegistryEntry
  {
    static bool m_bRegistered;
  };
}

//----------------------------------------------------------------------------------------
//
struct SDeviceSettingBase
{
  virtual ~SDeviceSettingBase() {}

  QString m_sName;
  detail::tFnCreateWidget m_fnCreatorFunction;

  virtual QVariant GetDefault() const = 0;

  QVariant GetValue() const;
  void Store(QVariant value);
};

template<typename Type>
struct SDeviceSetting : public SDeviceSettingBase
{
  Type m_defautValue;

  QVariant GetDefault() const override
  {
    return QVariant::fromValue(m_defautValue);
  }

  Type GetValue() const
  {
    return SDeviceSettingBase::GetValue().template value<Type>();
  }

  void Store(Type value)
  {
    SDeviceSettingBase::Store(QVariant::fromValue(value));
  }
};

//----------------------------------------------------------------------------------------
//
class CDeviceSettingFactory
{
public:
  typedef std::map<QString /*sName*/,
                   std::unique_ptr<SDeviceSettingBase>> tSettings;

  static void CreateSettingsWidgets(QWidget* pContainer);
  static CDeviceSettingFactory& Instance();
  static void InitializeSettings();

  template<typename Type>
  static SDeviceSetting<Type>* Setting(const QString& sName)
  {
    auto it = Instance().m_settings.find(sName);
    if (Instance().m_settings.end() != it)
    {
      return dynamic_cast<SDeviceSetting<Type>*>(it->second.get());
    }
    return nullptr;
  }

  template<typename Type>
  static bool Register(const QString& sSetting, const Type& defaultValue,
                         const detail::tFnCreateWidget& fnCreateWidget)
  {
    std::unique_ptr<SDeviceSetting<Type>> spSettings =
        std::make_unique<SDeviceSetting<Type>>();
    spSettings->m_sName = sSetting;
    spSettings->m_fnCreatorFunction = fnCreateWidget;
    spSettings->m_defautValue = defaultValue;
    Instance().m_settings.emplace(sSetting, std::move(spSettings));
    return true;
  }

private:
  tSettings  m_settings;
};

//----------------------------------------------------------------------------------------
//
#define DECLARE_DEVICE_SETTING(EntryType, Name, Type, DefaultValue, WidgetCreatorFn) \
  template<> bool detail::SDeviceRegistryEntry<EntryType>::m_bRegistered = \
    CDeviceSettingFactory::Register<Type>(Name, DefaultValue, WidgetCreatorFn);

#endif // DEVICESETTINGS_H
