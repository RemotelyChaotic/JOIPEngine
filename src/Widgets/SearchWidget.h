#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QPointer>
#include <QWidget>
#include <memory>

namespace Ui {
  class CSearchWidget;
}
class CUndoRedoFilter;

class CSearchWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSearchWidget(QWidget* pParent = nullptr);
  ~CSearchWidget();

  void SetFilterUndo(bool bValue);
  bool IsFilterUndoSet() const;

  void SetFilter(const QString& sFilter);
  QString Filter();

  void SetFocus();

signals:
  void SignalFilterChanged(const QString& sText);

protected slots:
  void on_pFilterLineEdit_editingFinished();
  void on_pFilterLineEdit_textChanged(const QString& sText);

private:
  std::unique_ptr<Ui::CSearchWidget> m_spUi;
  QPointer<CUndoRedoFilter>          m_pUndoFilter;
};

#endif // SEARCHWIDGET_H
