#ifndef CPUSHNOTIFICATION_H
#define CPUSHNOTIFICATION_H

#include "OverlayButton.h"
#include <QPointer>
#include <QTimer>
#include <optional>

class QLabel;
class QPropertyAnimation;

class CPushNotification : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 iYOffset READ YOffset WRITE SetYOffset)

public:
  explicit CPushNotification(const QString& sMsg,
                             std::optional<std::chrono::milliseconds> displayTime,
                             bool bSingleShot = true,
                             QWidget* pParent = nullptr);
  ~CPushNotification() override;

  void Climb() override;
  void Hide() override;
  void Hide(std::chrono::milliseconds in);
  void Resize() override;
  void Show() override;

  void Move(qint32 iX, qint32 iY);

  void SetMessage(const QString& sMsg);
  void SetTargetPosition(qint32 iYOffsetTarget);

  void SetYOffset(qint32 iValue);
  qint32 YOffset() const;

private:
  QPointer<QPropertyAnimation> m_pPopInOutAnim;
  QPointer<QLabel>             m_pMsg;
  QTimer                       m_timer;
  std::optional<qint32>        m_iTargetPosition;
  qint32                       m_iYOffset;
};

#endif // CPUSHNOTIFICATION_H
