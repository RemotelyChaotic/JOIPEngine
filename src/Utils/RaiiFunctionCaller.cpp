#include "RaiiFunctionCaller.h"

CRaiiFunctionCaller::CRaiiFunctionCaller(std::function<void(void)> fnToCall) :
  m_fnToCall(fnToCall)
{
}
CRaiiFunctionCaller::~CRaiiFunctionCaller()
{
  if (nullptr != m_fnToCall) { m_fnToCall(); };
}
