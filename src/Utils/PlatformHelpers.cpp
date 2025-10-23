#include "PlatformHelpers.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHostInfo>
#include <QProcess>
#include <QUrl>

namespace helpers
{
  // the functions in this namespace are partially from QtCtreator code:
  constexpr EHostOsType HostOs()
  {
#if defined(Q_OS_WIN)
    return eOsTypeWindows;
#elif defined(Q_OS_LINUX)
    return eOsTypeLinux;
#elif defined(Q_OS_MAC)
    return eOsTypeMac;
#elif defined(Q_OS_UNIX)
    return eOsTypeOtherUnix;
#else
    return eOsTypeOther;
#endif
  }

  bool IsWindowsHost() { return HostOs() == EHostOsType::eOsTypeWindows; }
  bool IsMacHost() { return HostOs() == EHostOsType::eOsTypeMac; }

  void ShowInGraphicalShell(const QString& sPathIn)
  {
    const QFileInfo fileInfo(sPathIn);
    // Mac, Windows support folder or file.
    if (IsWindowsHost())
    {
      const QString sExplorer = "explorer.exe";
      QStringList param;
      if (!fileInfo.isDir())
      {
        param += QLatin1String("/select,");
      }
      param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
      QProcess::startDetached(sExplorer, param);
    }
    else if (IsMacHost())
    {
      QStringList scriptArgs;
      scriptArgs << QLatin1String("-e")
                 << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                        .arg(fileInfo.canonicalFilePath());
      QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
      scriptArgs.clear();
      scriptArgs << QLatin1String("-e")
                 << QLatin1String("tell application \"Finder\" to activate");
      QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    }
    else
    {
      QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.absolutePath()));
    }
  }
}
