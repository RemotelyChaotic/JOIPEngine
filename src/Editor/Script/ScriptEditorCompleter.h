#ifndef CSCRIPTEDITORCOMPLETER_H
#define CSCRIPTEDITORCOMPLETER_H

#include <QCompleter>
#include <QPointer>
#include <QPlainTextEdit>
#include <QTextEdit>

#include <functional>

class CEditorModelWrapper;

class CScriptEditorCompleter : public QObject
{
  Q_OBJECT

public:
  explicit CScriptEditorCompleter(QPointer<QTextEdit> pEditor);
  explicit CScriptEditorCompleter(QPointer<QPlainTextEdit> pEditor);
  ~CScriptEditorCompleter();

  bool IsCompleterOpen() const;

  void SetCompleterColumn(qint32 column);
  void SetEndOfwordFunction(std::function<bool(QString)> fn);
  void SetModel(QAbstractItemModel* pModel);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private slots:
  void InsertCompletion(const QString& sCompletion);

private:
  void Init(QWidget* pWidget);
  QString TextUnderCursor();

  QPointer<QCompleter>          m_pCompleter;
  QPointer<CEditorModelWrapper> m_pModelWrapper;
  QPointer<QTextEdit>           m_pEditor;
  QPointer<QPlainTextEdit>      m_pEditorPlain;
  std::function<bool(const QString&)> m_fnEndOfWord;
};

#endif // CSCRIPTEDITORCOMPLETER_H
