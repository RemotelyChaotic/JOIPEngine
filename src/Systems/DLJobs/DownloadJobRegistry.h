#ifndef DOWNLOADJOBREGISTRY_H
#define DOWNLOADJOBREGISTRY_H

#include "IDownloadJob.h"
#include <QString>
#include <QWidget>
#include <functional>
#include <map>
#include <type_traits>
#include <typeinfo>

struct SDownloadJobConfig
{
  QString m_sClassType;
  QString m_sSettingsEntry;
  QStringList m_vsAllowedHosts;
};

//----------------------------------------------------------------------------------------
//
class CDownloadJobFactory
{
public:
  typedef std::map<QString /*sType*/, SDownloadJobConfig> tJobSettings;

  static CDownloadJobFactory& Instance()
  {
    // we don't like static initialisation fiaskos
    static CDownloadJobFactory instance;
    return instance;
  }

  template<typename T,
           typename std::enable_if<std::is_base_of<IDownloadJob, T>::value, bool>::type = true>
  static bool Register(const QString& sHostSetting, const QStringList& vsDefaultValue)
  {
    const QString sType = QString(typeid(T).name());
    Instance().m_jobSettings.insert({sType, {sType, sHostSetting, vsDefaultValue}});
    Instance().m_jobCreatorMap.insert({sType, [](){ return new T(); }});
    return true;
  }

  template<typename T,
           typename U,
           typename std::enable_if<std::is_base_of<IDownloadJob, T>::value &&
                                   std::is_base_of<IDownloadJobWidgetProvider, U>::value, bool>::type = true>
  static bool RegisterWidgetProvider()
  {
    const QString sType = QString(typeid(T).name());
    Instance().m_jobWidgetCreatorMap.insert({sType, [](QWidget* pParent) -> QWidget* {
                                               U creator;
                                               QWidget* pWidget = creator();
                                               pWidget->setParent(pParent);
                                               return pWidget;
                                             }});
    return true;
  }

  static const tJobSettings& GetHostSettingMap()
  {
    return Instance().m_jobSettings;
  }

  template<typename T,
           typename std::enable_if<std::is_base_of<IDownloadJob, T>::value, bool>::type = true>
  static IDownloadJob* GetJob()
  {
    auto it = Instance().m_jobCreatorMap.find(QString(typeid(T).name()));
    if (Instance().m_jobCreatorMap.end() != it)
    {
      return it->second();
    }
    return nullptr;
  }

  static IDownloadJob* GetJob(const QString& sType)
  {
    auto it = Instance().m_jobCreatorMap.find(sType);
    if (Instance().m_jobCreatorMap.end() != it)
    {
      return it->second();
    }
    return nullptr;
  }

  template<typename T,
           typename std::enable_if<std::is_base_of<IDownloadJob, T>::value, bool>::type = true>
  static QWidget* GetJobWidget(QWidget* pParent)
  {
    auto it = Instance().m_jobWidgetCreatorMap.find(QString(typeid(T).name()));
    if (Instance().m_jobWidgetCreatorMap.end() != it)
    {
      return it->second(pParent);
    }
    return nullptr;
  }

  static QWidget* GetJobWidget(QWidget* pParent, const QString& sType)
  {
    auto it = Instance().m_jobWidgetCreatorMap.find(sType);
    if (Instance().m_jobWidgetCreatorMap.end() != it)
    {
      return it->second(pParent);
    }
    return nullptr;
  }

private:
  tJobSettings                                                  m_jobSettings;
  std::map<QString, std::function<IDownloadJob*(void)>>         m_jobCreatorMap;
  std::map<QString, std::function<QWidget*(QWidget*)>>          m_jobWidgetCreatorMap;
};

//----------------------------------------------------------------------------------------
//
namespace detail
{
  template<typename T,
           typename std::enable_if<
             std::is_base_of<IDownloadJob, T>::value ||
             std::is_base_of<IDownloadJobWidgetProvider, T>::value, bool>::type = true>
  struct SJobRegistryEntry
  {
    static bool m_bRegistered;
  };
}

//----------------------------------------------------------------------------------------
//
#define DECLARE_DOWNLOADJOB(Class, HostSettingName, HostSettingDefaultHosts) \
  template<> bool detail::SJobRegistryEntry<Class>::m_bRegistered =  \
    CDownloadJobFactory::Register<Class>(HostSettingName, HostSettingDefaultHosts);
#define DECLARE_JOBSETTINGS_WIDGETPROVIDER(Class, JobClass) \
  template<> bool detail::SJobRegistryEntry<Class>::m_bRegistered =  \
    CDownloadJobFactory::RegisterWidgetProvider<JobClass, Class>();

#endif // DOWNLOADJOBREGISTRY_H
