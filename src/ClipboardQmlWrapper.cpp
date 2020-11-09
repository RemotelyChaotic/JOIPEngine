#include "ClipboardQmlWrapper.h"
#include <QApplication>

CClipboardQmlWrapper::CClipboardQmlWrapper(QObject* pParent) :
  QObject(pParent),
  m_pClipboard(QApplication::clipboard()),
  m_mode(Clipboard)
{
  if (nullptr != m_pClipboard)
  {
    connect(m_pClipboard.data(), &QClipboard::changed, this, &CClipboardQmlWrapper::changed);
    connect(m_pClipboard.data(), &QClipboard::selectionChanged, this, &CClipboardQmlWrapper::selectionChanged);
    connect(m_pClipboard.data(), &QClipboard::findBufferChanged, this, &CClipboardQmlWrapper::findBufferChanged);
    connect(m_pClipboard.data(), &QClipboard::dataChanged, this, &CClipboardQmlWrapper::dataChanged);
  }
}

CClipboardQmlWrapper::~CClipboardQmlWrapper() {}

//----------------------------------------------------------------------------------------
//
bool CClipboardQmlWrapper::SupportsSelection() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->supportsSelection();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CClipboardQmlWrapper::SupportsFindBuffer() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->supportsFindBuffer();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CClipboardQmlWrapper::OwnsSelection() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->ownsSelection();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CClipboardQmlWrapper::OwnsClipboard() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->ownsClipboard();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CClipboardQmlWrapper::OwnsFindBuffer() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->ownsFindBuffer();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
CClipboardQmlWrapper::ClipboardMode CClipboardQmlWrapper::Mode() const
{
  return m_mode;
}

//----------------------------------------------------------------------------------------
//
void CClipboardQmlWrapper::SetMode(CClipboardQmlWrapper::ClipboardMode mode)
{
  if (m_mode != mode)
  {
    m_mode = mode;
    emit modeChanged(mode);
  }
}

//----------------------------------------------------------------------------------------
//
QString CClipboardQmlWrapper::Text() const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->text(static_cast<QClipboard::Mode>(m_mode));
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CClipboardQmlWrapper::SetText(const QString& sText)
{
  if (nullptr != m_pClipboard)
  {
    m_pClipboard->setText(sText, static_cast<QClipboard::Mode>(m_mode));
  }
}

//----------------------------------------------------------------------------------------
//
void CClipboardQmlWrapper::clear()
{
  if (nullptr != m_pClipboard)
  {
    m_pClipboard->clear(static_cast<QClipboard::Mode>(m_mode));
  }
}

//----------------------------------------------------------------------------------------
//
QString CClipboardQmlWrapper::text(QString& subtype) const
{
  if (nullptr != m_pClipboard)
  {
    return m_pClipboard->text(subtype, static_cast<QClipboard::Mode>(m_mode));
  }
  return QString();
}
