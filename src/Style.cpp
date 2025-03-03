#include "Style.h"

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
    QFileInfo info(joip_style::StyleFolder() +
                   QDir::separator() + sName +
                   QDir::separator() + c_sStyleFile);
    if (info.exists())
    {
      QFile styleFile(info.absoluteFilePath());
      pApp->setStyleSheet(QString());
      pApp->setStyleSheet("file:///" + info.absoluteFilePath());

      QFileInfo qmlStyle(joip_style::StyleFolder() +
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
  const QString sStyleFolder = StyleFolder();
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
QString joip_style::StyleFile(const QString& sStyle)
{
  QFileInfo info(StyleFolder() +
                 QDir::separator() + sStyle +
                 QDir::separator() + c_sStyleFile);
  return info.absoluteFilePath();
}

//----------------------------------------------------------------------------------------
//
QString joip_style::StyleFolder()
{
#if defined(Q_OS_ANDROID)
  return c_sDefaultAndroidStyleFolder;
#elif defined(Q_OS_LINUX)
  QString sRootPath = QLibraryInfo::location(QLibraryInfo::PrefixPath);
  const QString sAppImg = qgetenv("APPIMAGE");
  if (!sAppImg.isEmpty())
  {
    sRootPath = QFileInfo(sAppImg).absolutePath();
  }
  return sRootPath + QDir::separator() + c_sStyleFolder;
#else
  return QLibraryInfo::location(QLibraryInfo::PrefixPath) +
      QDir::separator() + c_sStyleFolder;
#endif
}

//----------------------------------------------------------------------------------------
//
void joip_style::SetStyle(QApplication* pApp, const QString& sStyle, const QString& sFont)
{
  if (sStyle == c_sDefaultStyle)
  {
    LoadDefaultStyle(pApp);
  }
  else
  {
    ResolveStyle(pApp, sStyle);
  }

  QFont font(sFont, 10);
  pApp->setFont(font);
}

