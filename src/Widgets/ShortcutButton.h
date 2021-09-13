#ifndef CSHORTCUTBUTTON_H
#define CSHORTCUTBUTTON_H

#include <QPushButton>

class QAction;
class QIcon;
class QKeySequence;
class QPaintEvent;

class CShortcutButton : public QPushButton
{
  Q_OBJECT
  Q_PROPERTY(bool shortcutAsText READ IsShortcutAsTextEnabled WRITE EnableShortcutAsText NOTIFY shortcutAsTextChanged)

public:
  explicit CShortcutButton(QWidget* pParent = nullptr);
  explicit CShortcutButton(const QString& text, QWidget* pParent = nullptr);
  CShortcutButton(const QIcon& icon, const QString& text, QWidget* pParent = nullptr);
  ~CShortcutButton() override;

  void SetShortcut(const QKeySequence& sequence);

  bool IsShortcutAsTextEnabled() const;
  void EnableShortcutAsText(bool bEnabled);

  // shadow base function
  void setToolTip(const QString& sText);

signals:
  void shortcutAsTextChanged();

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  QAction* m_pAction;
  QString  m_sBaseToolTip;
  bool     m_bShortCutAsTextEnabled;
};

#endif // CSHORTCUTBUTTON_H
