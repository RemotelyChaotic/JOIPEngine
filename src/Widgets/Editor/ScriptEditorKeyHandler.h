#ifndef CSCRIPTEDITORKEYHANDLER_H
#define CSCRIPTEDITORKEYHANDLER_H

#include <QKeyEvent>

#include <map>
#include <memory>
#include <set>

class CScriptEditorWidget;

class IScriptEditorKeyHandler
{
public:
  IScriptEditorKeyHandler(CScriptEditorWidget* pCodeEditor,
                          Qt::Key* pPreviouslyClickedKey);
  virtual ~IScriptEditorKeyHandler();

  virtual bool KeyEvent(QKeyEvent* pKeyEvent) = 0;
  virtual std::set<Qt::Key> HandledKeys() const = 0;

  static void RegisterHandlers(std::map<Qt::Key, std::shared_ptr<IScriptEditorKeyHandler>>& handlerMap,
                               CScriptEditorWidget* pCodeEditor,
                               Qt::Key* pPreviouslyClickedKey);

protected:
  CScriptEditorWidget*               m_pCodeEditor;
  Qt::Key*                           m_pPreviouslyClickedKey;
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorEnterHandler : public IScriptEditorKeyHandler
{
public:
  CScriptEditorEnterHandler(CScriptEditorWidget* pCodeEditor,
                            Qt::Key* pPreviouslyClickedKey);
  ~CScriptEditorEnterHandler() override;

  bool KeyEvent(QKeyEvent* pKeyEvent) override;
  std::set<Qt::Key> HandledKeys() const override;
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorTabHandler : public IScriptEditorKeyHandler
{
public:
  CScriptEditorTabHandler(CScriptEditorWidget* pCodeEditor,
                            Qt::Key* pPreviouslyClickedKey);
  ~CScriptEditorTabHandler() override;

  bool KeyEvent(QKeyEvent* pKeyEvent) override;
  std::set<Qt::Key> HandledKeys() const override;
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorBracesHandler : public IScriptEditorKeyHandler
{
public:
  CScriptEditorBracesHandler(CScriptEditorWidget* pCodeEditor,
                             Qt::Key* pPreviouslyClickedKey);
  ~CScriptEditorBracesHandler() override;

  bool KeyEvent(QKeyEvent* pKeyEvent) override;
  std::set<Qt::Key> HandledKeys() const override;
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorQuotesHandler : public IScriptEditorKeyHandler
{
public:
  CScriptEditorQuotesHandler(CScriptEditorWidget* pCodeEditor,
                             Qt::Key* pPreviouslyClickedKey);
  ~CScriptEditorQuotesHandler() override;

  bool KeyEvent(QKeyEvent* pKeyEvent) override;
  std::set<Qt::Key> HandledKeys() const override;
};

#endif // CSCRIPTEDITORKEYHANDLER_H
