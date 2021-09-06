#ifndef CRAIIFUNCTIONCALLER_H
#define CRAIIFUNCTIONCALLER_H

#include <functional>

class CRaiiFunctionCaller
{
public:
  CRaiiFunctionCaller(std::function<void(void)> fnToCall);
  ~CRaiiFunctionCaller();

private:
  std::function<void(void)> m_fnToCall;
};

#endif // CRAIIFUNCTIONCALLER_H
