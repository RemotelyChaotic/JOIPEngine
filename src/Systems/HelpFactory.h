#ifndef HELPFACTORY_H
#define HELPFACTORY_H

#include "ThreadedSystem.h"
#include <QObject>
#include <map>

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

  QString GetHelp(QString sKey);
  void RegisterHelp(QString sKey, QString sResource);

private:
  std::map<QString /*sKey*/, QString /*resourcePath*/> m_htmlHelpMap;
};

#endif // HELPFACTORY_H
