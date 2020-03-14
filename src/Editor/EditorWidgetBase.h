#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CEditorActionBar;
class CEditorModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
namespace QtNodes {
  class FlowScene;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorWidgetBase : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT
public:
  explicit CEditorWidgetBase(QWidget* pParent = nullptr);
  ~CEditorWidgetBase() override;

  void Initialize() override = 0;
  virtual void LoadProject(tspProject spProject) = 0;
  virtual void UnloadProject() = 0;
  virtual void SaveProject() = 0;

  void SetActionBar(CEditorActionBar* pActionBar);
  void SetEditorModel(CEditorModel* pItemModel);

signals:
  void SignalProjectEdited();

protected:
  virtual void OnActionBarAboutToChange() {}
  virtual void OnActionBarChanged() {}

  CEditorActionBar* ActionBar() const;
  CEditorModel* EditorModel() const;
  QtNodes::FlowScene* FlowSceneModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CScriptEditorModel* ScriptEditorModel() const;

  CEditorActionBar*                                m_pActionBar;
  CEditorModel*                                    m_pEditorModel;
};

#endif // EDITORWIDGETBASE_H
