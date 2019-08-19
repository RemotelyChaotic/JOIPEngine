#ifndef SCENENODEWIDGET_H
#define SCENENODEWIDGET_H

#include "EditorWidgetBase.h"
#include <memory>

class CDatabaseManager;
class CFlowView;
class CSettings;
namespace Ui {
  class CEditorSceneNodeWidget;
}
namespace QtNodes {
  class DataModelRegistry;
  class FlowScene;
  class Node;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorSceneNodeWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorSceneNodeWidget(QWidget* pParent = nullptr);
  ~CEditorSceneNodeWidget() override;

  void Initialize() override;

  void LoadProject(tspProject spCurrentProject);
  void UnloadProject();
  void SaveNodeLayout();

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void SlotAddSceneButtonClicked();
  void SlotNodeCreated(QtNodes::Node &n);
  void SlotRemoveNodeButtonClicked();

private:
  std::unique_ptr<Ui::CEditorSceneNodeWidget>       m_spUi;
  std::shared_ptr<CSettings>                        m_spSettings;
  std::shared_ptr<QtNodes::DataModelRegistry>       m_spDataModelRegistry;
  tspProject                                        m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                   m_wpDbManager;
  CFlowView*                                        m_pFlowView;
  QtNodes::FlowScene*                               m_pFlowScene;
};

#endif // SCENENODEWIDGET_H
