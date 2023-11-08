#ifndef SCRIPTEDITORWIDGET_H
#define SCRIPTEDITORWIDGET_H

#include "IScriptEditorAddons.h"

#include <repository.h>
#include <syntaxhighlighter.h>

#include <QIcon>
#include <QPlainTextEdit>
#include <QPointer>
#include <QLabel>
#include <QTextBlock>
#include <functional>
#include <map>
#include <vector>

class CEditorHighlighter;
class CFoldBlockArea;
class CLineNumberArea;
class CHighlightedSearchableTextEdit;
class CEditorSearchBar;
class CTextEditZoomEnabler;
class CWidgetArea;
class IScriptEditorKeyHandler;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

//----------------------------------------------------------------------------------------
//
class CScriptEditorWidget : public QPlainTextEdit
{
  Q_OBJECT
  Q_PROPERTY(QColor  bracketColor0             READ BracketColor0             WRITE SetBracketColor0)
  Q_PROPERTY(QColor  bracketColor1             READ BracketColor1             WRITE SetBracketColor1)
  Q_PROPERTY(QColor  bracketColor2             READ BracketColor2             WRITE SetBracketColor2)
  Q_PROPERTY(QColor  bracketColor3             READ BracketColor3             WRITE SetBracketColor3)
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
  Q_PROPERTY(QColor  wordHighlightColor        READ WordHighlightColor        WRITE SetWordHighlightColor       )

  friend class CFoldBlockArea;
  friend class CLineNumberArea;
  friend class CWidgetArea;

public:
  enum EScriptEditorAddonPosition
  {
    eLeft = Qt::AlignLeft,
    eRight = Qt::AlignRight,
    eTop = Qt::AlignTop,
    eBottom = Qt::AlignBottom
  };

  CScriptEditorWidget(QWidget* pParent = nullptr);
  ~CScriptEditorWidget() override;

  void SetHighlightDefinition(const QString& sPath);

  void SetBracketColor0(QColor c);
  const QColor& BracketColor0() const  { return m_bracketColor0; }
  void SetBracketColor1(QColor c);
  const QColor& BracketColor1() const  { return m_bracketColor1; }
  void SetBracketColor2(QColor c);
  const QColor& BracketColor2() const  { return m_bracketColor2; }
  void SetBracketColor3(QColor c);
  const QColor& BracketColor3() const  { return m_bracketColor3; }
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
  void SetWordHighlightColor(const QColor& color);
  const QColor& WordHighlightColor() { return m_wordhighlightColor; }
  void SetTheme(const QString& sTheme);
  const QString& Theme() { return m_sTheme; }
  void SetWidgetsBackgroundColor(const QColor& color) { m_widgetsBackgroundColor = color; }
  const QColor& WidgetsBackgroundColor() { return m_widgetsBackgroundColor; }

  QPointer<CEditorHighlighter> Highlighter() const;
  QPointer<CEditorSearchBar>   SearchBar() const;
  QPointer<CTextEditZoomEnabler> ZoomEnabler() const;

  QMenu* CreateContextMenu();
  void UpdateArea(EScriptEditorAddonPosition pos, qint32 iNewBlockCount);
  void ResetAddons();

public slots:
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);
  void SlotUpdateAllAddons(const QRect& rect, qint32 iDy);

protected:
  bool eventFilter(QObject* pTarget, QEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

  QRect                                            m_foldSelection;

private slots:
  void HighlightCurrentLine();
  void HighlightCurrentWord();
  void UpdateLeftAreaWidth(qint32 iNewBlockCount);
  void UpdateRightAreaWidth(qint32 iNewBlockCount);
  void UpdateTopAreaHeight(qint32 iNewBlockCount);
  void UpdateBottomAreaHeight(qint32 iNewBlockCount);

private:
  QPointer<QLabel> WidgetAreaWidget(qint32 iLine);

  std::unique_ptr<KSyntaxHighlighting::Repository> m_spRepository;
  QPointer<CHighlightedSearchableTextEdit>         m_pHighlightedSearchableEdit;
  QPointer<CTextEditZoomEnabler>                   m_pZoomEnabler;
  std::map<EScriptEditorAddonPosition, std::vector<IScriptEditorAddon*>>
                                                   m_vpEditorAddonsMap;
  std::map<Qt::Key, std::shared_ptr<IScriptEditorKeyHandler>>
                                                   m_vpEditorKeyHandlerMap;
  std::function<QPointer<QLabel>(qint32)>          m_fnWidget;
  QIcon                                            m_foldedIcon;
  QIcon                                            m_unfoldedIcon;
  QString                                          m_sTheme;
  QColor                                           m_bracketColor0;
  QColor                                           m_bracketColor1;
  QColor                                           m_bracketColor2;
  QColor                                           m_bracketColor3;
  QColor                                           m_foldAreaBackgroundColor;
  QColor                                           m_lineNumberBackgroundColor;
  QColor                                           m_lineNumberTextColor;
  QColor                                           m_highlightLineColor;
  QColor                                           m_highlightSearchBackgroundColor;
  QColor                                           m_highlightSearchColor;
  QColor                                           m_wordhighlightColor;
  QColor                                           m_widgetsBackgroundColor;
  Qt::Key                                          m_previouslyClickedKey;
};

#endif // SCRIPTEDITORWIDGET_H
