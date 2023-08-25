#ifndef RESOURCEMODELVIEW_H
#define RESOURCEMODELVIEW_H

#include "CompressJobSettingsOverlay.h"

#include "Editor/EditorJobs/IEditorJobStateListener.h"

#include "Editor/IEditorTool.h"

#include <QWidget>
#include <QPointer>
#include <memory>

class CCompressJobSettingsOverlay;
class CEditorModel;
class CResourceTreeItemModel;
class CResourceTreeItemSortFilterProxyModel;
namespace Ui {
class CResourceModelView;
}
class QItemSelectionModel;
class QUndoStack;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;


class CResourceModelView : public QWidget,
                           public IEditorToolBox,
                           public IEditorJobStateListener
{
  Q_OBJECT
  Q_PROPERTY(QImage cardIcon READ CardIcon WRITE SetCardIcon NOTIFY SignalCardIconChanged)
  Q_PROPERTY(qint32 cardIconSize READ CardIconSize WRITE SetCardIconSize NOTIFY SignalCardIconSizeChanged)

public:
  enum EView
  {
    eTree = 0,
    eExplorer,
    eBoth
  };

  explicit CResourceModelView(QWidget *parent = nullptr);
  ~CResourceModelView();

  void Initialize(CEditorModel* pEditorModel, QUndoStack* pStack, CResourceTreeItemModel* pModel);
  void ProjectLoaded(tspProject spCurrentProject, bool bReadOnly);
  void ProjectUnloaded();
  void CdUp();

  std::vector<QPointer<QItemSelectionModel>> SelectionModels() const;
  QPointer<CResourceTreeItemSortFilterProxyModel> Proxy() const { return m_pProxy; }
  QStringList SelectedResources() const;

  void SetView(EView view);
  EView View() const;
  void SetLandscape(bool bLandscape);
  void ShowContextMenu(CResourceTreeItemModel* pModel, const QModelIndex& idx,
                       const QPoint& globalPos);
  bool Landscape();

  const QImage& CardIcon() const { return m_cardIcon; }
  void SetCardIcon(const QImage& img);
  qint32 CardIconSize() const { return m_cardIconSize; }
  void SetCardIconSize(qint32 iValue);

  QStringList Tools() const override;
  void ToolTriggered(const QString& sTool) override;

  void JobFinished(qint32 iId) override;
  void JobStarted(qint32 iId) override;
  void JobMessage(qint32 iId, const QString& sMsg) override;
  void JobProgressChanged(qint32 iId, qint32 iProgress) override;

signals:
  void SignalCardIconChanged();
  void SignalCardIconSizeChanged();
  void SignalProjectEdited();
  void SignalResourceSelected(const QString& sName);

protected slots:
  void SlotExpanded(const QModelIndex& index);
  void SlotCollapsed(const QModelIndex& index);
  void SlotCurrentChanged(const QModelIndex& current,
                          const QModelIndex& previous);
  void SlotJobSettingsConfirmed(const CCompressJobSettingsOverlay::SCompressJobSettings& settings);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  std::unique_ptr<Ui::CResourceModelView>         m_spUi;
  tspProject                                      m_spCurrentProject;
  QPointer<CCompressJobSettingsOverlay>           m_pCompressJobOverlay;
  QPointer<CEditorModel>                          m_pEditorModel;
  QPointer<CResourceTreeItemSortFilterProxyModel> m_pProxy;
  QPointer<CResourceTreeItemModel>                m_pModel;
  QPointer<QUndoStack>                            m_pStack;
  QImage                                          m_cardIcon;
  QString                                         m_sCurrentResourceConversion;
  qint32                                          m_cardIconSize;
  bool                                            m_bInitializing;
  bool                                            m_bLandscape;
};

#endif // RESOURCEMODELVIEW_H
