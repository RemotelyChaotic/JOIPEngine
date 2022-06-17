#ifndef CTEXTEDITZOOMENABLER_H
#define CTEXTEDITZOOMENABLER_H

#include "Widgets/OverlayBase.h"
#include <QLabel>
#include <QPointer>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <variant>

class CTextEditZoomEnabler : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 innerMargin READ InnerMargin WRITE SetInnerMargin CONSTANT)

public:
  CTextEditZoomEnabler(QPointer<QTextEdit> pEditor);
  CTextEditZoomEnabler(QPointer<QPlainTextEdit> pEditor);
  ~CTextEditZoomEnabler() override;

  void SetInnerMargin(qint32 iInnerMargin) { m_iInnerMargin = iInnerMargin; }
  qint32 InnerMargin() const { return m_iInnerMargin; }

public slots:
  void Climb() override;
  void Resize() override;
  void Show() override;
  void SetZoom(double dZoom);

protected:
  bool event(QEvent* pEvent) override;
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  QPointer<QLabel>                                            m_pLabel;
  std::variant<QPointer<QTextEdit>, QPointer<QPlainTextEdit>> m_pEditor;
  double                                                      m_dZoomLevel = 1.0;
  qint32                                                      m_iInnerMargin = 9;
};

#endif // CTEXTEDITZOOMENABLER_H
