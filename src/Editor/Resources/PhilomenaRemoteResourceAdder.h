#ifndef PHILOMENAREMOTERESOURCEADDER_H
#define PHILOMENAREMOTERESOURCEADDER_H

#include "BooruResourceAdder.h"

class CPhilomenaRemoteResourceAdder : public CBooruResourceAdder
{
  Q_OBJECT
public:
  explicit CPhilomenaRemoteResourceAdder(QObject* pParent = nullptr);
  ~CPhilomenaRemoteResourceAdder() override;

  bool CanHandleUrl(const QUrl& url) const override;

  QString Name() const override { return "Philomena API"; };

private:
  QUrl GetRequestUrl(const QUrl& url) override;
  void HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile, const QByteArray& arr) override;
  void HandleResponseImage(const SRequestItem& item, const QByteArray& arr) override;
  void NetworkReplyErrorImpl(QNetworkReply::NetworkError code) override;
};

#endif // PHILOMENAREMOTERESOURCEADDER_H
