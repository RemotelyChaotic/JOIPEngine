#ifndef CCOMMANDCHANGELAYOUT_H
#define CCOMMANDCHANGELAYOUT_H

#include <QComboBox>
#include <QPointer>
#include <QUndoCommand>

class CCommandChangeLayout : public QUndoCommand
{
public:
  CCommandChangeLayout(QPointer<QComboBox> pComboBox,
                       QUndoCommand* pParent = nullptr);

  ~CCommandChangeLayout();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QComboBox>     m_pComboBox;
  QString                 m_sOriginalValue;
  QString                 m_sNewValue;
};

#endif // CCOMMANDCHANGELAYOUT_H
