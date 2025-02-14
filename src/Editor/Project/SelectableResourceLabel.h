#ifndef CSELECTABLERESOURCELABEL_H
#define CSELECTABLERESOURCELABEL_H

#include <QIcon>
#include <QLabel>
#include <QPointer>

#include <memory>

class CResourceDetailViewFetcherThread;
class CResourceTreeItemModel;
class CThreadedSystem;
typedef std::shared_ptr<struct SProject> tspProject;

class CSelectableResourceLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(QIcon unsetIcon READ UnsetIcon WRITE SetUnsetIcon NOTIFY SignalUnsetIconChanged);

public:
  explicit CSelectableResourceLabel(QWidget* pParent = nullptr);
  ~CSelectableResourceLabel() override;

  void SetCurrentProject(const tspProject& spProj);
  void SetResourceModel(QPointer<CResourceTreeItemModel> pResourceModel);
  QString CurrentResource() const;
  void SetCurrentResource(const QString& sResource);
  const QIcon& UnsetIcon() const;
  void SetUnsetIcon(const QIcon& icon);
  std::shared_ptr<CResourceDetailViewFetcherThread> ResourceFetcher() const;

signals:
  void SignalResourcePicked(const QString& sOld, const QString& sNew);
  void SignalUnsetIconChanged();

protected slots:
  void SlotResourceLoadFinished(const QString& sName, const QPixmap& pixmap);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void OpenSelectResource(const QPoint& createPoint);
  void UpdateResource();

private:
  std::unique_ptr<CThreadedSystem> m_spThreadedLoader;
  tspProject                       m_spCurrentProject;
  QPointer<CResourceTreeItemModel> m_pResourceModel;
  QIcon                            m_unsetIcon;
  QString                          m_sCurrentResource;
};

#endif // CSELECTABLERESOURCELABEL_H
