#ifndef CCOMMANDCHANGEOPENEDSEQUENCE_H
#define CCOMMANDCHANGEOPENEDSEQUENCE_H

#include "Systems/Database/Project.h"
#include <QComboBox>
#include <QUndoCommand>
#include <QPointer>
#include <functional>

class CDatabaseManager;
class CEditorEditableFileModel;
class CTimelineWidget;

class CCommandChangeOpenedSequence : public QUndoCommand
{
public:
  CCommandChangeOpenedSequence(QPointer<QComboBox> pResourcesComboBox,
                               QPointer<CTimelineWidget> pScriptDisplayWidget,
                               QPointer<QWidget> pGuard,
                               const std::function<void(qint32)>& fnReloadEditor,
                               bool* pbChangingIndexFlag,
                               QString* psLastCachedScript,
                               const QString& sOldSequence,
                               const QString& sNewSequence,
                               QUndoCommand* pParent = nullptr);
  ~CCommandChangeOpenedSequence();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<QComboBox> m_pResourcesComboBox;
  QPointer<CTimelineWidget> m_pTimelineWidget;
  QPointer<CEditorEditableFileModel> m_pEditorModel;
  QPointer<QWidget> m_pGuard;
  std::function<void(qint32)> m_fnReloadEditor;
  bool* m_pbChangingIndexFlag;
  QString* m_psLastCachedScript;
  QString m_sOldSequence;
  QString m_sNewSequence;

private:
  void DoUndoRedo(const QString& sSequenceNext);
};

#endif // CCOMMANDCHANGEOPENEDSEQUENCE_H
