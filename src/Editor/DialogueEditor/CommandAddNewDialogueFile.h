#ifndef CCOMMANDADDNEWDIALOGUEFILE_H
#define CCOMMANDADDNEWDIALOGUEFILE_H

#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <QWidget>
#include <memory>

class CDatabaseManager;
class CEditorModel;

class CCommandAddNewDialogueFile : public QUndoCommand
{
public:
  CCommandAddNewDialogueFile(const tspProject& spProject,
                           QPointer<CEditorModel> pEditorModel,
                           QPointer<QWidget> pParent);
  ~CCommandAddNewDialogueFile();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

  QString AddedResource() const;

protected:
  tspProject m_spProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CEditorModel> m_pEditorModel;
  QPointer<QWidget> m_pParent;
  SResourceData m_data;
};

#endif // CCOMMANDADDNEWDIALOGUEFILE_H
