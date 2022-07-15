#ifndef RESOURCEMODELVIEW_H
#define RESOURCEMODELVIEW_H

#include <QWidget>
#include <QPointer>
#include <memory>

class CResourceTreeItemModel;
class CResourceTreeItemSortFilterProxyModel;
namespace Ui {
class CResourceModelView;
}
class QItemSelectionModel;
class QUndoStack;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;


class CResourceModelView : public QWidget
{
  Q_OBJECT

public:
  enum EView
  {
    eTree = 0,
    eExplorer,
    eBoth
  };

  explicit CResourceModelView(QWidget *parent = nullptr);
  ~CResourceModelView();

  void Initialize(QUndoStack* pStack, CResourceTreeItemModel* pModel);
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

signals:
  void SignalResourceSelected(const QString& sName);

protected slots:
  void SlotExpanded(const QModelIndex& index);
  void SlotCollapsed(const QModelIndex& index);
  void SlotCurrentChanged(const QModelIndex& current,
                          const QModelIndex& previous);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  std::unique_ptr<Ui::CResourceModelView>         m_spUi;
  tspProject                                      m_spCurrentProject;
  QPointer<CResourceTreeItemSortFilterProxyModel> m_pProxy;
  QPointer<CResourceTreeItemModel>                m_pModel;
  QPointer<QUndoStack>                            m_pStack;
  bool                                            m_bInitializing;
  bool                                            m_bLandscape;
};

#endif // RESOURCEMODELVIEW_H
