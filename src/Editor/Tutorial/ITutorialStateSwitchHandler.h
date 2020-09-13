#ifndef ITUTORIALSTATESWITCHER_H
#define ITUTORIALSTATESWITCHER_H

#include "Enums.h"

class ITutorialStateSwitchHandler
{
public:
  virtual ~ITutorialStateSwitchHandler() {}

  virtual void OnResetStates() = 0;
  virtual void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) = 0;

protected:
  ITutorialStateSwitchHandler();
};

#endif // ITUTORIALSTATESWITCHER_H
