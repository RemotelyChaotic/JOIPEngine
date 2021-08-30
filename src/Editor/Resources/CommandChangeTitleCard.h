#ifndef CCOMMANDCHANGETITLECARD_H
#define CCOMMANDCHANGETITLECARD_H

#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDatabaseManager;

class CCommandChangeTitleCard : public QUndoCommand
{
public:
  CCommandChangeTitleCard(const tspProject& spCurrentProject,
                          const QString& sOldTitleCard,
                          const QString& sNewTitleCard,
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

private:
  void DoUndoRedo(const QString& sTitleCard);
};

#endif // CCOMMANDCHANGETITLECARD_H
