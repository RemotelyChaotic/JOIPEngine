#include "JoipMessageHandler.h"

namespace
{
  void JoipMessageHandler(QtMsgType type, const QMessageLogContext& context,
                          const QString& sMsg)
  {
    CJoipMessageHandler::Message(type, context, sMsg);
  }
}

//----------------------------------------------------------------------------------------
//
CJoipMessageHandler::CJoipMessageHandler()
{
}
CJoipMessageHandler::~CJoipMessageHandler()
{
  qInstallMessageHandler(0);
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::AddHandler(ICustomMessageHandler* pHandler)
{
  Instance()->AddHandlerImpl(pHandler);
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::InstallMsgHandler()
{
  Instance()->InstallHandlerImpl();
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::Message(QtMsgType type, const QMessageLogContext& context,
                    const QString& sMsg)
{
  Instance()->MessageImpl(type, context, sMsg);
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::RemoveHandler(ICustomMessageHandler* pHandler)
{
  Instance()->RemoveHandlerImpl(pHandler);
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::AddHandlerImpl(ICustomMessageHandler* pHandler)
{
  auto it = std::find(m_vpRegisteredHandlers.begin(), m_vpRegisteredHandlers.end(), pHandler);
  if (m_vpRegisteredHandlers.end() == it)
  {
    m_vpRegisteredHandlers.push_back(pHandler);
  }
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::InstallHandlerImpl()
{
  m_fnDefaultHandler = qInstallMessageHandler(::JoipMessageHandler);
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::MessageImpl(QtMsgType type, const QMessageLogContext& context,
                                      const QString& sMsg)
{
  for (auto& pHandler : m_vpRegisteredHandlers)
  {
    if (pHandler->MessageImpl(type, context, sMsg))
    {
      return;
    }
  }

  // propagate msg to default handler
  if (nullptr != m_fnDefaultHandler)
  {
    m_fnDefaultHandler(type, context, sMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CJoipMessageHandler::RemoveHandlerImpl(ICustomMessageHandler* pHandler)
{
  auto it = std::find(m_vpRegisteredHandlers.begin(), m_vpRegisteredHandlers.end(), pHandler);
  if (m_vpRegisteredHandlers.end() != it)
  {
    m_vpRegisteredHandlers.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
CJoipMessageHandler* CJoipMessageHandler::Instance()
{
  if (nullptr != m_spInstance)
  {
    return m_spInstance.get();
  }

  m_spInstance = std::make_unique<CJoipMessageHandler>();
  return m_spInstance.get();
}

//----------------------------------------------------------------------------------------
//
std::unique_ptr<CJoipMessageHandler> CJoipMessageHandler::m_spInstance = nullptr;
