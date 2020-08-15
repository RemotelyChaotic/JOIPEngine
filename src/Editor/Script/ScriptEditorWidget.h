#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QPlainTextEdit>
#include <QPointer>
#include <QLabel>

#include <map>

class CLineNumberArea;
namespace KSyntaxHighlighting
{
  class Repository;
  class SyntaxHighlighter;
}
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
  Q_PROPERTY(QColor  lineNumberBackgroundColor READ LineNumberBackgroundColor WRITE SetLineNumberBackgroundColor)
  Q_PROPERTY(QColor  lineNumberTextColor       READ LineNumberTextColor       WRITE SetLineNumberTextColor      )
  Q_PROPERTY(QColor  highlightLineColor        READ HighlightLineColor        WRITE SetHighlightLineColor       )
  Q_PROPERTY(QString theme                     READ Theme                     WRITE SetTheme                    )
  Q_PROPERTY(QColor  widgetsBackgroundColor    READ WidgetsBackgroundColor    WRITE SetWidgetsBackgroundColor   )

public:
  CScriptEditorWidget(QWidget* pParent = nullptr);

  void SetHighlightDefinition(const QString& sPath);

  void SetLineNumberBackgroundColor(const QColor& color) { m_lineNumberBackgroundColor = color; }
  const QColor& LineNumberBackgroundColor() { return m_lineNumberBackgroundColor; }
  void SetLineNumberTextColor(const QColor& color) { m_lineNumberTextColor = color; }
  const QColor& LineNumberTextColor() { return m_lineNumberTextColor; }
  void SetHighlightLineColor(const QColor& color) { m_highlightLineColor = color; }
  const QColor& HighlightLineColor() { return m_highlightLineColor; }
  void SetTheme(const QString& sTheme);
  const QString& Theme() { return m_sTheme; }
  void SetWidgetsBackgroundColor(const QColor& color) { m_widgetsBackgroundColor = color; }
  const QColor& WidgetsBackgroundColor() { return m_widgetsBackgroundColor; }

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
  std::unique_ptr<KSyntaxHighlighting::Repository> m_spRepository;
  QPointer<KSyntaxHighlighting::SyntaxHighlighter> m_pHighlighter;
  CLineNumberArea*                                 m_pLineNumberArea;
  CWidgetArea*                                     m_pWidgetArea;
  QString                                          m_sTheme;
  QColor                                           m_lineNumberBackgroundColor;
  QColor                                           m_lineNumberTextColor;
  QColor                                           m_highlightLineColor;
  QColor                                           m_widgetsBackgroundColor;
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
