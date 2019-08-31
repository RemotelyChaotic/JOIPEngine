#ifndef IAPPSTATESCREEN_H
#define IAPPSTATESCREEN_H

#include "WindowContext.h"
#include "Widgets/IWidgetBaseInterface.h"
#include <memory>


class IAppStateScreen : public IWidgetBaseInterface
{
public:
  explicit IAppStateScreen(const std::shared_ptr<CWindowContext>& spWindowContext) :
    IWidgetBaseInterface(),
    m_spWindowContext(spWindowContext)
  {}
  ~IAppStateScreen() override {}

  void Initialize() override {}
  virtual void Load() = 0;
  virtual void Unload() = 0;

protected:
  std::shared_ptr<CWindowContext>  m_spWindowContext;
};

#endif // IAPPSTATESCREEN_H
