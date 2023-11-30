#ifndef CCOMMANDCHANGEDESCRIBTION_H
#define CCOMMANDCHANGEDESCRIBTION_H

#include <QPointer>
#include <QUndoCommand>
#include <QTextDocument>
#include <functional>

class CCommandChangeDescribtion : public QUndoCommand
{
public:
  CCommandChangeDescribtion(QPointer<QTextDocument> pDescribtionDocument,
                            const std::function<void(void)>& fnOnUndoRedo,
                            QUndoCommand *parent = nullptr);
  ~CCommandChangeDescribtion();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QTextDocument> m_pDescribtionDocument;
  std::function<void(void)> m_fnOnUndoRedo;
  bool                    m_bAddedRedoCommand;
};

#endif // CCOMMANDCHANGEDESCRIBTION_H
