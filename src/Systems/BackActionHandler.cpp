#include "BackActionHandler.h"
#include "Application.h"

#include <QGuiApplication>
#include <QKeyEvent>

CBackActionHandler::CBackActionHandler() :
  QObject(nullptr),
  m_optSlotToCallOnBack(std::nullopt)
{
  CApplication::Instance()->installEventFilter(this);
}
CBackActionHandler::~CBackActionHandler() {}

//----------------------------------------------------------------------------------------
//
void CBackActionHandler::ClearSlotToCall()
{
  m_optSlotToCallOnBack = std::nullopt;
}

//----------------------------------------------------------------------------------------
//
void CBackActionHandler::RegisterSlotToCall(QPointer<QObject> pObject, QString sSlot)
{
  m_optSlotToCallOnBack = {pObject, sSlot};
}

//----------------------------------------------------------------------------------------
//
bool CBackActionHandler::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    if (pEvt->type() == QEvent::Type::KeyRelease)
    {
      QKeyEvent* pKeyEvt = static_cast<QKeyEvent*>(pEvt);
      if (Qt::Key_Back == pKeyEvt->key())
      {
        return HandleBackAction();
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CBackActionHandler::HandleBackAction()
{
  if (!m_optSlotToCallOnBack.has_value())
  {
    return false;
  }

  auto& pair = m_optSlotToCallOnBack.value();
  bool bOk = QMetaObject::invokeMethod(pair.first, pair.second.toStdString().data(),
                                       Qt::QueuedConnection);
  return bOk;
}
