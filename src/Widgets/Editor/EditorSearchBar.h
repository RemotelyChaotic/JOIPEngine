#ifndef CSCRIPTSEARCHBAR_H
#define CSCRIPTSEARCHBAR_H

#include "Widgets/OverlayBase.h"

#include <QFlags>
#include <QPointer>

class CSearchWidget;
class QPushButton;

class CEditorSearchBar : public COverlayBase
{
  Q_OBJECT
public:
  enum ESearchDirection
  {
    eForward,
    eBackward,
    eNone
  };
  Q_ENUM(ESearchDirection)

  enum ESearchDisplayFlag : qint32
  {
    eNoDisplayFlags = 0,
    eBackButton = 1,
    eForwardButton = 2,
    eCloseButton = 4,
    eAllDisplayFlags = eBackButton | eForwardButton | eCloseButton
  };
  Q_DECLARE_FLAGS(ESearchDisplayElementFlags, ESearchDisplayFlag)

  explicit CEditorSearchBar(QWidget* pParent = nullptr,
                            ESearchDisplayElementFlags displayFlags = ESearchDisplayFlag::eAllDisplayFlags);
  ~CEditorSearchBar() override;

  bool IsSearchingForward() const { return ESearchDirection::eForward == m_searchDir; }
  ESearchDirection SearchDirection() const { return m_searchDir; }

  void SetFilter(const QString& sString);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;
  void SlotShowHideSearchFilter();

signals:
  void SignalFilterChanged(CEditorSearchBar::ESearchDirection direction, const QString& sText);
  void SignalShown();
  void SignalHidden();

private:
  void SlotBackClicked();
  void SlotForwardClicked();
  void SlotFilterChanged(const QString& sText, bool bReturnPressed);

  QPointer<CSearchWidget> m_pSearchWidget;
  QPointer<QPushButton>   m_pBack;
  QPointer<QPushButton>   m_pForward;
  QPointer<QPushButton>   m_pCloseButton;
  CEditorSearchBar::ESearchDirection m_searchDir;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CEditorSearchBar::ESearchDisplayElementFlags)

#endif // CSCRIPTSEARCHBAR_H
