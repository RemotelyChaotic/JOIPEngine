#ifndef COVERLAYBUTTON_H
#define COVERLAYBUTTON_H

#include "OverlayBase.h"
#include <QPointer>
#include <QString>

class CShortcutButton;

class COverlayButton : public COverlayBase
{
  Q_OBJECT

public:
  static qint32 c_iOverlayButtonZOrder;

  explicit COverlayButton(const QString& sOverlayName,
                          const QString& sButtonName,
                          const QString& sToolTip,
                          const QString& sKeyBinding,
                          QWidget* pParent = nullptr);
  ~COverlayButton() override;

  void Initialize();

  void Climb() override;
  void Resize() override;

signals:
  void SignalButtonClicked();

protected:
  void SlotKeyBindingsChanged();

  QPointer<CShortcutButton> m_pButton;

private:
  void retranslateUi(QWidget* pHelpOverlay);

  QString m_sOverlayName;
  QString m_sButtonName;
  QString m_sToolTip;
  QString m_sKeyBinding;
};

#endif // COVERLAYBUTTON_H
