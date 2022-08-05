#ifndef CSCRIPTSEARCHBAR_H
#define CSCRIPTSEARCHBAR_H

#include "Widgets/OverlayBase.h"
#include <QPointer>

class CSearchWidget;
class QPushButton;

class CEditorSearchBar : public COverlayBase
{
  Q_OBJECT
public:
  enum ESearhDirection
  {
    eForward,
    eBackward,
    eNone
  };
  Q_ENUM(ESearhDirection)

  explicit CEditorSearchBar(QWidget* pParent = nullptr);
  ~CEditorSearchBar() override;

  bool IsSearchingForward() const { return m_searchDir == ESearhDirection::eForward; }
  ESearhDirection SearchDirection() const { return m_searchDir; }

  void SetFilter(const QString& sString);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;

signals:
  void SignalFilterChanged(CEditorSearchBar::ESearhDirection direction, const QString& sText);
  void SignalHidden();

private:
  void SlotBackClicked();
  void SlotForwardClicked();
  void SlotFilterChanged(const QString& sText);

  QPointer<CSearchWidget> m_pSearchWidget;
  QPointer<QPushButton>   m_pBack;
  QPointer<QPushButton>   m_pForward;
  QPointer<QPushButton>   m_pCloseButton;
  CEditorSearchBar::ESearhDirection m_searchDir;
};

#endif // CSCRIPTSEARCHBAR_H
