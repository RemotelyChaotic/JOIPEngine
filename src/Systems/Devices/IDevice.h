#ifndef IDEVICE_H
#define IDEVICE_H

#include <QtGlobal>

class IDevice
{
public:
  virtual ~IDevice() {}

  virtual QString Name() const = 0;
  virtual QString DisplayName() const = 0;
  virtual void SendLinearCmd(quint32 iDurationMs, double dPosition) = 0;
  virtual void SendRotateCmd(bool bClockwise, double dSpeed) = 0;
  virtual void SendStopCmd() = 0;
  virtual void SendVibrateCmd(double dSpeed) = 0;

protected:
  IDevice() {}
};

#endif // IDEVICE_H
