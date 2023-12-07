#ifndef PATTERNPREVIEWWIDGET_H
#define PATTERNPREVIEWWIDGET_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSequencePreviewWidget;
}

class CSequencePreviewWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSequencePreviewWidget(QWidget* pParent = nullptr);
  ~CSequencePreviewWidget();

private:
  std::unique_ptr<Ui::CSequencePreviewWidget> m_spUi;
};

#endif // PATTERNPREVIEWWIDGET_H
