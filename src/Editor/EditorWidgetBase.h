#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CEditorActionBar;
class CEditorModel;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
namespace QtNodes {
  class FlowScene;
}
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

signals:
  void SignalProjectEdited();
  void SignalUnloadFinished();

protected:
  virtual void OnActionBarAboutToChange() {}
  virtual void OnActionBarChanged() {}

  CEditorActionBar* ActionBar() const;
  CEditorModel* EditorModel() const;
  QtNodes::FlowScene* FlowSceneModel() const;
  CKinkTreeModel* KinkModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CScriptEditorModel* ScriptEditorModel() const;
  QUndoStack* UndoStack() const;

  void SetLoaded(bool bLoaded);

  CEditorActionBar*                                m_pActionBar;
  CEditorModel*                                    m_pEditorModel;
  bool                                             m_bLoaded;
};

#endif // EDITORWIDGETBASE_H
