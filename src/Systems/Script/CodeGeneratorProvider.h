#ifndef CCODEGENERATORPROVIDER_H
#define CCODEGENERATORPROVIDER_H

#include "ICodeGenerator.h"
#include <map>
#include <memory>
#include <type_traits>

class CCodeGeneratorProvider
{
  CCodeGeneratorProvider();
  ~CCodeGeneratorProvider();

public:
  static CCodeGeneratorProvider& Instance();
  std::shared_ptr<ICodeGenerator> Generator(const QString& sCodeType) const;

  template<typename T, typename = std::enable_if_t<std::is_base_of_v<ICodeGenerator, T>>>
  static bool RegisterGenerator(const QString& sCodeType)
  {
    Instance().m_generators.insert({sCodeType, std::make_shared<T>()});
    return true;
  }

private:
  std::map<QString, std::shared_ptr<ICodeGenerator>> m_generators;
};

//----------------------------------------------------------------------------------------
//
namespace detail
{
  template<typename T,
           typename std::enable_if_t<
             std::is_base_of_v<ICodeGenerator, T> ||
             std::is_base_of_v<ICodeGenerator, T>, bool> = true>
  struct SCodeGeneratorRegistryEntry
  {
    static bool m_bRegistered;
  };
}

//----------------------------------------------------------------------------------------
//
#define DECLARE_CODE_GENERATOR(Type, string) \
 template<> bool detail::SCodeGeneratorRegistryEntry<Type>::m_bRegistered =  \
   CCodeGeneratorProvider::RegisterGenerator<Type>(string);

#endif // CCODEGENERATORPROVIDER_H
