#ifndef WEBRESOURCEDOWNLOADMANAGER_H
#define WEBRESOURCEDOWNLOADMANAGER_H

#include "IRemoteResourceAdder.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QUndoStack>

#include <memory>
#include <vector>

struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CWebResourceDownloadManager : public QObject
{
  Q_OBJECT
public:
  explicit CWebResourceDownloadManager(QWidget* pParent);
  ~CWebResourceDownloadManager() override;

  void AddResource(const QUrl& sPath, bool bDownloadAndAddAsFile = false);
  bool CanDownloadAndSaveAsFile(const QUrl& url) const;

  void SetCurrentProject(const tspProject& spProject);
  void SetUndostack(QPointer<QUndoStack> pStack);

protected slots:
  void SlotNewResourceFile(const QUrl& url, const QByteArray& ba, bool bAddAsFile);

private:
  std::vector<std::shared_ptr<IRemoteResourceAdder>> m_vspResourceAdders;
  std::unique_ptr<QNetworkAccessManager>             m_spNAManager;
  tspProject                                         m_spCurrentProject;
  QPointer<QWidget>                                  m_pParent;
  QPointer<QUndoStack>                               m_pUndoStack;
};

#endif // WEBRESOURCEDOWNLOADMANAGER_H
