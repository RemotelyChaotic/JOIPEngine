#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSearchWidget;
}

class CSearchWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSearchWidget(QWidget* pParent = nullptr);
  ~CSearchWidget();

signals:
  void SignalFilterChanged(const QString& sText);

protected slots:
  void on_pFilterLineEdit_textChanged(const QString& sText);

private:
  std::unique_ptr<Ui::CSearchWidget> m_spUi;
};

#endif // SEARCHWIDGET_H
