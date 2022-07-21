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

  /*!
   * \brief Close of Application was requested
   * \return true if event is consumed, false otherwise (similarly to eventFilter)
   */
  virtual bool CloseApplication() = 0;
  void Initialize() override {}
  virtual void Load() = 0;
  virtual void Unload() = 0;

signals:
  virtual void UnloadFinished() = 0;

protected:
  std::shared_ptr<CWindowContext>  m_spWindowContext;
};

Q_DECLARE_INTERFACE(IAppStateScreen, "IAppStateScreen")

#endif // IAPPSTATESCREEN_H
