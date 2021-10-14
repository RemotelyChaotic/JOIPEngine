#include "NotificationSender.h"
#include "MainWindow.h"

#include "Widgets/PushNotification.h"

CNotificationSender* CNotificationSender::m_pInstance = nullptr;

//----------------------------------------------------------------------------------------
//
CNotificationSender::CNotificationSender() :
  QObject(nullptr),
  m_pMainWindow(nullptr)
{
  m_pInstance = this;
}
CNotificationSender::~CNotificationSender()
{
  m_pInstance = nullptr;
}

//----------------------------------------------------------------------------------------
//
CNotificationSender* CNotificationSender::Instance()
{
  return m_pInstance;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SendNotification(const QString& sMsg)
{
  bool bOk =
      QMetaObject::invokeMethod(this, "SlotSendNotification", Qt::QueuedConnection,
                                Q_ARG(QString, sMsg));
  assert(bOk);
  Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SetMainWindow(CMainWindow* pMainWindow)
{
  m_pMainWindow = pMainWindow;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SlotSendNotification(const QString& sMsg)
{
  if (nullptr != m_pMainWindow)
  {
    using namespace std::chrono_literals;
    CPushNotification* pNotification =
        new CPushNotification(sMsg, 5s, m_pMainWindow.data());
    pNotification->Climb();
    pNotification->Move(pNotification->x(), pNotification->height());
    pNotification->Resize();
    pNotification->Show();
  }
}
