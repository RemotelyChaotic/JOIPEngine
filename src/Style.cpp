#include "Style.h"

#include "Application.h"
#include "Settings.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFont>
#include <QLibraryInfo>
#include <QQuickStyle>
#include <QString>

namespace
{
  const QString c_sDefaultStyleFile = QString(joip_style::c_sDefaultStyleFolder) + "style_default.css";
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
        pApp->setStyleSheet(QString());
        pApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));

        QQuickStyle::setStyle("Material");
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
                   QDir::separator() + joip_style::c_sStyleFolder +
                   QDir::separator() + sName +
                   QDir::separator() + c_sStyleFile);
    if (info.exists())
    {
      QFile styleFile(info.absoluteFilePath());
      pApp->setStyleSheet(QString());
      pApp->setStyleSheet("file:///" + info.absoluteFilePath());

      QFileInfo qmlStyle(QLibraryInfo::location(QLibraryInfo::PrefixPath) +
                         QDir::separator() + joip_style::c_sStyleFolder +
                         QDir::separator() + sName +
                         QDir::separator() + joip_style::c_sQmlStyleSubFolder);
      if (qmlStyle.exists())
      {
        QQuickStyle::setStyle(QUrl::fromLocalFile(qmlStyle.absoluteFilePath()).toString());
        QQuickStyle::setFallbackStyle("Material");
      }
      else
      {
        QQuickStyle::setStyle("Material");
      }
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

