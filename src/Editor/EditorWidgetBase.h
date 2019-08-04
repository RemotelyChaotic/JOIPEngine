#ifndef EDITORWIDGETBASE_H
#define EDITORWIDGETBASE_H

#include <QWidget>

// define für Screen guards
#define WIDGET_INITIALIZED_GUARD \
  if (!IsInitialized()) { return; }

class CEditorActionBar;

class CEditorWidgetBase : public QWidget
{
  Q_OBJECT
public:
  explicit CEditorWidgetBase(QWidget* pParent = nullptr, CEditorActionBar* pActionBar = nullptr);
  ~CEditorWidgetBase() override;

  virtual void Initialize() = 0;

  bool IsInitialized() const { return m_bInitialized; }

protected:
  void SetInitialized(bool bInit) { m_bInitialized = bInit; }

  CEditorActionBar*                                m_pActionBar;
  bool                                             m_bInitialized;
};

#endif // EDITORWIDGETBASE_H
