#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include <QIcon>
#include <QPlainTextEdit>
#include <QPointer>
#include <QLabel>
#include <QTextBlock>
#include <map>

class CEditorHighlighter;
class CFoldBlockArea;
class CLineNumberArea;
class CHighlightedSearchableTextEdit;
class CEditorSearchBar;
class CWidgetArea;
namespace KSyntaxHighlighting
{
  class Repository;
}
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

//----------------------------------------------------------------------------------------
//
class CScriptEditorWidget : public QPlainTextEdit
{
  Q_OBJECT
  Q_PROPERTY(QColor  foldAreaBackgroundColor   READ FoldAreaBackgroundColor   WRITE SetFoldAreaBackgroundColor)
  Q_PROPERTY(QIcon   iconFolded                READ IconFolded                WRITE SetIconFolded)
  Q_PROPERTY(QIcon   iconUnfolded              READ IconUnfolded              WRITE SetIconUnfolded)
  Q_PROPERTY(QColor  lineNumberBackgroundColor READ LineNumberBackgroundColor WRITE SetLineNumberBackgroundColor)
  Q_PROPERTY(QColor  lineNumberTextColor       READ LineNumberTextColor       WRITE SetLineNumberTextColor      )
  Q_PROPERTY(QColor  highlightLineColor        READ HighlightLineColor        WRITE SetHighlightLineColor       )
  Q_PROPERTY(QColor  highlightSearchBackgroundColor READ HighlightSearchBackgroundColor WRITE SetHighlightSearchBackgroundColor )
  Q_PROPERTY(QColor  highlightSearchColor      READ HighlightSearchColor      WRITE SetHighlightSearchColor     )
  Q_PROPERTY(QString theme                     READ Theme                     WRITE SetTheme                    )
  Q_PROPERTY(QColor  widgetsBackgroundColor    READ WidgetsBackgroundColor    WRITE SetWidgetsBackgroundColor   )

public:
  CScriptEditorWidget(QWidget* pParent = nullptr);

  void SetHighlightDefinition(const QString& sPath);

  void SetFoldAreaBackgroundColor(const QColor& color) { m_foldAreaBackgroundColor = color; }
  const QColor& FoldAreaBackgroundColor() { return m_foldAreaBackgroundColor; }
  void SetIconFolded(const QIcon &icon) { m_foldedIcon = icon; }
  const QIcon& IconFolded() const { return m_foldedIcon; }
  void SetIconUnfolded(const QIcon &icon) { m_unfoldedIcon = icon; }
  const QIcon& IconUnfolded() const { return m_unfoldedIcon; }
  void SetLineNumberBackgroundColor(const QColor& color) { m_lineNumberBackgroundColor = color; }
  const QColor& LineNumberBackgroundColor() { return m_lineNumberBackgroundColor; }
  void SetLineNumberTextColor(const QColor& color) { m_lineNumberTextColor = color; }
  const QColor& LineNumberTextColor() { return m_lineNumberTextColor; }
  void SetHighlightLineColor(const QColor& color) { m_highlightLineColor = color; }
  const QColor& HighlightLineColor() { return m_highlightLineColor; }
  void SetHighlightSearchBackgroundColor(const QColor& color);
  const QColor& HighlightSearchBackgroundColor() { return m_highlightSearchBackgroundColor; }
  void SetHighlightSearchColor(const QColor& color);
  const QColor& HighlightSearchColor() { return m_highlightSearchColor; }
  void SetTheme(const QString& sTheme);
  const QString& Theme() { return m_sTheme; }
  void SetWidgetsBackgroundColor(const QColor& color) { m_widgetsBackgroundColor = color; }
  const QColor& WidgetsBackgroundColor() { return m_widgetsBackgroundColor; }

  CLineNumberArea* LineNumberArea() const { return m_pLineNumberArea; }
  CWidgetArea*     WidgetArea() const { return m_pWidgetArea; }
  CFoldBlockArea*  FoldBlockArea() const { return m_pFoldBlockArea; }
  QPointer<CEditorHighlighter> Highlighter() const;
  QPointer<CEditorSearchBar>   SearchBar() const;

  void FoldBlockAreaMouseEvent(QMouseEvent* pEvt);
  void FoldBlockAreaPaintEvent(QPaintEvent* pEvent);
  qint32 FoldBlockAreaWidth() const;
  void LineNumberAreaPaintEvent(QPaintEvent* pEvent);
  qint32 LineNumberAreaWidth() const;
  void ResetWidget();
  void WidgetAreaPaintEvent(QPaintEvent* pEvent);
  qint32 WidgetAreaWidth() const;

  QMenu* CreateContextMenu();

public slots:
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);

protected:
  bool eventFilter(QObject* pTarget, QEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void HighlightCurrentLine();
  void UpdateLeftAreaWidth(qint32 iNewBlockCount);
  void UpdateFoldBlockArea(const QRect&, qint32);
  void UpdateLineNumberArea(const QRect&, qint32);
  void UpdateWidgetArea(const QRect&, qint32);

private:
  std::unique_ptr<KSyntaxHighlighting::Repository> m_spRepository;
  QPointer<CHighlightedSearchableTextEdit>         m_pHighlightedSearchableEdit;
  CLineNumberArea*                                 m_pLineNumberArea;
  CWidgetArea*                                     m_pWidgetArea;
  CFoldBlockArea*                                  m_pFoldBlockArea;
  QIcon                                            m_foldedIcon;
  QIcon                                            m_unfoldedIcon;
  QString                                          m_sTheme;
  QColor                                           m_foldAreaBackgroundColor;
  QColor                                           m_lineNumberBackgroundColor;
  QColor                                           m_lineNumberTextColor;
  QColor                                           m_highlightLineColor;
  QColor                                           m_highlightSearchBackgroundColor;
  QColor                                           m_highlightSearchColor;
  QColor                                           m_widgetsBackgroundColor;
  Qt::Key                                          m_previouslyClickedKey;
  QRect                                            m_foldSelection;
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

//----------------------------------------------------------------------------------------
//
class CFoldBlockArea : public QWidget
{
public:
  CFoldBlockArea(CScriptEditorWidget* pEditor) :
    QWidget(pEditor)
  {
    m_pCodeEditor = pEditor;
    setMouseTracking(true);
  }

  QSize sizeHint() const override
  {
    return QSize(m_pCodeEditor->FoldBlockAreaWidth(), m_pCodeEditor->height());
  }

protected:
  void mousePressEvent(QMouseEvent* pEvt) override;
  void paintEvent(QPaintEvent *event) override;

private:
  CScriptEditorWidget*               m_pCodeEditor;
};

#endif // SCRIPTEDITORWIDGET_H
