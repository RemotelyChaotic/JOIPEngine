#include "ScriptEditorCompleter.h"
#include "ScriptEditorWidget.h"

#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextDocumentFragment>

namespace
{
  constexpr qint32 c_iTimerInterval = 2000;
}

//----------------------------------------------------------------------------------------
//
class CEditorModelWrapper : public QAbstractItemModel
{
  Q_OBJECT
public:
  CEditorModelWrapper(QObject* pObj) :
    QAbstractItemModel(pObj)
  {
  }
  ~CEditorModelWrapper() override = default;

  void SetCurrentColumn(qint32 column)
  {
    m_iCurrentColumn = column;
  }

  void SetSourceModel(QPointer<QAbstractItemModel> pSource)
  {
    if (m_pSource == pSource) { return; }

    beginResetModel();

    if (nullptr != m_pSource)
    {
      disconnect(m_pSource, &QAbstractItemModel::dataChanged,
                 this, &CEditorModelWrapper::dataChanged);

      disconnect(m_pSource, &QAbstractItemModel::headerDataChanged,
                 this, &CEditorModelWrapper::headerDataChanged);

      disconnect(m_pSource, &QAbstractItemModel::rowsAboutToBeInserted,
                 this, &CEditorModelWrapper::rowsAboutToBeInserted);

      disconnect(m_pSource, &QAbstractItemModel::rowsInserted,
                 this, &CEditorModelWrapper::rowsInserted);

      disconnect(m_pSource, &QAbstractItemModel::columnsAboutToBeInserted,
                 this, &CEditorModelWrapper::columnsAboutToBeInserted);

      disconnect(m_pSource, &QAbstractItemModel::columnsInserted,
                 this, &CEditorModelWrapper::columnsInserted);

      disconnect(m_pSource, &QAbstractItemModel::rowsAboutToBeRemoved,
                 this, &CEditorModelWrapper::rowsAboutToBeRemoved);

      disconnect(m_pSource, &QAbstractItemModel::rowsRemoved,
                 this, &CEditorModelWrapper::rowsRemoved);

      disconnect(m_pSource, &QAbstractItemModel::columnsAboutToBeRemoved,
                 this, &CEditorModelWrapper::columnsAboutToBeRemoved);

      disconnect(m_pSource, &QAbstractItemModel::columnsRemoved,
                 this, &CEditorModelWrapper::columnsRemoved);

      disconnect(m_pSource, &QAbstractItemModel::rowsAboutToBeMoved,
                 this, &CEditorModelWrapper::rowsAboutToBeMoved);

      disconnect(m_pSource, &QAbstractItemModel::rowsMoved,
                 this, &CEditorModelWrapper::rowsMoved);

      disconnect(m_pSource, &QAbstractItemModel::columnsAboutToBeMoved,
                 this, &CEditorModelWrapper::columnsAboutToBeMoved);

      disconnect(m_pSource, &QAbstractItemModel::columnsMoved,
                 this, &CEditorModelWrapper::columnsMoved);

      disconnect(m_pSource, &QAbstractItemModel::layoutAboutToBeChanged,
                 this, &CEditorModelWrapper::layoutAboutToBeChanged);

      disconnect(m_pSource, &QAbstractItemModel::layoutChanged,
                 this, &CEditorModelWrapper::layoutChanged);

      disconnect(m_pSource, &QAbstractItemModel::modelAboutToBeReset, this, &CEditorModelWrapper::modelAboutToBeReset);
      disconnect(m_pSource, &QAbstractItemModel::modelReset, this, &CEditorModelWrapper::modelReset);
    }

    m_pSource = pSource;
    if (nullptr == m_pSource)
    {
      endResetModel();
      return;
    }

    connect(m_pSource, &QAbstractItemModel::dataChanged,
            this, &CEditorModelWrapper::dataChanged);

    connect(m_pSource, &QAbstractItemModel::headerDataChanged,
            this, &CEditorModelWrapper::headerDataChanged);

    connect(m_pSource, &QAbstractItemModel::rowsAboutToBeInserted,
            this, &CEditorModelWrapper::rowsAboutToBeInserted);

    connect(m_pSource, &QAbstractItemModel::rowsInserted,
            this, &CEditorModelWrapper::rowsInserted);

    connect(m_pSource, &QAbstractItemModel::columnsAboutToBeInserted,
            this, &CEditorModelWrapper::columnsAboutToBeInserted);

    connect(m_pSource, &QAbstractItemModel::columnsInserted,
            this, &CEditorModelWrapper::columnsInserted);

    connect(m_pSource, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &CEditorModelWrapper::rowsAboutToBeRemoved);

    connect(m_pSource, &QAbstractItemModel::rowsRemoved,
            this, &CEditorModelWrapper::rowsRemoved);

    connect(m_pSource, &QAbstractItemModel::columnsAboutToBeRemoved,
            this, &CEditorModelWrapper::columnsAboutToBeRemoved);

    connect(m_pSource, &QAbstractItemModel::columnsRemoved,
            this, &CEditorModelWrapper::columnsRemoved);

    connect(m_pSource, &QAbstractItemModel::rowsAboutToBeMoved,
            this, &CEditorModelWrapper::rowsAboutToBeMoved);

    connect(m_pSource, &QAbstractItemModel::rowsMoved,
            this, &CEditorModelWrapper::rowsMoved);

    connect(m_pSource, &QAbstractItemModel::columnsAboutToBeMoved,
            this, &CEditorModelWrapper::columnsAboutToBeMoved);

    connect(m_pSource, &QAbstractItemModel::columnsMoved,
            this, &CEditorModelWrapper::columnsMoved);

    connect(m_pSource, &QAbstractItemModel::layoutAboutToBeChanged,
            this, &CEditorModelWrapper::layoutAboutToBeChanged);

    connect(m_pSource, &QAbstractItemModel::layoutChanged,
            this, &CEditorModelWrapper::layoutChanged);

    connect(m_pSource, &QAbstractItemModel::modelAboutToBeReset, this, &CEditorModelWrapper::modelAboutToBeReset);
    connect(m_pSource, &QAbstractItemModel::modelReset, this, &CEditorModelWrapper::modelReset);

    endResetModel();
  }

  QModelIndex index(int iRow, int iColumn,
                    const QModelIndex& parent = QModelIndex()) const override
  {
    if (nullptr != m_pSource)
    {
      return m_pSource->index(iRow, iColumn, parent);
    }
    return QModelIndex();
  }
  QModelIndex parent(const QModelIndex& child) const override
  {
    if (nullptr != m_pSource)
    {
      return m_pSource->parent(child);
    }
    return QModelIndex();
  }

  int rowCount(const QModelIndex& parent = QModelIndex()) const override
  {
    if (nullptr != m_pSource)
    {
      // we need to overwrite the column since the default parent is an invalid index
      // but we need to know the column to find the count, since the rowcount depends on
      // the column
      return m_pSource->rowCount(createIndex(parent.row(), m_iCurrentColumn));
    }
    return 0;
  }
  int columnCount(const QModelIndex& parent = QModelIndex()) const override
  {
    if (nullptr != m_pSource)
    {
      return m_pSource->columnCount(parent);
    }
    return 0;
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
  {
    if (nullptr != m_pSource)
    {
      return m_pSource->data(index, role);
    }
    return QVariant();
  }

  Qt::ItemFlags flags(const QModelIndex &index) const override
  {
    if (nullptr != m_pSource)
    {
      return m_pSource->flags(index);
    }
    return QAbstractItemModel::flags(index);
  }

private:
  QPointer<QAbstractItemModel> m_pSource;
  qint32 m_iCurrentColumn = -1;
};

#include "ScriptEditorCompleter.moc"

//----------------------------------------------------------------------------------------
//
CScriptEditorCompleter::CScriptEditorCompleter(QPointer<QTextEdit> pEditor) :
  QObject(pEditor),
  m_pCompleter(new QCompleter(this)),
  m_pEditor(pEditor)
{
  Init(m_pEditor);
}
CScriptEditorCompleter::CScriptEditorCompleter(QPointer<QPlainTextEdit> pEditor) :
  QObject(pEditor),
  m_pCompleter(new QCompleter(this)),
  m_pEditorPlain(pEditor)
{
  Init(m_pEditorPlain);
}
CScriptEditorCompleter::~CScriptEditorCompleter()
{

}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorCompleter::IsCompleterOpen() const
{
  return m_pCompleter->popup()->isVisible();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::SetCompleterColumn(qint32 column)
{
  m_pCompleter->setCompletionColumn(column);
  m_pModelWrapper->SetCurrentColumn(column);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::SetEndOfwordFunction(std::function<bool(QString)> fn)
{
  m_fnEndOfWord = fn;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::SetModel(QAbstractItemModel* pModel)
{
  m_pModelWrapper->SetSourceModel(pModel);
}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorCompleter::keyPressEvent(QKeyEvent* pKeyEvt)
{
  qint32 k = pKeyEvt->key();
  if (IsCompleterOpen())
  {
    switch (k)
    {
      case Qt::Key_Escape:
      case Qt::Key_Backtab:
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Tab:
      case Qt::Key_Enter:
      case Qt::Key_Return:
        pKeyEvt->ignore();
        return true;
      default:
        break;
    }
  }
  else
  {
    switch (k)
    {
      case Qt::Key_Escape:
      case Qt::Key_Backtab:
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Tab:
      case Qt::Key_Enter:
      case Qt::Key_Return:
        SlotTimeoutInteraction();
        break;
    }
  }

  const bool bCtrlOrShift = pKeyEvt->modifiers().testFlag(Qt::ControlModifier) ||
                            pKeyEvt->modifiers().testFlag(Qt::ShiftModifier);
  if (bCtrlOrShift && pKeyEvt->text().isEmpty())
  {
    return false;
  }

  m_changedTimer.start(c_iTimerInterval);

  const bool bHasModifier = (pKeyEvt->modifiers() != Qt::NoModifier) && !bCtrlOrShift;
  QString sCompletionPrefix = TextUnderCursor(pKeyEvt);

  if (bHasModifier || pKeyEvt->text().isEmpty() || sCompletionPrefix.length() < 3 ||
      (nullptr != m_fnEndOfWord && m_fnEndOfWord(pKeyEvt->text().right(1))) )
  {
    m_pCompleter->setCompletionPrefix(QString());
    m_pCompleter->popup()->hide();
    return false;
  }

  if (sCompletionPrefix != m_pCompleter->completionPrefix())
  {
    m_pCompleter->setCompletionPrefix(sCompletionPrefix);
    m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
  }
  QRect cr;
  if (nullptr != m_pEditor)
  {
    cr = m_pEditor->cursorRect();
  }
  else if (nullptr != m_pEditorPlain)
  {
    cr = m_pEditorPlain->cursorRect();
  }
  cr.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
              + m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
  m_pCompleter->complete(cr); // popup it up!

  return false;
}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorCompleter::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if ((nullptr != m_pEditor || nullptr != m_pEditorPlain) && nullptr != pObj &&
      nullptr != pEvt)
  {
    if (QEvent::FocusIn == pEvt->type())
    {
      if (nullptr != m_pEditor)
      {
        m_pCompleter->setWidget(m_pEditor);
      }
      else if (nullptr != m_pEditorPlain)
      {
        m_pCompleter->setWidget(m_pEditorPlain);
      }
    }
    else if (QEvent::FocusOut == pEvt->type())
    {
      SlotTimeoutInteraction(true);
    }
    else if (QEvent::KeyPress == pEvt->type())
    {
      QKeyEvent* pKeyEvt = static_cast<QKeyEvent*>(pEvt);
      qint32 k = pKeyEvt->key();
      if (IsCompleterOpen())
      {
        switch (k)
        {
          case Qt::Key_Escape:
          case Qt::Key_Backtab:
          case Qt::Key_Up:
          case Qt::Key_Down:
          case Qt::Key_Tab:
          case Qt::Key_Enter:
          case Qt::Key_Return:
            pKeyEvt->ignore();
            return true; // let the completer do default behavior
          default:
            break;
        }
      }

      const bool bCtrlOrShift = pKeyEvt->modifiers().testFlag(Qt::ControlModifier) ||
                                pKeyEvt->modifiers().testFlag(Qt::ShiftModifier);
      if (bCtrlOrShift && pKeyEvt->text().isEmpty())
      {
        return false;
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::SlotInsertCompletion(const QString& sCompletion)
{
  QTextCursor tc;
  if (nullptr != m_pEditor)
  {
    tc = m_pEditor->textCursor();
  }
  else if (nullptr != m_pEditorPlain)
  {
    tc = m_pEditorPlain->textCursor();
  }

  int extra = sCompletion.length() - m_pCompleter->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(sCompletion.right(extra));

  if (nullptr != m_pEditor)
  {
    m_pEditor->setTextCursor(tc);
  }
  else if (nullptr != m_pEditorPlain)
  {
    m_pEditorPlain->setTextCursor(tc);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::SlotTimeoutInteraction(bool bForce)
{
  if (IsCompleterOpen() && !bForce)
  {
    m_changedTimer.start(c_iTimerInterval);
    return;
  }

  m_changedTimer.stop();

  QTextCursor tc;
  if (nullptr != m_pEditor)
  {
    tc = m_pEditor->textCursor();
  }
  else if (nullptr != m_pEditorPlain)
  {
    tc = m_pEditorPlain->textCursor();
  }

  tc.select(QTextCursor::LineUnderCursor);

  const QString s = tc.selectedText();
  emit SignalLineChanged(s);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleter::Init(QWidget* pWidget)
{
  pWidget->installEventFilter(this);
  m_pCompleter->setWidget(pWidget);
  m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  m_pModelWrapper = new CEditorModelWrapper(m_pCompleter);
  m_pCompleter->setModel(m_pModelWrapper);
  m_pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_pCompleter->setWrapAround(false);
  QObject::connect(m_pCompleter, QOverload<const QString &>::of(&QCompleter::activated),
                   this, &CScriptEditorCompleter::SlotInsertCompletion);

  m_changedTimer.setInterval(c_iTimerInterval);
  m_changedTimer.setSingleShot(true);
  connect(&m_changedTimer, &QTimer::timeout, this, [this]() { SlotTimeoutInteraction(); });
}

//----------------------------------------------------------------------------------------
//
QString CScriptEditorCompleter::TextUnderCursor(QKeyEvent* pKeyEvt)
{
  QTextCursor tc;
  if (nullptr != m_pEditor)
  {
    tc = m_pEditor->textCursor();
  }
  else if (nullptr != m_pEditorPlain)
  {
    tc = m_pEditorPlain->textCursor();
  }

  qint32 iPosRel = tc.anchor();
  tc.select(QTextCursor::WordUnderCursor);

  qint32 k = pKeyEvt->key();
  if (Qt::Key_Backspace == k)
  {
    QString s = tc.selectedText();
    return s.left(s.length()-1);
  }
  else if (Qt::Key_Delete == k)
  {
    QString s = tc.selectedText();
    qint32 iBegin = tc.anchor();
    qint32 iLeft = iPosRel-iBegin;
    return s.left(iLeft) + s.right(s.length()-1-iLeft);
  }
  else
  {
    return tc.selectedText() + pKeyEvt->text();
  }
}
