#ifndef CSCRIPTEDITORKEYHANDLER_H
#define CSCRIPTEDITORKEYHANDLER_H

#include <QKeyEvent>
#include <QPlainTextEdit>

#include <map>
#include <memory>
#include <set>

class CScriptEditorWidget;

class IScriptEditorKeyHandler
{
public:
  IScriptEditorKeyHandler(QPlainTextEdit* pCodeEditor,
                          Qt::Key* pPreviouslyClickedKey);
  virtual ~IScriptEditorKeyHandler();

  virtual bool KeyEvent(QKeyEvent* pKeyEvent) = 0;
  virtual std::set<Qt::Key> HandledKeys() const = 0;

  static void RegisterHandlers(std::map<Qt::Key, std::shared_ptr<IScriptEditorKeyHandler>>& handlerMap,
                               QPlainTextEdit* pCodeEditor,
                               Qt::Key* pPreviouslyClickedKey);

protected:
  QPlainTextEdit*                    m_pCodeEditor;
  Qt::Key*                           m_pPreviouslyClickedKey;
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorEnterHandler : public IScriptEditorKeyHandler
{
public:
  CScriptEditorEnterHandler(QPlainTextEdit* pCodeEditor,
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
  CScriptEditorTabHandler(QPlainTextEdit* pCodeEditor,
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
  CScriptEditorBracesHandler(QPlainTextEdit* pCodeEditor,
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
  CScriptEditorQuotesHandler(QPlainTextEdit* pCodeEditor,
                             Qt::Key* pPreviouslyClickedKey);
  ~CScriptEditorQuotesHandler() override;

  bool KeyEvent(QKeyEvent* pKeyEvent) override;
  std::set<Qt::Key> HandledKeys() const override;
};

#endif // CSCRIPTEDITORKEYHANDLER_H
