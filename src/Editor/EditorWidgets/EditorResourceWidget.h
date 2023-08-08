#ifndef EDITORRESOURCEWIDGET_H
#define EDITORRESOURCEWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorResourceWidget.h"
#include "ui_EditorActionBar.h"
#include <QNetworkReply>
#include <QPointer>
#include <memory>


class CResourceTutorialStateSwitchHandler;
namespace Ui {
  class CEditorResourceWidget;
}
class CDatabaseManager;
class CSettings;
class CTagsEditorOverlay;
class CWebResourceOverlay;
class QNetworkAccessManager;
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
  void UnloadProject() override;
  void SaveProject() override {}
  void OnHidden() override {}
  void OnShown() override {}

signals:
  void SignalResourceSelected(const QString& sName);

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
  void SlotWebResourceSelected(const QString& sResource);
  void SlotWebSourceSelected(const QString& sResource);
  void SlotNetworkReplyError(QNetworkReply::NetworkError code);
  void SlotNetworkReplyFinished();
  void SlotViewResourceSelected(const QString& sResource);

private:
  void HandleResize(bool bForceUpdate);

  std::unique_ptr<CWebResourceOverlay>                 m_spSourceOverlay;
  std::unique_ptr<CWebResourceOverlay>                 m_spWebOverlay;
  std::unique_ptr<QNetworkAccessManager>               m_spNAManager;
  std::unique_ptr<CTagsEditorOverlay>                  m_spTagsOverlay;
  std::shared_ptr<Ui::CEditorResourceWidget>           m_spUi;
  std::shared_ptr<CResourceTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::shared_ptr<CSettings>                           m_spSettings;
  tspProject                                           m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                      m_wpDbManager;
  QPointer<QNetworkReply>                              m_pResponse;
};

#endif // EDITORRESOURCEWIDGET_H
