#ifndef CCOMMANDADDRESOURCE_H
#define CCOMMANDADDRESOURCE_H

#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <map>
#include <memory>

class CSettings;
class CDatabaseManager;

class CCommandAddResource : public QUndoCommand
{
public:
  CCommandAddResource(const tspProject& spProject,
                      QPointer<QWidget> pParentForDialog,
                      // QByteArray suports copy-on-write... we rely on that fact
                      const std::map<QUrl, QByteArray>& vsFiles,
                      QUndoCommand* pParent = nullptr);
  ~CCommandAddResource();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspResourceMap     m_addedResources;
  tspProject         m_spCurrentProject;
  std::shared_ptr<CSettings>      m_spSettings;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<QWidget>      m_pParentForDialog;
  std::map<QUrl, QByteArray> m_vsFiles;
};

#endif // CCOMMANDADDRESOURCE_H
