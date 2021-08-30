#ifndef CCOMMANDCHANGECURRENTRESOURCE_H
#define CCOMMANDCHANGECURRENTRESOURCE_H

#include "Systems/DatabaseManager.h"
#include <QUndoCommand>
#include <QPointer>
#include <functional>
#include <memory>

class CResourceDisplayWidget;
class QItemSelectionModel;
class QSortFilterProxyModel;

class CCommandChangeCurrentResource : public QUndoCommand
{
public:
  CCommandChangeCurrentResource(std::shared_ptr<SProject> spCurrentProject,
                                QPointer<CResourceDisplayWidget> pResourceDisplayWidget,
                                QPointer<QItemSelectionModel> pSelectionModel,
                                QPointer<QSortFilterProxyModel> pProxyModel,
                                const QString& sOld,
                                const QString& sNew,
                                std::function<void(const QString&)> fnOnSelectionChanged,
                                QUndoCommand* pParent = nullptr);
  ~CCommandChangeCurrentResource();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  std::shared_ptr<SProject>       m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CResourceDisplayWidget> m_pResourceDisplayWidget;
  QPointer<QItemSelectionModel>    m_pSelectionModel;
  QPointer<QSortFilterProxyModel>  m_pProxyModel;
  QString m_sOldResource;
  QString m_sNewResource;
  std::function<void(const QString&)> m_fnOnSelectionChanged;

private:
  void RunDoOrUndo(const QString& sResource);
};

#endif // CCOMMANDCHANGECURRENTRESOURCE_H
