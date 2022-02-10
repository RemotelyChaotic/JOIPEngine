#include "ShortcutButton.h"

#include <QAction>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  void drawText(QPainter& painter, double x, double y, Qt::Alignment flags,
                const QString& sText, QRectF* pBoundingRect = nullptr)
  {
     const qreal size = 32767.0;
     QPointF corner(x, y - size);
     if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
     else if (flags & Qt::AlignRight) corner.rx() -= size;
     if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
     else if (flags & Qt::AlignTop) corner.ry() += size;
     else flags |= Qt::AlignBottom;
     QRectF rect{corner.x(), corner.y(), size, size};
     painter.drawText(rect, flags, sText, pBoundingRect);
  }
}

//----------------------------------------------------------------------------------------
//
CShortcutButton::CShortcutButton(QWidget* pParent) :
  QPushButton(pParent),
#if defined(Q_OS_ANDROID)
#else
  m_pAction(new QAction(this)),
#endif
  m_sBaseToolTip(),
  m_bShortCutAsTextEnabled(false)
{
#if defined(Q_OS_ANDROID)
#else
  connect(m_pAction, &QAction::triggered, this, &QPushButton::clicked, Qt::DirectConnection);
  addAction(m_pAction);
#endif
}
CShortcutButton::CShortcutButton(const QString& text, QWidget* pParent) :
  QPushButton(text, pParent),
#if defined(Q_OS_ANDROID)
#else
  m_pAction(new QAction(this)),
#endif
  m_sBaseToolTip(),
  m_bShortCutAsTextEnabled(false)
{
#if defined(Q_OS_ANDROID)
#else
  connect(m_pAction, &QAction::triggered, this, &QPushButton::clicked, Qt::DirectConnection);
  addAction(m_pAction);
#endif
}
CShortcutButton::CShortcutButton(const QIcon& icon, const QString& text, QWidget* pParent) :
  QPushButton(icon, text, pParent),
#if defined(Q_OS_ANDROID)
#else
  m_pAction(new QAction(this)),
#endif
  m_sBaseToolTip(),
  m_bShortCutAsTextEnabled(false)
{
#if defined(Q_OS_ANDROID)
#else
  connect(m_pAction, &QAction::triggered, this, &QPushButton::clicked, Qt::DirectConnection);
  addAction(m_pAction);
#endif
}
CShortcutButton::~CShortcutButton() {}


//----------------------------------------------------------------------------------------
//
void CShortcutButton::SetShortcut(const QKeySequence& sequence)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(sequence)
#else
  m_pAction->setShortcut(sequence);
  QPushButton::setToolTip(m_sBaseToolTip + "<br><i>" + m_pAction->shortcut().toString() + "</i>");
#endif
}

//----------------------------------------------------------------------------------------
//
bool CShortcutButton::IsShortcutAsTextEnabled() const
{
  return m_bShortCutAsTextEnabled;
}

//----------------------------------------------------------------------------------------
//
void CShortcutButton::EnableShortcutAsText(bool bEnabled)
{
  if (m_bShortCutAsTextEnabled != bEnabled)
  {
    m_bShortCutAsTextEnabled = bEnabled;
    emit shortcutAsTextChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CShortcutButton::setToolTip(const QString& sText)
{
  m_sBaseToolTip = sText;
  QPushButton::setToolTip(m_sBaseToolTip
#if defined(Q_OS_ANDROID)
#else
                          + "<br><i>" + m_pAction->shortcut().toString() + "</i>"
#endif
   );
}

//----------------------------------------------------------------------------------------
//
void CShortcutButton::paintEvent(QPaintEvent* pEvt)
{
  QPushButton::paintEvent(pEvt);

#if defined(Q_OS_ANDROID)
#else
  if (m_bShortCutAsTextEnabled)
  {
    QPainter painter(this);
    QFont painterFont = painter.font();
    painterFont.setPixelSize(10);
    painter.setFont(painterFont);

    const QString sText = m_pAction->shortcut().toString();

    QFontMetrics fontMetrics(font(), this);
    QRect textRect = fontMetrics.boundingRect(sText);
    if (sText.length() == 1) textRect.setWidth(textRect.width()*2);

    drawText(painter,
             rect().x() + rect().width() - textRect.width(),
             rect().y() + rect().height() - textRect.height(),
             Qt::AlignLeft | Qt::AlignTop, sText);
  }
  #endif
}
