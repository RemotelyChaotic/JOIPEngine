#ifndef CLIPBOARDQMLWRAPPER_H
#define CLIPBOARDQMLWRAPPER_H

#include <QClipboard>
#include <QObject>
#include <QPointer>

class CClipboardQmlWrapper : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool supportsSelection READ SupportsSelection)
  Q_PROPERTY(bool supportsFindBuffer READ SupportsFindBuffer)
  Q_PROPERTY(bool ownsSelection READ OwnsSelection)
  Q_PROPERTY(bool ownsClipboard READ OwnsClipboard)
  Q_PROPERTY(bool ownsFindBuffer READ OwnsFindBuffer)
  Q_PROPERTY(QString text READ Text WRITE SetText)
  Q_PROPERTY(ClipboardMode mode READ Mode WRITE SetMode NOTIFY modeChanged)

public:
  enum ClipboardMode { Clipboard, Selection, FindBuffer, LastMode = FindBuffer };
  Q_ENUM(ClipboardMode)

  explicit CClipboardQmlWrapper(QObject* pParent = nullptr);
  ~CClipboardQmlWrapper();

  bool SupportsSelection() const;
  bool SupportsFindBuffer() const;

  bool OwnsSelection() const;
  bool OwnsClipboard() const;
  bool OwnsFindBuffer() const;

  ClipboardMode Mode() const;
  void SetMode(ClipboardMode);

  QString Text() const;
  void SetText(const QString &);

public slots:
  void clear();
  QString text(QString& subtype) const;

signals:
  void changed(int mode);
  void selectionChanged();
  void findBufferChanged();
  void dataChanged();
  void modeChanged(ClipboardMode mode);

private:
  QPointer<QClipboard>    m_pClipboard;
  ClipboardMode           m_mode;
};

Q_DECLARE_METATYPE(CClipboardQmlWrapper*)
Q_DECLARE_METATYPE(CClipboardQmlWrapper::ClipboardMode)

#endif // CLIPBOARDQMLWRAPPER_H
