#ifndef CCOMMANDCHANGEOPENEDFLOW_H
#define CCOMMANDCHANGEOPENEDFLOW_H

#include "Systems/Database/Project.h"
#include <QComboBox>
#include <QUndoCommand>
#include <QPointer>
#include <functional>

class CDatabaseManager;
class CEditorEditableFileModel;
class CNodeEditorFlowScene;

class CCommandChangeOpenedFlow : public QUndoCommand
{
public:
  CCommandChangeOpenedFlow(QPointer<QComboBox> pResourcesComboBox,
                           QPointer<CNodeEditorFlowScene> pScene,
                           QPointer<QWidget> pGuard,
                           const std::function<void(qint32)>& fnReloadEditor,
                           bool* pbChangingIndexFlag,
                           QString* psLastCachedScript,
                           const QString& sOldFlow,
                           const QString& sNewFlow,
                           QUndoCommand* pParent = nullptr);
  ~CCommandChangeOpenedFlow();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void DoUndoRedo(const QString& sSequenceNext);

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<QComboBox> m_pResourcesComboBox;
  QPointer<CNodeEditorFlowScene> m_pScene;
  QPointer<CEditorEditableFileModel> m_pEditorModel;
  QPointer<QWidget> m_pGuard;
  std::function<void(qint32)> m_fnReloadEditor;
  bool* m_pbChangingIndexFlag;
  QString* m_psLastCachedScript;
  QString m_sOldFlow;
  QString m_sNewFlow;
};

#endif // CCOMMANDCHANGEOPENEDFLOW_H
