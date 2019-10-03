#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CEditorActionBar;
class CResourceTreeItemModel;
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
  void SetResourceModel(CResourceTreeItemModel* pItemModel);

protected:
  virtual void OnActionBarAboutToChange() {}
  virtual void OnActionBarChanged() {}

  CEditorActionBar* ActionBar();
  CResourceTreeItemModel* ResourceModel();

  CEditorActionBar*                                m_pActionBar;
  CResourceTreeItemModel*                          m_pResourceTreeModel;
};

#endif // EDITORWIDGETBASE_H
