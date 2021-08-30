#ifndef CUNDOREDOFILTER_H
#define CUNDOREDOFILTER_H

#include <QObject>
#include <QMenu>
#include <functional>

class CUndoRedoFilter : public QObject
{
  Q_OBJECT

public:
  explicit CUndoRedoFilter(QObject* pParent, const std::function<QMenu*(void)>& fnCreateContextMenu);
  ~CUndoRedoFilter() override;

signals:
  void UndoTriggered();
  void RedoTriggered();

protected:
  bool eventFilter(QObject* pTarget, QEvent* pEvent) override;

private:
  std::function<QMenu*(void)> m_fnCreateContextMenu;
};

#endif // CUNDOREDOFILTER_H
