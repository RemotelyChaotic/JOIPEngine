#ifndef E6XBOORURESOURCEADDER_H
#define E6XBOORURESOURCEADDER_H

#include "BooruResourceAdder.h"

class CE6xBooruResourceAdder : public CBooruResourceAdder
{
  Q_OBJECT

public:
  CE6xBooruResourceAdder(QObject* pParent = nullptr);
  ~CE6xBooruResourceAdder() override;

  bool CanHandleUrl(const QUrl& url) const override;

  QString Name() const override { return "eXXX Danbooru API"; };

private:
  QUrl GetRequestUrl(const QUrl& url) override;
  void HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile, const QByteArray& arr) override;
  void HandleResponseImage(const SRequestItem& item, const QByteArray& arr) override;
  void NetworkReplyErrorImpl(QNetworkReply::NetworkError code) override;
};

#endif // E6XBOORURESOURCEADDER_H
