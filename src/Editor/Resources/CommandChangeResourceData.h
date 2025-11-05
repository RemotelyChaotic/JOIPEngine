#ifndef CCOMMANDCHANGERESOURCEDATA_H
#define CCOMMANDCHANGERESOURCEDATA_H

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"
#include <QUndoCommand>
#include <QPointer>
#include <functional>
#include <memory>

class CDatabaseManager;
class CResourceTreeItemModel;
class CResourceTreeItem;

class CCommandChangeResourceData : public QUndoCommand
{
public:
  CCommandChangeResourceData(QPointer<CResourceTreeItemModel> pModel,
                             tspProject spProject,
                             const QString& sResource,
                             qint32 iColumn,
                             const QVariant& data,
                             QUndoCommand* pParent = nullptr);
  ~CCommandChangeResourceData();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  tspProject                       m_spProject;
  tspResource                      m_spResurce;
  QPointer<CResourceTreeItemModel> m_pModel;
  QString                          m_sResource;
  qint32                           m_iColumn;
  QVariant                         m_oldData;
  QVariant                         m_data;
  bool                             m_bFirstRun;

private:
  void RunDoOrUndo(const QVariant& data);
  void SetText(CResourceTreeItem* pItem);
};

#endif // CCOMMANDCHANGERESOURCEDATA_H
