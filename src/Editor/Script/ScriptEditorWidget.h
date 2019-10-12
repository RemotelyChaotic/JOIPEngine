#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QPlainTextEdit>
#include <QPointer>
#include <QLabel>

#include <map>

class CLineNumberArea;
class CWidgetArea;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

//----------------------------------------------------------------------------------------
//
class CScriptEditorWidget : public QPlainTextEdit
{
  Q_OBJECT

public:
  CScriptEditorWidget(QWidget* pParent = nullptr);

  void LineNumberAreaPaintEvent(QPaintEvent* pEvent);
  qint32 LineNumberAreaWidth();
  void ResetWidget();
  void WidgetAreaPaintEvent(QPaintEvent* pEvent);
  qint32 WidgetAreaWidth();

public slots:
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);

protected:
  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void HighlightCurrentLine();
  void UpdateLeftAreaWidth(qint32 iNewBlockCount);
  void UpdateLineNumberArea(const QRect&, qint32);
  void UpdateWidgetArea(const QRect&, qint32);

private:
  CLineNumberArea* m_pLineNumberArea;
  CWidgetArea*     m_pWidgetArea;
};

//----------------------------------------------------------------------------------------
//
class CLineNumberArea : public QWidget
{
public:
  CLineNumberArea(CScriptEditorWidget* pEditor) :
    QWidget(pEditor)
  {
    m_pCodeEditor = pEditor;
  }

  QSize sizeHint() const override
  {
    return QSize(m_pCodeEditor->LineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override
  {
    m_pCodeEditor->LineNumberAreaPaintEvent(event);
  }

private:
  CScriptEditorWidget* m_pCodeEditor;
};

//----------------------------------------------------------------------------------------
//
class CWidgetArea : public QWidget
{
public:
  CWidgetArea(CScriptEditorWidget* pEditor) :
    QWidget(pEditor)
  {
    m_pCodeEditor = pEditor;
  }

  QSize sizeHint() const override
  {
    return QSize(m_pCodeEditor->WidgetAreaWidth(), 0);
  }

  void AddError(QString sException, qint32 iLine, QString sStack);
  void ClearAllErrors();
  void HideAllErrors();
  QPointer<QLabel> Widget(qint32 iLine);

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;
  void paintEvent(QPaintEvent *event) override;

private:
  CScriptEditorWidget*               m_pCodeEditor;
  std::map<qint32, QPointer<QLabel>> m_errorLabelMap;
};


#endif // SCRIPTEDITORWIDGET_H
