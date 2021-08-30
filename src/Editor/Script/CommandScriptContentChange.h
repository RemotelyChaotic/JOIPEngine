#ifndef CCOMMANDSCRIPTCONTENTCHANGE_H
#define CCOMMANDSCRIPTCONTENTCHANGE_H

#include <QPointer>
#include <QUndoCommand>
#include <QTextDocument>

class CCommandScriptContentChange : public QUndoCommand
{
public:
  CCommandScriptContentChange(QPointer<QTextDocument> pScriptDocument,
                              QUndoCommand* pParent = nullptr);
  ~CCommandScriptContentChange();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QTextDocument> m_pScriptDocument;
  bool                    m_bAddedRedoCommand;
};

#endif // CCOMMANDSCRIPTCONTENTCHANGE_H
