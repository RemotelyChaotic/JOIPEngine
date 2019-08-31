#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;

//----------------------------------------------------------------------------------------
//
class CScriptEditorWidget : public QPlainTextEdit
{
  Q_OBJECT

public:
  CScriptEditorWidget(QWidget* pParent = nullptr);

  void LineNumberAreaPaintEvent(QPaintEvent* pEvent);
  qint32 LineNumberAreaWidth();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void HighlightCurrentLine();
  void UpdateLineNumberAreaWidth(qint32 iNewBlockCount);
  void UpdateLineNumberArea(const QRect&, qint32);

private:
  QWidget* m_pLineNumberArea;
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


#endif // SCRIPTEDITORWIDGET_H
