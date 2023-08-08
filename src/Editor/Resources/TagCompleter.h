#ifndef CTAGCOMPLETER_H
#define CTAGCOMPLETER_H

#include "Systems/Project.h"
#include <QCompleter>

class CDatabaseManager;

class CTagCompleter : public QCompleter
{
  Q_OBJECT

public:
  explicit CTagCompleter(QAbstractItemModel* pModel = nullptr, QObject* pParent = nullptr);

  void SetCurrentProject(const tspProject& spProject);

  QStringList splitPath(const QString& sPath) const override;
  QString pathFromIndex(const QModelIndex& index) const override;

private:
  tspProject                      m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
};

#endif // CTAGCOMPLETER_H
