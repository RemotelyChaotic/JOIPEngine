#ifndef CWINDOWSNATIVEPUSHNOTIFICATION_H
#define CWINDOWSNATIVEPUSHNOTIFICATION_H

#include <wintoastlib.h>

#include <QPointer>
#include <QString>

#include <chrono>
#include <memory>

class CMainWindow;
class CWinToastHandler;

class CWindowsNativePushNotification
{
public:
  CWindowsNativePushNotification(QPointer<CMainWindow> pMainWindow);
  ~CWindowsNativePushNotification();

  void SetMainWindow(QPointer<CMainWindow> pMainWindow);
  void Show(const QString& sTitle, const QString& sMsg,
            std::chrono::milliseconds displayTime);

private:
  std::unique_ptr<WinToastLib::WinToastTemplate> m_spTempl;
  std::shared_ptr<CWinToastHandler>              m_spToastHandler;
  QPointer<CMainWindow>                          m_pMainWindow;
};

#endif // CWINDOWSNATIVEPUSHNOTIFICATION_H
