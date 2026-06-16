#ifndef COMMANDCHANGECUSTOMENGINEVERSION_H
#define COMMANDCHANGECUSTOMENGINEVERSION_H

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CCommandChangeCustomEngineVersion : public QUndoCommand
{
public:
  CCommandChangeCustomEngineVersion(QPointer<QCheckBox> pCheckBox,
                                    QPointer<QLabel> pWarningLabel,
                                    QList<QPointer<QWidget>> vpWidgetsToEnable,
                                    std::function<void(void)> fnOnUndoRedo,
                                    QUndoCommand* pParent = nullptr);
  ~CCommandChangeCustomEngineVersion() override;

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void SetNewValue(bool bValue);

  QList<QPointer<QWidget>>  m_vpWidgetsToEnable;
  QPointer<QCheckBox>       m_pCheckBox;
  QPointer<QLabel>          m_pWarningLabel;
  std::function<void(void)> m_fnOnUndoRedo;
  bool                      m_bOldValue;
  bool                      m_bNewValue;
};

#endif // COMMANDCHANGECUSTOMENGINEVERSION_H
