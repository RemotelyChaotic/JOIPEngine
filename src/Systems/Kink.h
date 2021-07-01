#ifndef KINK_H
#define KINK_H

#include <enum.h>
#include <QReadWriteLock>
#include <QColor>
#include <QObject>
#include <QString>
#include <map>
#include <memory>

class QJSEngine;

//----------------------------------------------------------------------------------------
//
struct SKink
{
  SKink();
  SKink(qint32 iId, QString sType, QString sName, QString sDescribtion);
  SKink(const SKink& other);
  ~SKink();

  mutable QReadWriteLock  m_rwLock;
  qint32                  m_iIdForOrdering;
  QString                 m_sType;
  QString                 m_sName;
  QString                 m_sDescribtion;
};

//----------------------------------------------------------------------------------------
//
class CKink : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CKink)
  CKink() {}
  Q_PROPERTY(qint32   idForOrdering              READ getIdForOrdering     CONSTANT)
  Q_PROPERTY(QString  type                       READ getType              CONSTANT)
  Q_PROPERTY(QString  name                       READ getName              CONSTANT)
  Q_PROPERTY(QString  describtion                READ getDescribtion       CONSTANT)

public:
  explicit CKink(QJSEngine* pEngine, const std::shared_ptr<SKink>& spKink);
  ~CKink();

  qint32 getIdForOrdering();
  QString getType();
  QString getName();
  QString getDescribtion();

  Q_INVOKABLE QColor color();

  std::shared_ptr<SKink> Data() { return m_spData; }

private:
  std::shared_ptr<SKink>              m_spData;
  QJSEngine*                          m_pEngine;
};

QColor CalculateKinkColor(const SKink& kink);

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SKink>      tspKink;
typedef std::map<QString, tspKink>  tKinks;
typedef std::map<QString, tKinks>   tKinkKategories;

#endif // KINK_H
