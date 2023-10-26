#ifndef SCRIPEDITORADDONWIDGETS_H
#define SCRIPEDITORADDONWIDGETS_H

#include "IScriptEditorAddons.h"

#include <QLabel>
#include <QPointer>
#include <QWidget>

class CScriptEditorWidget;

//----------------------------------------------------------------------------------------
//
class CLineNumberArea : public QWidget,
                        public IScriptEditorAddon
{
public:
  CLineNumberArea(CScriptEditorWidget* pEditor);
  ~CLineNumberArea() override;

  QSize sizeHint() const override;
  qint32 AreaHeight() const override;
  qint32 AreaWidth() const override;
  void Reset() override;
  void Update(const QRect& rect, qint32 iDy) override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  CScriptEditorWidget* m_pCodeEditor;
};

//----------------------------------------------------------------------------------------
//
class CWidgetArea : public QWidget,
                    public IScriptEditorAddon
{
public:
  CWidgetArea(CScriptEditorWidget* pEditor);
  ~CWidgetArea() override;

  QSize sizeHint() const override;
  qint32 AreaHeight() const override;
  qint32 AreaWidth() const override;
  void Reset() override;
  void Update(const QRect& rect, qint32 iDy) override;

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
class CFoldBlockArea : public QWidget,
                       public IScriptEditorAddon
{
public:
  CFoldBlockArea(CScriptEditorWidget* pEditor);
  ~CFoldBlockArea() override;

  QSize sizeHint() const override;
  qint32 AreaHeight() const override;
  qint32 AreaWidth() const override;
  void Reset() override;
  void Update(const QRect& rect, qint32 iDy) override;

protected:
  void mousePressEvent(QMouseEvent* pEvt) override;
  void paintEvent(QPaintEvent *event) override;

protected slots:
  void CursorPositionChanged();

private:
  CScriptEditorWidget*               m_pCodeEditor;
};

#endif // SCRIPEDITORADDONWIDGETS_H
