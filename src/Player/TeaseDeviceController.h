#ifndef CTEASEDEVICECONTROLLER_H
#define CTEASEDEVICECONTROLLER_H

#include <QObject>
#include <QPointer>

#include <memory>

class CDeviceManager;
class CSceneMainScreen;
class IDevice;

class CTeaseDeviceController : public QObject
{
  Q_OBJECT
  Q_PROPERTY(qint32 numberRegisteredConnectors READ NumberRegisteredConnectors CONSTANT)

public:
  explicit CTeaseDeviceController(QObject* pParent, QPointer<CSceneMainScreen> pMainScreen);
  ~CTeaseDeviceController() override;

  qint32 NumberRegisteredConnectors() const;

public slots:
  bool isDeviceConnected();
  void openSelectionScreen(double x, double y);
  void pause();
  void resume();
  void selectDevice(const QString& sDevice);
  void sendLinearCmd(quint32 iDurationMs, double dPosition);
  void sendRotateCmd(bool bClockwise, double dSpeed);
  void sendStopCmd();
  void sendVibrateCmd(double dSpeed);

private:
  std::shared_ptr<IDevice>      m_spCurrentDevice;
  std::weak_ptr<CDeviceManager> m_wpDeviceManager;
  QPointer<CSceneMainScreen>    m_pMainScreen;

  double                        m_dLastVibrateSpeed = 0.0;
  quint32                       m_ilastLinarDurationMs = 0;
  double                        m_dlastLinearPosition = 0.0;
  bool                          m_bLastRotateClockwise = false;
  double                        m_dlastRoateSpeed = 0.0;
};

#endif // CTEASEDEVICECONTROLLER_H
