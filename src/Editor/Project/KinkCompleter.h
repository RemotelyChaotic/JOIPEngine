#ifndef CKINKCOMPLETER_H
#define CKINKCOMPLETER_H

#include <QCompleter>
#include <memory>

class CDatabaseManager;
class CKinkTreeModel;

class CKinkCompleter : public QCompleter
{
  Q_OBJECT

public:
  explicit CKinkCompleter(CKinkTreeModel* pModel, QObject* pParent = nullptr);
  QStringList splitPath(const QString& sPath) const override;
  QString pathFromIndex(const QModelIndex& index) const override;

private:
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
};

#endif // CKINKCOMPLETER_H
