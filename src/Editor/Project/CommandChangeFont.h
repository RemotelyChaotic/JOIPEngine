#ifndef CCOMMANDCHANGEFONT_H
#define CCOMMANDCHANGEFONT_H

#include "Systems/Project.h"
#include <QFontComboBox>
#include <QPointer>
#include <QUndoCommand>

class CCommandChangeFont  : public QUndoCommand
{
public:
  CCommandChangeFont(QPointer<QFontComboBox> pFontComboBox,
                     const std::function<void(void)>& fnOnUndoRedo,
                     QUndoCommand* pParent = nullptr);
  ~CCommandChangeFont();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QFontComboBox> m_pFontComboBox;
  std::function<void(void)> m_fnOnUndoRedo;
  QString                 m_sOriginalValue;
  QString                 m_sNewValue;
};

#endif // CCOMMANDCHANGEFONT_H
