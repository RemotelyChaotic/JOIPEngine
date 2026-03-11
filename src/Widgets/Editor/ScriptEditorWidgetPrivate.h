#ifndef SRIPTEDITORWIDGETPRIVATE_H
#define SRIPTEDITORWIDGETPRIVATE_H

#include <QKeyEvent>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QPointer>
#include <QResizeEvent>

class CScriptEditorWidget;

//----------------------------------------------------------------------------------------
//
class CScriptEditorWidgetPrivate : public QPlainTextEdit
{
  friend class CScriptEditorWidget;
  friend class CFoldBlockArea;
  friend class CFooterArea;
  friend class CLineNumberArea;
  friend class CWidgetArea;

public:
  CScriptEditorWidgetPrivate(CScriptEditorWidget* pParent);
  ~CScriptEditorWidgetPrivate() override;

protected:
  void keyPressEvent(QKeyEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  QPointer<CScriptEditorWidget> m_pParent;
};

#endif // SRIPTEDITORWIDGETPRIVATE_H
