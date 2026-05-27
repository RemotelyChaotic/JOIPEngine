#ifndef COMMONREMOTERESOURCEADDER_H
#define COMMONREMOTERESOURCEADDER_H

#include "IRemoteResourceAdder.h"

#include <QObject>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class CCommonRemoteResourceAdder : public QObject, public IRemoteResourceAdder
{
  Q_OBJECT

public:
  explicit CCommonRemoteResourceAdder(QObject* pParent = nullptr);
  ~CCommonRemoteResourceAdder() override;

  void AddResource(const QUrl& url, bool bDownloadAndAddAsFile = false) override;

  bool CanDownloadAndSaveAsFile() const override;

  bool CanHandleUrl(const QUrl& url) const override;

  QString Name() const override { return "Generic Downloader"; };

  void SetNetworkAccessManager(QNetworkAccessManager* pNAManager);

signals:
  void SignalNewResourceFile(const SResourceData& res, const std::vector<STagData>& vsTags,
                             const QByteArray& ba, bool bAddAsFile) override;

private slots:
  void SlotNetworkReplyError(QNetworkReply::NetworkError code);
  void SlotNetworkReplyFinished();

private:
  QPointer<QNetworkAccessManager>      m_pNAManager;
  std::vector<QPointer<QNetworkReply>> m_vpResponses;
};

#endif // COMMONREMOTERESOURCEADDER_H
