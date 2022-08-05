#ifndef CHIGHLIGHTEDSEARCHABLETEXTEDIT_H
#define CHIGHLIGHTEDSEARCHABLETEXTEDIT_H

#include "EditorSearchBar.h"

#include <QObject>
#include <QPointer>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <variant>

class CEditorHighlighter;

class CHighlightedSearchableTextEdit : public QObject
{
  Q_OBJECT

public:
  CHighlightedSearchableTextEdit(QPointer<QTextEdit> pEditor);
  CHighlightedSearchableTextEdit(QPointer<QPlainTextEdit> pEditor);
  ~CHighlightedSearchableTextEdit() override;

  QPointer<CEditorHighlighter> Highlighter() const;
  QPointer<CEditorSearchBar>   SearchBar() const;

  QMenu* CreateContextMenu();
  void SetSyntaxHighlightingEnabled(bool bEnabled);

protected slots:
  void SlotShowHideSearchFilter();
  void SlotSearchAreaHidden();
  void SlotSearchFilterChanged(CEditorSearchBar::ESearhDirection direction, const QString& sText);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  QMenu* CreateStandardContextMenu();
  QTextDocument* Document() const;
  void Initalize(QTextDocument* pDoc, QWidget* pParent);
  void SetTextCursor(QTextCursor cursor);
  QTextCursor TextCursor() const;

  std::variant<QPointer<QTextEdit>, QPointer<QPlainTextEdit>> m_pEditor;
  QPointer<CEditorHighlighter>      m_pHighlighter;
  QPointer<CEditorSearchBar>        m_pSearchBar;
  QTextCursor                       m_highlightCursor;
  QString                           m_sLastSearch;
};

#endif // CHIGHLIGHTEDSEARCHABLETEXTEDIT_H
