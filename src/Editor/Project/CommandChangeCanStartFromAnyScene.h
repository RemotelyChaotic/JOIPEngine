#ifndef CCOMMANDCHANGECANSTARTFROMANYSCENE_H
#define CCOMMANDCHANGECANSTARTFROMANYSCENE_H

#include <QCheckBox>
#include <QPointer>
#include <QUndoCommand>
#include <functional>

class CCommandChangeCanStartFromAnyScene : public QUndoCommand
{
public:
  CCommandChangeCanStartFromAnyScene(QPointer<QCheckBox> pCheckBox,
                                     const std::function<void(void)>& fnOnUndoRedo,
                                     QUndoCommand* pParent = nullptr);
  ~CCommandChangeCanStartFromAnyScene();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QCheckBox> m_pCanStartFromAnySceneCheckBox;
  std::function<void(void)> m_fnOnUndoRedo;
  bool             m_bOriginalValue;
  bool             m_bNewValue;
};

#endif // CCOMMANDCHANGECANSTARTFROMANYSCENE_H
