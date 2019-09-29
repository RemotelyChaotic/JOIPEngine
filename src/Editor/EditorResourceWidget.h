#ifndef EDITORRESOURCEWIDGET_H
#define EDITORRESOURCEWIDGET_H

#include "EditorWidgetBase.h"
#include <QNetworkReply>
#include <QPointer>
#include <memory>


namespace Ui {
  class CEditorResourceWidget;
}
class CDatabaseManager;
class CSettings;
class CWebResourceOverlay;
class QNetworkAccessManager;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorResourceWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorResourceWidget(QWidget* pParent = nullptr);
  ~CEditorResourceWidget() override;

  void Initialize() override;
  void LoadProject(tspProject spCurrentProject) override;
  void UnloadProject() override;
  void SaveProject() override {}

signals:
  void SignalResourceSelected(const QString& sName);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;

protected slots:
  void on_pFilterLineEdit_editingFinished();
  void on_pResourceDisplayWidget_OnClick();
  void SlotAddButtonClicked();
  void SlotAddWebButtonClicked();
  void SlotRemoveButtonClicked();
  void SlotTitleCardButtonClicked();
  void SlotMapButtonClicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
  void SlotWebResourceSelected(const QString& sResource);
  void SlotNetworkReplyError(QNetworkReply::NetworkError code);
  void SlotNetworkReplyFinished();

private:
  void AddFiles(const QStringList& vsFiles, const QStringList& imageFormatsList,
                const QStringList& videoFormatsList, const QStringList& audioFormatsList,
                const QStringList& otherFormatsList);

  std::unique_ptr<Ui::CEditorResourceWidget> m_spUi;
  std::unique_ptr<CWebResourceOverlay>       m_spWebOverlay;
  std::unique_ptr<QNetworkAccessManager>     m_spNAManager;
  std::shared_ptr<CSettings>                 m_spSettings;
  tspProject                                 m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  QPointer<QNetworkReply>                    m_pResponse;
};

#endif // EDITORRESOURCEWIDGET_H
