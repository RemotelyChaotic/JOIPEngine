#ifndef SCENENODEMODELWIDGET_H
#define SCENENODEMODELWIDGET_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneNodeModelWidget;
}

class CSceneNodeModelWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneNodeModelWidget(QWidget* pParent = nullptr);
  ~CSceneNodeModelWidget();

signals:

protected:

private:
  std::unique_ptr<Ui::CSceneNodeModelWidget> m_spUi;
};

#endif // SCENENODEMODELWIDGET_H
