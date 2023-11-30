#ifndef CCOMMANDCHANGELAYOUT_H
#define CCOMMANDCHANGELAYOUT_H

#include <QComboBox>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

struct SProject;
typedef std::shared_ptr<SProject>      tspProject;

class CCommandChangeLayout : public QUndoCommand
{
public:
  CCommandChangeLayout(QPointer<QComboBox> pComboBox,
                       const tspProject& spCurrentProject,
                       const std::function<void(void)>& fnOnUndoRedo,
                       QUndoCommand* pParent = nullptr);

  ~CCommandChangeLayout();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QComboBox>     m_pComboBox;
  std::function<void(void)> m_fnOnUndoRedo;
  tspProject              m_spCurrentProject;
  QString                 m_sOriginalValue;
  QString                 m_sNewValue;
};

#endif // CCOMMANDCHANGELAYOUT_H
