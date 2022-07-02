#ifndef CPUSHNOTIFICATION_H
#define CPUSHNOTIFICATION_H

#include "OverlayButton.h"
#include <QPointer>

class QLabel;
class QPropertyAnimation;

class CPushNotification : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 iYOffset READ YOffset WRITE SetYOffset)

public:
  explicit CPushNotification(const QString& sMsg,
                             std::chrono::milliseconds displayTime,
                             QWidget* pParent = nullptr);
  ~CPushNotification() override;

  void Initialize();

  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;

  void Move(qint32 iX, qint32 iY);

  void SetYOffset(qint32 iValue);
  qint32 YOffset() const;

private:
  QPointer<QPropertyAnimation> m_pPopInOutAnim;
  QPointer<QLabel>             m_pMsg;
  qint32                       m_iYOffset;
};

#endif // CPUSHNOTIFICATION_H
