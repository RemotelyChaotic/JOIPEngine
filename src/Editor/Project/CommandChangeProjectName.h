#ifndef CCOMMANDCHANGEPROJECTNAME_H
#define CCOMMANDCHANGEPROJECTNAME_H

#include <QUndoCommand>
#include <QLineEdit>
#include <QPointer>

class CEditorModel;

class CCommandChangeProjectName : public QUndoCommand
{
public:
  CCommandChangeProjectName(QPointer<QLineEdit> pNameLineEdit,
                            QPointer<CEditorModel> pEditorModel,
                            QUndoCommand* pParent = nullptr);
  ~CCommandChangeProjectName();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QLineEdit>    m_pNameLineEdit;
  QPointer<CEditorModel> m_pEditorModel;
  QString                m_sOldName;
  QString                m_sNewName;
};

#endif // CCOMMANDCHANGEPROJECTNAME_H
