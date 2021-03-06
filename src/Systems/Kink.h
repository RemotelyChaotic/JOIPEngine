#ifndef KINK_H
#define KINK_H

#include <enum.h>
#include <QReadWriteLock>
#include <QString>
#include <map>
#include <memory>


//----------------------------------------------------------------------------------------
//
struct SKink
{
  SKink();
  SKink(QString sType, QString sName, QString sDescribtion);
  SKink(const SKink& other);
  ~SKink();

  mutable QReadWriteLock  m_rwLock;
  QString                 m_sType;
  QString                 m_sName;
  QString                 m_sDescribtion;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SKink>      tspKink;
typedef std::map<QString, tspKink>  tKinks;
typedef std::map<QString, tKinks>   tKinkKategories;

#endif // KINK_H
