#ifndef CCOMMANDCHANGEVERSION_H
#define CCOMMANDCHANGEVERSION_H

#include "Systems/Database/Project.h"
#include <QPointer>
#include <QSpinBox>
#include <QUndoCommand>

class CCommandChangeVersion : public QUndoCommand
{
public:
  CCommandChangeVersion(QPointer<QSpinBox> pProjectMajorVersion,
                        QPointer<QSpinBox> pProjectMinorVersion,
                        QPointer<QSpinBox> pProjectPatchVersion,
                        const std::function<void(void)>& fnOnUndoRedo,
                        QUndoCommand* pParent = nullptr);
  ~CCommandChangeVersion();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void ApplyValue(const SVersion& value);

  QPointer<QSpinBox> m_pProjectMajorVersion;
  QPointer<QSpinBox> m_pProjectMinorVersion;
  QPointer<QSpinBox> m_pProjectPatchVersion;
  std::function<void(void)> m_fnOnUndoRedo;
  SVersion           m_originalVersion;
  SVersion           m_newVersion;
};

#endif // CCOMMANDCHANGEVERSION_H
