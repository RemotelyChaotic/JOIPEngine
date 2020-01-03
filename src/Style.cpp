#include "Style.h"

#include "Application.h"
#include "Settings.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFont>
#include <QLibraryInfo>
#include <QString>

namespace
{
  const QString c_sStyleFolder = "styles";
  const QString c_sDefaultStyleFile = "://resources/style_default.css";
  const QString c_sStyleFile = "style.css";

//----------------------------------------------------------------------------------------
//
  void LoadDefaultStyle(QApplication* pApp)
  {
    QFileInfo info(c_sDefaultStyleFile);
    if (info.exists())
    {
      QFile styleFile(c_sDefaultStyleFile);
      if (styleFile.open(QIODevice::ReadOnly))
      {
        pApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
      }
      else
      {
        qWarning() << QString(QT_TR_NOOP("Could not open stylesheet from resources: %1."))
                      .arg(info.absoluteFilePath());
      }
    }
  }

//----------------------------------------------------------------------------------------
//
  void ResolveStyle(QApplication* pApp, const QString& sName)
  {
    QFileInfo info(QLibraryInfo::location(QLibraryInfo::PrefixPath) +
                   QDir::separator() + c_sStyleFolder + QDir::separator() + sName +
                   QDir::separator() + c_sStyleFile);
    if (info.exists())
    {
      QFile styleFile(info.absoluteFilePath());
      pApp->setStyleSheet("file:///" + info.absoluteFilePath());
    }
    else
    {
      qWarning() << QString(QT_TR_NOOP("Could not find stylesheet with the name: %1."))
                    .arg(info.absoluteFilePath());
      LoadDefaultStyle(pApp);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QStringList joip_style::AvailableStyles()
{
  const QString sStyleFolder = QLibraryInfo::location(QLibraryInfo::PrefixPath) +
      QDir::separator() + c_sStyleFolder;
  QDirIterator iter(sStyleFolder, QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::NoIteratorFlags);

  QStringList out;
  out << c_sDefaultStyle;
  while (iter.hasNext())
  {
    out << QDir(iter.next()).dirName();
  }

  return out;
}

//----------------------------------------------------------------------------------------
//
void joip_style::SetStyle(QApplication* pApp)
{
  auto spSettings = dynamic_cast<CApplication*>(pApp)->Settings();

  QString sStyle = spSettings->Style();
  if (sStyle == c_sDefaultStyle)
  {
    LoadDefaultStyle(pApp);
  }
  else
  {
    ResolveStyle(pApp, sStyle);
  }

  QFont font(spSettings->Font(), 10);
  pApp->setFont(font);
}

