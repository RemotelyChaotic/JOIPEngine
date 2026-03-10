#ifndef SCRIPEDITORADDONWIDGETS_H
#define SCRIPEDITORADDONWIDGETS_H

#include "IScriptEditorAddons.h"

#include <QLabel>
#include <QListWidget>
#include <QPointer>
#include <QWidget>

#include <functional>

class CScriptEditorWidget;
namespace Ui {
  class CScriptFooterArea;
}

//----------------------------------------------------------------------------------------
//
class CLineNumberArea : public QWidget,
                        public IScriptEditorAddon
{
  Q_OBJECT
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
  CScriptEditorWidget* m_pCodeEditor = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CWidgetArea : public QWidget,
                    public IScriptEditorAddon
{
  Q_OBJECT
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
  void ForEach(std::function<void(qint32, QPointer<QLabel>)> fn);
  void HideAllErrors();
  QPointer<QLabel> Widget(qint32 iLine);

signals:
  void SignalAddedError();
  void SignalClearErrors();

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;
  void paintEvent(QPaintEvent *event) override;

private:
  CScriptEditorWidget*               m_pCodeEditor = nullptr;
  std::map<qint32, QPointer<QLabel>> m_errorLabelMap;
};

//----------------------------------------------------------------------------------------
//
class CFoldBlockArea : public QWidget,
                       public IScriptEditorAddon
{
  Q_OBJECT
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
  CScriptEditorWidget*               m_pCodeEditor = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CFooterArea : public QWidget,
                    public IScriptEditorAddon
{
  Q_OBJECT
public:
  CFooterArea(CScriptEditorWidget* pEditor, CWidgetArea* pWidgetArea);
  ~CFooterArea() override;

  QSize sizeHint() const override;
  qint32 AreaHeight() const override;
  qint32 AreaWidth() const override;
  void Reset() override;
  void Update(const QRect& rect, qint32 iDy) override;

protected:
  void paintEvent(QPaintEvent* pEvent) override;

private slots:
  void CursorPositionChanged();
  void ClearAllErors();
  void ErrorAdded();
  void ToggleErrorList();
  void StyleChanged();
  void UpdateWhitespaceText();
  void WhiteSpaceButtonPressed();
  void ZoomChanged(qint32 iZoom);
  void ZoomValueChanged(qint32 iZoom);

private:
  std::unique_ptr<Ui::CScriptFooterArea> m_spUi;
  QPointer<QLabel>                       m_pWsButtonLabel;
  CScriptEditorWidget*                   m_pCodeEditor = nullptr;
  CWidgetArea*                           m_pWidgetArea = nullptr;
  QPointer<QListWidget>                  m_pListView;
};

#endif // SCRIPEDITORADDONWIDGETS_H
