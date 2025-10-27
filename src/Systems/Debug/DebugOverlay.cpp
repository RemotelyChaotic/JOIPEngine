#include "DebugOverlay.h"
#include "Application.h"
#include "DebugInterface.h"
#include "JoipMessageHandler.h"
#include "Settings.h"
#include "ui_DebugOverlay.h"

#include "Widgets/Editor/HighlightedSearchableTextEdit.h"
#include "Widgets/Editor/TextEditZoomEnabler.h"
#include "Widgets/PositionalMenu.h"

#include <QAction>
#include <QDateTime>

#include <cassert>

CDebugOverlay* CDebugOverlay::m_pInstance = nullptr;

CDebugOverlay::CDebugOverlay(QWidget* pParent) :
    COverlayBase(INT_MAX-1, pParent),
    ICustomMessageHandler(),
    m_spUi(new Ui::CDebugOverlay),
    m_spSettings(CApplication::Instance()->Settings()),
    m_wpDebugInterface(CApplication::Instance()->DebugInterface())
{
  m_spUi->setupUi(this);
  //setAttribute(Qt::WA_TranslucentBackground);
  m_spUi->LogTextEdit->setAttribute(Qt::WA_TranslucentBackground);
  m_spUi->CloseButton->setProperty("styleSmall", true);
  m_spUi->CliButton->setProperty("styleSmall", true);

  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(m_spUi->LogTextEdit);
  m_pHighlightedSearchableEdit->SetSyntaxHighlightingEnabled(false);
  m_pZoomEnabler = new CTextEditZoomEnabler(m_spUi->LogTextEdit);

  m_pActionToggle = new QAction(pParent);
  m_pActionToggle->setShortcutContext(Qt::WindowShortcut);
  m_pActionToggle->setShortcut(m_spSettings->keyBinding("Debug"));
  pParent->addAction(m_pActionToggle);
  connect(m_pActionToggle, &QAction::triggered, this, [this]() {
    if (m_spSettings->DebugOverlayEnabled())
    {
      Toggle();
    }
  });

  if (m_spSettings->DebugOverlayEnabled())
  {
    CJoipMessageHandler::AddHandler(this);
  }
  connect(m_spSettings.get(), &CSettings::debugOverlayEnabledChanged, this,
          &CDebugOverlay::SlotDebugOverlayEnabledChanged);
  connect(m_spSettings.get(), &CSettings::keyBindingsChanged, this,
          &CDebugOverlay::SlotKeyBindingsChanged);

  m_pInstance = this;
}

CDebugOverlay::~CDebugOverlay()
{
  m_pInstance = nullptr;

  parentWidget()->removeAction(m_pActionToggle);
  CJoipMessageHandler::RemoveHandler(this);
}

//----------------------------------------------------------------------------------------
//
CDebugOverlay* CDebugOverlay::GetInstance()
{
  return m_pInstance;
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::Hide()
{
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::Resize()
{
  QRect rect(QPoint(0,0), parentWidget()->size());
  resize(rect.size());
  move(rect.topLeft());
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::Show()
{
  COverlayBase::Show();
}

//----------------------------------------------------------------------------------------
//
bool CDebugOverlay::MessageImpl(QtMsgType type, const QMessageLogContext& context,
                                const QString& sMsg)
{
  // we could be in a thread here
  QString sFormatedMsg =
      QString("[%1] %2:%3 -> %4")
          .arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss:zzz"))
          .arg(context.function)
          .arg(context.line)
          .arg(sMsg);
  bool bOk =
      QMetaObject::invokeMethod(this, "SlotMessageImpl", Qt::QueuedConnection,
                                Q_ARG(QtMsgType, type), Q_ARG(QString, sFormatedMsg));
  assert(bOk); Q_UNUSED(bOk)
  return false;
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::on_CloseButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::on_CliButton_clicked()
{
  CPositionalMenu modelMenu(EMenuPopupPosition::eRight | EMenuPopupPosition::eTop);

  QAction* pAction = new QAction(tr("js"), &modelMenu);
  QObject::connect(pAction, &QAction::triggered, pAction, [&]()
                   {
                     m_spUi->CliButton->setText(tr("js:"));
                     // TODO: logic
                   });
  modelMenu.addAction(pAction);

  QPoint p =
      m_spUi->CliButton->parentWidget()->mapToGlobal(
          m_spUi->CliButton->pos());
  modelMenu.exec(p);
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::on_LogInput_editingFinished()
{
  if (auto spDebugInterface = m_wpDebugInterface.lock())
  {
    QString sRet = spDebugInterface->TryEval(m_spUi->LogInput->text());
    m_spUi->LogOutput->setText(sRet);
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::SlotDebugOverlayEnabledChanged()
{
  if (m_spSettings->DebugOverlayEnabled())
  {
    CJoipMessageHandler::AddHandler(this);
  }
  else
  {
    CJoipMessageHandler::RemoveHandler(this);
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::SlotKeyBindingsChanged()
{
  m_pActionToggle->setShortcut(m_spSettings->keyBinding("Debug"));
}

//----------------------------------------------------------------------------------------
//
void CDebugOverlay::SlotMessageImpl(QtMsgType type, const QString& sMsg)
{
  QString sColor;
  QString sType;
  switch(type)
  {
    case QtMsgType::QtDebugMsg: sColor = "#ffffff"; sType = "DEBUG: "; break;
    case QtMsgType::QtInfoMsg: sColor = "#ffffff"; sType = "INFO: "; break;
    case QtMsgType::QtWarningMsg: sColor = "#FFBF00"; sType = "WARN: "; break;
    case QtMsgType::QtCriticalMsg: sColor = "#D22B2B"; sType = "ERROR: "; break;
    case QtMsgType::QtFatalMsg: sColor = "#D22B2B"; sType = "FATAL: "; break;
  }

  m_spUi->LogTextEdit->appendHtml(QString("<span style='color:%1'>%2%3</span>")
                                           .arg(sColor).arg(sType).arg(sMsg));
  m_spUi->LogTextEdit->moveCursor(QTextCursor::End);
}
