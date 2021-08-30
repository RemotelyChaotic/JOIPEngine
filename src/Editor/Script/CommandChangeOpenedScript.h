#ifndef CCOMMANDCHANGEOPENEDSCRIPT_H
#define CCOMMANDCHANGEOPENEDSCRIPT_H

#include "Systems/Project.h"
#include <QComboBox>
#include <QUndoCommand>
#include <QPointer>
#include <functional>

class CDatabaseManager;
class CScriptEditorModel;
class CScriptEditorWidget;

class CCommandChangeOpenedScript : public QUndoCommand
{
public:
  CCommandChangeOpenedScript(QPointer<QComboBox> pResourcesComboBox,
                             QPointer<CScriptEditorWidget> pEditorWidget,
                             QPointer<QWidget> pGuard,
                             const std::function<void(qint32)>& fnReloadEditor,
                             bool* pbChangingIndexFlag,
                             QString* psLastCachedScript,
                             const QString& sOldScript,
                             const QString& sNewScript,
                             QUndoCommand* pParent = nullptr);
  ~CCommandChangeOpenedScript();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<QComboBox> m_pResourcesComboBox;
  QPointer<CScriptEditorWidget> m_pEditorWidget;
  QPointer<CScriptEditorModel> m_pEditorModel;
  QPointer<QWidget> m_pGuard;
  std::function<void(qint32)> m_fnReloadEditor;
  bool* m_pbChangingIndexFlag;
  QString* m_psLastCachedScript;
  QString m_sOldScript;
  QString m_sNewScript;

private:
  void DoUndoRedo(const QString& sScriptNext);
};

#endif // CCOMMANDCHANGEOPENEDSCRIPT_H
