#ifndef DANBOORURESOURCEADDER_H
#define DANBOORURESOURCEADDER_H

#include "BooruResourceAdder.h"

class CDanbooruResourceAdder : public CBooruResourceAdder
{
  Q_OBJECT
public:
  explicit CDanbooruResourceAdder(QObject* pParent = nullptr);
  ~CDanbooruResourceAdder() override;

  bool CanHandleUrl(const QUrl& url) const override;

  QString Name() const override { return "Danbooru API"; };

private:
  QUrl GetRequestUrl(const QUrl& url) override;
  void HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile, const QByteArray& arr) override;
  void HandleResponseImage(const SRequestItem& item, const QByteArray& arr) override;
  void NetworkReplyErrorImpl(QNetworkReply::NetworkError code) override;
};

#endif // DANBOORURESOURCEADDER_H
