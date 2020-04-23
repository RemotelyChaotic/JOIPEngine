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

public:
  explicit CShortcutButton(QWidget* pParent = nullptr);
  explicit CShortcutButton(const QString& text, QWidget* pParent = nullptr);
  CShortcutButton(const QIcon& icon, const QString& text, QWidget* pParent = nullptr);
  ~CShortcutButton() override;

  void SetShortcut(const QKeySequence& sequence);

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  QAction* m_pAction;
};

#endif // CSHORTCUTBUTTON_H
