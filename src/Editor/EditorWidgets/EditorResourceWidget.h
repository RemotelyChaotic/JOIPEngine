#ifndef EDITORRESOURCEWIDGET_H
#define EDITORRESOURCEWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorResourceWidget.h"
#include "ui_EditorActionBar.h"
#include <QNetworkReply>
#include <QPointer>

#include <memory>
#include <optional>


class CResourceTutorialStateSwitchHandler;
namespace Ui {
  class CEditorResourceWidget;
}
class CDatabaseManager;
class CSettings;
class CTagsEditorOverlay;
class CWebResourceOverlay;
class CWebResourceDownloadManager;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorResourceWidget : public CEditorWidgetBase
{
  Q_OBJECT
  friend class CResourceTutorialStateSwitchHandler;

public:
  explicit CEditorResourceWidget(QWidget* pParent = nullptr);
  ~CEditorResourceWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spCurrentProject) override;
  void LoadResource(tspResource, bool) override {};
  void UnloadProject() override;
  void SaveProject() override {}
  std::vector<EResourceType> SupportedDisplayingResources() override { return {}; }
  void OnHidden() override {}
  void OnShown() override {}

signals:
  void SignalResourceSelected(const QString& sName, bool bSpontanious);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

protected slots:
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pResourceDisplayWidget_OnClick();
  void on_pResourceDisplayWidget_SignalLoadFinished();
  void SlotAddButtonClicked();
  void SlotAddWebButtonClicked();
  void SlotChangeViewButtonClicked();
  void SlotCdUpClicked();
  void SlotRemoveButtonClicked();
  void SlotSetSourceButtonClicked();
  void SlotTitleCardButtonClicked();
  void SlotTagsButtonClicked();
  void SlotMapButtonClicked();
  void SlotWebSourceSelected(const QString& sResource);
  void SlotViewResourceSelected(const QString& sResource, bool bSpontanious);

private:
  void HandleResize(bool bForceUpdate);

  std::shared_ptr<Ui::CEditorResourceWidget>           m_spUi;
  std::shared_ptr<CWebResourceDownloadManager>         m_spDownloadManager;
  std::shared_ptr<CResourceTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::shared_ptr<CSettings>                           m_spSettings;
  tspProject                                           m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                      m_wpDbManager;
  QPointer<CWebResourceOverlay>                        m_spSourceOverlay;
  QPointer<CWebResourceOverlay>                        m_spWebOverlay;
  QPointer<CTagsEditorOverlay>                         m_spTagsOverlay;
  std::optional<bool>                                  m_optbDownloadDroppedFiles = std::nullopt;
};

#endif // EDITORRESOURCEWIDGET_H
