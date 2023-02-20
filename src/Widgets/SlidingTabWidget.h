#ifndef CSLIDINGTABWIDGET_H
#define CSLIDINGTABWIDGET_H

#include "SlidingWidget.h"

#include <QEasingCurve>
#include <QPointer>
#include <QTabWidget>
#include <chrono>

class QStackedWidget;

class CSlidingTabWidget : public QTabWidget, public CSlidingWidget
{
  Q_OBJECT
  Q_INTERFACES(CSlidingWidget)

  Q_PROPERTY(QEasingCurve::Type animation READ Animation WRITE SetAnimation NOTIFY animationChanged)
  Q_PROPERTY(ESlideDirection slideDirection READ SlideDirection WRITE SetSlideDirection NOTIFY slideDirectionChanged)
  Q_PROPERTY(qint32 speed READ Speed WRITE SetSpeed NOTIFY speedChanged)
  Q_PROPERTY(bool wrap READ Wrap WRITE SetWrap NOTIFY wrapChanged)

public:
  explicit CSlidingTabWidget(QWidget* pParent = nullptr);
  ~CSlidingTabWidget() override;

  // hides QTabWidget members
  int addTab(QWidget* pWidget, const QString &);
  int addTab(QWidget* pWidget, const QIcon& icon, const QString& sLabel);
  int insertTab(int index, QWidget* pWidget, const QString&);
  int insertTab(int index, QWidget* pWidget, const QIcon& icon, const QString& sLabel);

public slots:
  void SlideInNext();
  void SlideInPrev();
  void SlideInIdx(qint32 idx);

signals:
  void animationChanged() override;
  void animationFinished() override;
  void slideDirectionChanged() override;
  void speedChanged() override;
  void wrapChanged() override;

protected:
  qint32 Count() const override;
  qint32 CurrentIndex() const override;
  qint32 IndexOf(QWidget* pWidget) const override;
  QWidget* Widget(qint32 idx) const override;

  void SetCurrentIndex(qint32 idx) override;

  void InsertTabImpl(qint32 iIndex);

  QPointer<QStackedWidget> m_pStack;
};

#endif // CSLIDINGTABWIDGET_H
