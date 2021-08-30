#include "UndoRedoFilter.h"
#include "Application.h"
#include "Settings.h"
#include <QContextMenuEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QPointer>

CUndoRedoFilter::CUndoRedoFilter(QObject* pParent, const std::function<QMenu*(void)>& fnCreateContextMenu) :
  QObject(pParent),
  m_fnCreateContextMenu(fnCreateContextMenu)
{
  assert(nullptr != pParent);
  pParent->installEventFilter(this);
}

CUndoRedoFilter::~CUndoRedoFilter() = default;

//----------------------------------------------------------------------------------------
//
bool CUndoRedoFilter::eventFilter(QObject* pTarget, QEvent* pEvent)
{
  if (nullptr == pTarget || nullptr == pEvent || parent() != pTarget)
  {
    return false;
  }

  switch (pEvent->type())
  {
    case QEvent::ContextMenu:
    {
      if (nullptr != m_fnCreateContextMenu)
      {
        QContextMenuEvent* pContextEvent = static_cast<QContextMenuEvent*>(pEvent);
        QPointer<CUndoRedoFilter> pThisGuard(this);
        QMenu* pMenu = m_fnCreateContextMenu();
        QList<QAction*> vpActions = pMenu->actions();
        for (const QAction* pAction : qAsConst(vpActions))
        {
          // see qwidgettextcontrol.cpp -> createStandardContextMenu
          if (pAction->objectName() == QStringLiteral("edit-undo"))
          {
            pAction->disconnect();
            connect(pAction, &QAction::triggered, this, &CUndoRedoFilter::UndoTriggered);
          }
          else if (pAction->objectName() == QStringLiteral("edit-redo"))
          {
            pAction->disconnect();
            connect(pAction, &QAction::triggered, this, &CUndoRedoFilter::RedoTriggered);
          }
        }
        pMenu->exec(pContextEvent->globalPos());
        if (nullptr == pThisGuard) { return true; }
        delete pMenu;
        return true;
      }
    } break;
    case QEvent::ShortcutOverride:
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        // these are hard-coded by Qt
        bool bIsUndo = pKeyEvent->matches(QKeySequence::Undo);
        bool bIsRedo = pKeyEvent->matches(QKeySequence::Redo);

        if (bIsUndo) { emit UndoTriggered(); }
        if (bIsRedo) { emit RedoTriggered(); }

        return bIsUndo || bIsRedo;
    }
    default: break;
  }

  return false;
}
