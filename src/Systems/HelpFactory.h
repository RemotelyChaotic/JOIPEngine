#ifndef HELPFACTORY_H
#define HELPFACTORY_H

#include "ThreadedSystem.h"
#include <QMutex>
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

  QString GetHelp(QString sKey) const;
  void RegisterHelp(QString sKey, QString sResource);
  std::map<QString /*sKey*/, QString /*resourcePath*/> HelpMap() const;

private:
  mutable QMutex                                       m_mutex;
  std::map<QString /*sKey*/, QString /*resourcePath*/> m_htmlHelpMap;
};

#endif // HELPFACTORY_H
