#ifndef DATABASEACCESSOBJECT_H
#define DATABASEACCESSOBJECT_H

#include <QObject>

class CDatabaseAccessObject : public QObject
{
  Q_OBJECT
public:
  CDatabaseAccessObject();
  ~CDatabaseAccessObject() override;

signals:

public slots:
};

#endif // DATABASEACCESSOBJECT_H
