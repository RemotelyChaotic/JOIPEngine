#ifndef CCOMMANDADDRESOURCE_H
#define CCOMMANDADDRESOURCE_H

#include "Systems/Database/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <map>
#include <memory>
#include <set>

class CSettings;
class CDatabaseManager;

class CCommandAddResource : public QUndoCommand
{
public:
  CCommandAddResource(const tspProject& spProject,
                      QPointer<QWidget> pParentForDialog,
                      const std::set<QUrl> vsFiles,
                      QUndoCommand* pParent = nullptr);
  CCommandAddResource(const tspProject& spProject,
                      QPointer<QWidget> pParentForDialog,
                      const std::map<QString, SResourceData>& resData,
                      QUndoCommand* pParent = nullptr);
  ~CCommandAddResource();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  std::map<QString, SResourceData> m_addedResources;
  tspProject                       m_spCurrentProject;
  std::shared_ptr<CSettings>       m_spSettings;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QPointer<QWidget>                m_pParentForDialog;
  std::set<QUrl>                   m_vsFiles;
};

#endif // CCOMMANDADDRESOURCE_H
