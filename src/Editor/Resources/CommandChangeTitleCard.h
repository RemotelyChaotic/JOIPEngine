#ifndef CCOMMANDCHANGETITLECARD_H
#define CCOMMANDCHANGETITLECARD_H

#include "Systems/Database/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <functional>
#include <memory>

class CDatabaseManager;

class CCommandChangeTitleCard : public QUndoCommand
{
public:
  CCommandChangeTitleCard(const tspProject& spCurrentProject,
                          const QString& sOldTitleCard,
                          const QString& sNewTitleCard,
                          const std::function<void(void)>& fnOnChanged,
                          QUndoCommand* pParent = nullptr);
  ~CCommandChangeTitleCard();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject                      m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString m_sOldTitleCard;
  QString m_sNewTitleCard;
  std::function<void(void)> m_fnOnChanged;

private:
  void DoUndoRedo(const QString& sTitleCard);
};

#endif // CCOMMANDCHANGETITLECARD_H
