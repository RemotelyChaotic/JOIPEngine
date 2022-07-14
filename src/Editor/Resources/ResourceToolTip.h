#ifndef RESOURCETOOLTIP_H
#define RESOURCETOOLTIP_H

/*
 * Implementation taken in parts from to qtooltip.cpp
 */
#include "Systems/Resource.h"

#include <QPointer>
#include <QWidget>

#include <memory>

namespace Ui {
class CResourceToolTip;
}

//----------------------------------------------------------------------------------------
//
class CResourceToolTip
{
  CResourceToolTip() = delete;

public:
  static void showResource(const QPoint& pos, const tspResource& spResource, QWidget* pW = nullptr);
  static void showResource(const QPoint& pos, const tspResource& spResource, QWidget* pW, const QRect& rect);
  static void showResource(const QPoint& pos, const tspResource& spResource, QWidget* pW, const QRect& rect, qint32 iMsecShowTime);
  static inline void hideResourceTip() { showResource(QPoint(), nullptr); }

  static bool isVisible();
  static tspResource resource();

  static QPalette palette();
  static void setPalette(const QPalette&);
  static QFont font();
  static void setFont(const QFont&);
};


#endif // RESOURCETOOLTIP_H
