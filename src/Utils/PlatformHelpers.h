#ifndef CPLATFORMHELPERS_H
#define CPLATFORMHELPERS_H

#include <QString>

namespace helpers
{
  enum EHostOsType
  {
    eOsTypeWindows,
    eOsTypeLinux,
    eOsTypeMac,
    eOsTypeOtherUnix,
    eOsTypeOther
  };

  bool IsWindowsHost();
  bool IsMacHost();

  constexpr EHostOsType HostOs();

  void ShowInGraphicalShell(const QString& sPathIn);
}

#endif // CPLATFORMHELPERS_H
