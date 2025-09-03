#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include "Widgets/IWidgetBaseInterface.h"
#include "Editor/EditorWidgetRegistry.h"
#include <QIcon>
#include <QPointer>
#include <QWidget>
#include <memory>

class CEditorActionBar;
class CDialogueEditorTreeModel;
class CNodeEditorFlowScene;
class CEditorModel;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CEditorEditableFileModel;
class QUndoStack;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorWidgetBase : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT
  Q_PROPERTY(QIcon icon READ Icon WRITE SetIcon)

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
  QIcon Icon() const { return m_icon; }
  void SetActionBar(CEditorActionBar* pActionBar);
  void SetEditorModel(CEditorModel* pItemModel);
  void SetIcon(const QIcon& icon);
  void TakeFromLayout();

signals:
  void SignalProjectEdited();
  void SignalUnloadFinished();

protected:
  virtual void OnActionBarAboutToChange() {}
  virtual void OnActionBarChanged() {}

  QPointer<CEditorActionBar> ActionBar() const;
  QPointer<CDialogueEditorTreeModel> DialogueModel() const;
  QPointer<CEditorModel> EditorModel() const;
  QPointer<CNodeEditorFlowScene> FlowSceneModel() const;
  QPointer<CKinkTreeModel> KinkModel() const;
  QPointer<CResourceTreeItemModel> ResourceTreeModel() const;
  QPointer<CEditorEditableFileModel> EditableFileModel() const;
  QPointer<QUndoStack> UndoStack() const;

  void SetLoaded(bool bLoaded);

  QPointer<QWidget>                                m_pOriginalParent;
  QPointer<CEditorActionBar>                       m_pActionBar;
  QPointer<CEditorModel>                           m_pEditorModel;
  QIcon                                            m_icon;
  bool                                             m_bLoaded;
};

#endif // EDITORWIDGETBASE_H
