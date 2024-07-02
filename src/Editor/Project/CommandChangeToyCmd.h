#ifndef CCOMMANDCHANGETOYCMD_H
#define CCOMMANDCHANGETOYCMD_H

#include <QComboBox>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

typedef std::shared_ptr<struct SProject>      tspProject;

class CCommandChangeToyCmd : public QUndoCommand
{
public:
  CCommandChangeToyCmd(QPointer<QComboBox> pComboBox,
                       const tspProject& spCurrentProject,
                       const std::function<void(void)>& fnOnUndoRedo,
                       QUndoCommand* pParent = nullptr);

  ~CCommandChangeToyCmd();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QComboBox>     m_pComboBox;
  std::function<void(void)> m_fnOnUndoRedo;
  tspProject              m_spCurrentProject;
  qint32                  m_iOriginalValue;
  qint32                  m_iNewValue;
};

#endif // CCOMMANDCHANGETOYCMD_H
