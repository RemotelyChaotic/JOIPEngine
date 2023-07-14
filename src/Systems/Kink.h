#ifndef KINK_H
#define KINK_H

#include "DatabaseInterface/TagData.h"
#include <QReadWriteLock>
#include <QColor>
#include <QObject>
#include <QString>
#include <map>
#include <memory>

//----------------------------------------------------------------------------------------
//
struct SKink : public STagData
{
  SKink();
  SKink(qint32 iId, QString sType, QString sName, QString sDescribtion);
  SKink(const SKink& other);
  ~SKink();

  mutable QReadWriteLock  m_rwLock;
  qint32                  m_iIdForOrdering = -1;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SKink>      tspKink;
typedef std::map<QString, tspKink>  tKinks;
typedef std::map<QString, tKinks>   tKinkKategories;

#endif // KINK_H
