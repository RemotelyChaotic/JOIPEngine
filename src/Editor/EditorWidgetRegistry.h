#ifndef EDITORWIDGETREGISTRY_H
#define EDITORWIDGETREGISTRY_H

#include "EditorWidgetTypes.h"
#include "Settings.h"
#include <QWidget>
#include <functional>
#include <map>
#include <type_traits>

class CEditorLayoutBase;
class CEditorWidgetBase;

class CEditorFactory
{
public:
  using tLayoutCreatorFunction = std::function<CEditorLayoutBase*(QWidget*)>;
  using tWidgetCreatorFunction = std::function<CEditorWidgetBase*(QWidget*)>;

  static CEditorFactory& Instance()
  {
    // we don't like static initialisation fiaskos
    static CEditorFactory instance;
    return instance;
  }

  template<typename T,
           typename std::enable_if<std::is_base_of<CEditorLayoutBase, T>::value, bool>::type = true>
  static bool Register(CSettings::EditorType type)
  {
    Instance().m_LayoutreatorMap.insert({type, [](QWidget* pParent){ return new T(pParent); }});
    return true;
  }
  template<typename T,
           typename std::enable_if<std::is_base_of<CEditorWidgetBase, T>::value, bool>::type = true>
  static bool Register(EEditorWidget type)
  {
    Instance().m_widgetCreatorMap.insert({type, [](QWidget* pParent){ return new T(pParent); }});
    return true;
  }

  static CEditorLayoutBase* CreateLayoutInstance(QWidget* pParent, CSettings::EditorType type)
  {
    auto it = Instance().m_LayoutreatorMap.find(type);
    if (Instance().m_LayoutreatorMap.end() != it)
    {
      return it->second(pParent);
    }
    return nullptr;
  }
  static CEditorWidgetBase* CreateWidgetInstance(QWidget* pParent, EEditorWidget type)
  {
    auto it = Instance().m_widgetCreatorMap.find(type);
    if (Instance().m_widgetCreatorMap.end() != it)
    {
      return it->second(pParent);
    }
    return nullptr;
  }

private:
  std::map<CSettings::EditorType, tLayoutCreatorFunction> m_LayoutreatorMap;
  std::map<EEditorWidget, tWidgetCreatorFunction>         m_widgetCreatorMap;
};

//----------------------------------------------------------------------------------------
//
namespace detail
{
  template<typename T,
           typename std::enable_if<
             std::is_base_of<CEditorLayoutBase, T>::value ||
             std::is_base_of<CEditorWidgetBase, T>::value, bool>::type = true>
  struct SRegistryEntry
  {
    static bool m_bRegistered;
    static qint32 m_iId;
  };
}

//----------------------------------------------------------------------------------------
//
#define DECLARE_EDITORWIDGET(Class, Type) \
  template<> bool detail::SRegistryEntry<Class>::m_bRegistered =  \
    CEditorFactory::Register<Class>(Type); \
  template<> qint32 detail::SRegistryEntry<Class>::m_iId = static_cast<qint32>(Type);

#define DECLARE_EDITORLAYOUT(Class, Type) \
  template<> bool detail::SRegistryEntry<Class>::m_bRegistered =  \
    CEditorFactory::Register<Class>(Type); \
  template<> qint32 detail::SRegistryEntry<Class>::m_iId = static_cast<qint32>(Type);

#endif // EDITORWIDGETREGISTRY_H
