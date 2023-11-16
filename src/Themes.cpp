#include "Themes.h"

#include <repository.h>
#include <theme.h>

#include <QDebug>
#include <QDir>
#include <QLibraryInfo>
#include <QString>

//----------------------------------------------------------------------------------------
//
namespace
{
  KSyntaxHighlighting::Repository* ThemeRepository()
  {
    static std::unique_ptr<KSyntaxHighlighting::Repository> spRepository;
    if (nullptr == spRepository)
    {
      spRepository.reset(new KSyntaxHighlighting::Repository());
      spRepository->addCustomSearchPath(joip_style::ThemeFolder());
    }
    return spRepository.get();
  }
}

//----------------------------------------------------------------------------------------
//
QStringList joip_style::AvailableThemes()
{
  KSyntaxHighlighting::Repository* pRepository = ThemeRepository();
  QStringList vsThemes;
  for (const KSyntaxHighlighting::Theme& theme : pRepository->themes())
  {
    vsThemes << theme.name();
  }
  return vsThemes;
}

//----------------------------------------------------------------------------------------
//
QString joip_style::ThemeFolder()
{
  // "/themes" is appended to the given path by the library
#if defined(Q_OS_ANDROID)
  return QString(":/android/resources");
#else
  return QLibraryInfo::location(QLibraryInfo::PrefixPath);
#endif
}

