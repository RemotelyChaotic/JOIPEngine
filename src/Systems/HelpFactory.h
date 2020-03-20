#ifndef HELPFACTORY_H
#define HELPFACTORY_H

#include "ThreadedSystem.h"
#include <QObject>

class CHelpFactory : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CHelpFactory)

public:
  CHelpFactory();
  ~CHelpFactory() override;

public slots:
  void Initialize() override;
  void Deinitialize() override;
};

#endif // HELPFACTORY_H
