#include "Style.h"

#include <QDebug>
#include <QFileInfo>
#include <QFont>
#include <QString>

namespace
{
  const QString c_sDefaultStyle = "style.css";
}

//----------------------------------------------------------------------------------------
//
void joip_style::SetStyle(QApplication* pApp)
{
  QFileInfo info(c_sDefaultStyle);
  if (info.exists())
  {
    QFile styleFile(info.absoluteFilePath());
    if (styleFile.open(QIODevice::ReadOnly))
    {
      pApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
    else
    {
      qWarning() << QString(QT_TR_NOOP("Could not open stylesheet with the name: %1."))
                    .arg(info.absoluteFilePath());
    }
  }
  else
  {
    QFile styleFile("://resources/style_default.css");
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

  QFont font("Equestria", 10);
  pApp->setFont(font);
}

