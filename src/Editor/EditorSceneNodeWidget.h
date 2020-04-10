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
struct SScene;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;

class CEditorSceneNodeWidget : public CEditorWidgetBase
{
  Q_OBJECT
  Q_PROPERTY(QColor normalBoundaryColor         MEMBER m_normalBoundaryColor)
  Q_PROPERTY(QColor selectedBoundaryColor       MEMBER m_selectedBoundaryColor)
  Q_PROPERTY(QColor gradientColor0              MEMBER m_gradientColor0)
  Q_PROPERTY(QColor gradientColor1              MEMBER m_gradientColor1)
  Q_PROPERTY(QColor gradientColor2              MEMBER m_gradientColor2)
  Q_PROPERTY(QColor gradientColor3              MEMBER m_gradientColor3)
  Q_PROPERTY(QColor shadowColor                 MEMBER m_shadowColor)
  Q_PROPERTY(QColor fontColor                   MEMBER m_fontColor)
  Q_PROPERTY(QColor fontColorFaded              MEMBER m_fontColorFaded)
  Q_PROPERTY(QColor connectionPointColor        MEMBER m_connectionPointColor)
  Q_PROPERTY(QColor backgroundColor             MEMBER m_backgroundColor)
  Q_PROPERTY(QColor fineGridColor               MEMBER m_fineGridColor)
  Q_PROPERTY(QColor coarseGridColor             MEMBER m_coarseGridColor)
  Q_PROPERTY(QColor normalConnectionColor       MEMBER m_normalColor)
  Q_PROPERTY(QColor selectedConnectionColor     MEMBER m_selectedColor)
  Q_PROPERTY(QColor selectedConnectionHaloColor MEMBER m_selectedHaloColor)
  Q_PROPERTY(QColor hoveredConnectionColor      MEMBER m_hoveredColor)

public:
  explicit CEditorSceneNodeWidget(QWidget* pParent = nullptr);
  ~CEditorSceneNodeWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spCurrentProject) override;
  void UnloadProject() override;
  void SaveProject() override;

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void SlotAddSceneButtonClicked();
  void SlotRemoveNodeButtonClicked();
  void SlotStyleChanged();

private:
  std::unique_ptr<Ui::CEditorSceneNodeWidget>       m_spUi;
  std::shared_ptr<CSettings>                        m_spSettings;
  tspProject                                        m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                   m_wpDbManager;
  CFlowView*                                        m_pFlowView;

  QColor                                            m_normalBoundaryColor;
  QColor                                            m_selectedBoundaryColor;
  QColor                                            m_gradientColor0;
  QColor                                            m_gradientColor1;
  QColor                                            m_gradientColor2;
  QColor                                            m_gradientColor3;
  QColor                                            m_shadowColor;
  QColor                                            m_fontColor;
  QColor                                            m_fontColorFaded;
  QColor                                            m_connectionPointColor;

  QColor                                            m_backgroundColor;
  QColor                                            m_fineGridColor;
  QColor                                            m_coarseGridColor;

  QColor                                            m_normalColor;
  QColor                                            m_selectedColor;
  QColor                                            m_selectedHaloColor;
  QColor                                            m_hoveredColor;
};

#endif // SCENENODEWIDGET_H
