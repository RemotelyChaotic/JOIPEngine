#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include "Widgets/IWidgetBaseInterface.h"
#include "Editor/EditorWidgetRegistry.h"
#include <QPointer>
#include <QWidget>
#include <memory>

class CEditorActionBar;
class CFlowScene;
class CEditorModel;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
class QUndoStack;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorWidgetBase : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT
public:
  explicit CEditorWidgetBase(QWidget* pParent = nullptr);
  ~CEditorWidgetBase() override;

  void Initialize() override = 0;
  virtual void EditedProject() = 0;
  virtual void LoadProject(tspProject spProject) = 0;
  virtual void UnloadProject() = 0;
  virtual void SaveProject() = 0;
  virtual void OnHidden() = 0;
  virtual void OnShown() = 0;

  bool IsLoaded() { return m_bLoaded; }
  void SetActionBar(CEditorActionBar* pActionBar);
  void SetEditorModel(CEditorModel* pItemModel);
  void TakeFromLayout();

signals:
  void SignalProjectEdited();
  void SignalUnloadFinished();

protected:
  virtual void OnActionBarAboutToChange() {}
  virtual void OnActionBarChanged() {}

  QPointer<CEditorActionBar> ActionBar() const;
  QPointer<CEditorModel> EditorModel() const;
  QPointer<CFlowScene> FlowSceneModel() const;
  QPointer<CKinkTreeModel> KinkModel() const;
  QPointer<CResourceTreeItemModel> ResourceTreeModel() const;
  QPointer<CScriptEditorModel> ScriptEditorModel() const;
  QPointer<QUndoStack> UndoStack() const;

  void SetLoaded(bool bLoaded);

  QPointer<QWidget>                                m_pOriginalParent;
  QPointer<CEditorActionBar>                       m_pActionBar;
  QPointer<CEditorModel>                           m_pEditorModel;
  bool                                             m_bLoaded;
};

#endif // EDITORWIDGETBASE_H