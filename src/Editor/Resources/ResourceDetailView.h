#ifndef CRESOURCEDETAILVIEW_H
#define CRESOURCEDETAILVIEW_H

#include "Systems/Project.h"
#include <QListView>
#include <QPixmap>
#include <map>
#include <memory>

class CResourceDetailViewFetcherThread;
class CThreadedSystem;

class CResourceDetailView : public QListView
{
  Q_OBJECT
  Q_PROPERTY(QIcon iconFile   READ IconFile   WRITE SetIconFile)
  Q_PROPERTY(QIcon iconFolder READ IconFolder WRITE SetIconFolder)

public:
  explicit CResourceDetailView(QWidget* pParent = nullptr);
  ~CResourceDetailView();

  void Initialize(tspProject spProject);
  void DeInitilaze();

  void SetIconFile(const QIcon& icon);
  const QIcon& IconFile() const;
  void SetIconFolder(const QIcon& icon);
  const QIcon& IconFolder() const;
  void SetReadOnly(bool bReadOnly);
  bool ReadOnly();

  const QPixmap& PreviewImageForResource(const QString& sResource) const;
  void RequestResource(const QModelIndex& index);

public slots:
  void Expand(const QModelIndex& index);
  void Collapse(const QModelIndex& index);
  void UpdateResources();

signals:
  void Expanded(const QModelIndex& index);
  void Collapsed(const QModelIndex& index);

protected slots:
  void SlotDoubleClicked(const QModelIndex& index);
  void SlotResourceLoadFinished(const QString& sResource, const QPixmap& pixmap);

protected:
  void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& viRoles = QVector<int>()) override;
  bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* pEvent) override;

private:
  void RequestResourcesFromCurrentFolder();
  std::shared_ptr<CResourceDetailViewFetcherThread> ResourceFetcher() const;

  std::unique_ptr<CThreadedSystem>            m_spThreadedLoader;
  tspProject                                  m_spProject;
  std::map<QString, QPixmap>                  m_imageCache;
  bool                                        m_bReadOnly;
  QIcon                                       m_iconFile;
  QIcon                                       m_iconFolder;
};

#endif // CRESOURCEDETAILVIEW_H
