#ifndef CJOIPMESSAGEHANDLER_H
#define CJOIPMESSAGEHANDLER_H

#include "ICustomMessageHandler.h"

#include <QDebug>

class CJoipMessageHandler
{
public:
  CJoipMessageHandler();
  ~CJoipMessageHandler();

  static void AddHandler(ICustomMessageHandler* pHandler);
  static void InstallMsgHandler();
  static void Message(QtMsgType type, const QMessageLogContext& context,
                      const QString& sMsg);
  static void RemoveHandler(ICustomMessageHandler* pHandler);

protected:
  void AddHandlerImpl(ICustomMessageHandler* pHandler);
  void InstallHandlerImpl();
  void MessageImpl(QtMsgType type, const QMessageLogContext& context,
                   const QString& sMsg);
  void RemoveHandlerImpl(ICustomMessageHandler* pHandler);

private:
  static CJoipMessageHandler* Instance();

  static std::unique_ptr<CJoipMessageHandler> m_spInstance;
  std::function<void(QtMsgType,const QMessageLogContext&,const QString&)>
      m_fnDefaultHandler = nullptr;
  std::vector<ICustomMessageHandler*> m_vpRegisteredHandlers;
};

#endif // CJOIPMESSAGEHANDLER_H
