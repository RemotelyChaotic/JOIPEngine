#ifndef CSCRIPTSEARCHBAR_H
#define CSCRIPTSEARCHBAR_H

#include "Widgets/OverlayBase.h"
#include <QPointer>

class CSearchWidget;
class QPushButton;

class CScriptSearchBar : public COverlayBase
{
  Q_OBJECT
public:
  explicit CScriptSearchBar(QWidget* pParent = nullptr);
  ~CScriptSearchBar() override;

  bool IsSearchingForward() { return m_bForward; }
  void SetFilter(const QString& sString);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;

signals:
  void SignalFilterChanged(bool bForward, const QString& sText);
  void SignalHidden();

private:
  void SlotBackClicked();
  void SlotForwardClicked();
  void SlotFilterChanged(const QString& sText);

  QPointer<CSearchWidget> m_pSearchWidget;
  QPointer<QPushButton>   m_pBack;
  QPointer<QPushButton>   m_pForward;
  QPointer<QPushButton>   m_pCloseButton;
  bool                    m_bForward;
};

#endif // CSCRIPTSEARCHBAR_H
