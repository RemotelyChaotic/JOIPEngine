#ifndef HELPOVERLAY_H
#define HELPOVERLAY_H

#include "OverlayButton.h"
#include <QAction>
#include <QPaintEvent>
#include <QPointer>
#include <QTimer>
#include <QTextCursor>
#include <QWidget>
#include <memory>

class CHelpButtonOverlay;
class CHelpFactory;
class CHighlightedSearchableTextEdit;
class CSettings;
class CShortcutButton;
class CTextEditZoomEnabler;
namespace Ui {
  class CHelpOverlay;
}
class QPixmapDropShadowFilter;
class QPropertyAnimation;
class QStandardItem;
class QTextBrowser;

namespace helpOverlay {
  extern const char* const c_sHelpPagePropertyName;
}

//----------------------------------------------------------------------------------------
//
class CHelpOverlayBackGround : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(qint32 circleRadius MEMBER m_iCircleRadius)
  friend class CHelpOverlay;

public:
  explicit CHelpOverlayBackGround(QWidget* pParent = nullptr);
  ~CHelpOverlayBackGround() override;

  void Reset();
  void StartAnimation(const QPoint& origin, qint32 iEndValue);

public slots:
  void SlotCircleAnimationFinished();
  void SlotUpdate();

signals:
  void SignalCircleAnimationFinished();

protected:
  void mouseMoveEvent(QMouseEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvt) override;

  bool                                        m_bOpened;
  QPoint                                      m_circleOrigin;
  qint32                                      m_iCircleRadius;

private:
  void MousePositionCheck();

  //QPointer<QPixmapDropShadowFilter>             m_pDropShadowFilter;
  QPointer<QPropertyAnimation>                  m_pCircleAnimation;
  std::map<QWidget*, std::pair<QRect, QImage>>  m_shownIconImages;
  QPoint                                        m_cursor;
  QTimer                                        m_updateTimer;
};

//----------------------------------------------------------------------------------------
//
class CHelpOverlay : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(QColor linkColor READ LinkColor WRITE SetLinkColor NOTIFY SignalLinkColorChanged)
  friend class CHelpOverlayBackGround;

public:
  explicit CHelpOverlay(QPointer<CHelpButtonOverlay> pHelpButton, QWidget* pParent = nullptr);
  ~CHelpOverlay() override;

  static QPointer<CHelpOverlay> Instance();

  QColor LinkColor() const;
  void SetLinkColor(const QColor& col);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show(const QPoint& animationOrigin, QWidget* pRootToSearch);

signals:
  void SignalLinkColorChanged();
  void SignalOverlayOpened();
  void SignalOverlayClosed();

protected slots:
  void Show() override;
  void SlotCircleAnimationFinished();
  void SlotKeyBindingsChanged();
  void on_pHtmlBrowser_backwardAvailable(bool bAvailable);
  void on_pHtmlBrowser_forwardAvailable(bool bAvailable);
  void on_BackButton_clicked();
  void on_HomeButton_clicked();
  void on_ForardButton_clicked();
  void on_CloseButton_clicked();
  void on_pHtmlBrowser_sourceChanged(const QUrl& source);
  void SlotCurrentIndexChanged(const QModelIndex& current, const QModelIndex& previous);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

  std::vector<QPointer<QWidget>>    m_vpHelpWidgets;

private:
  void InitTree();
  void InitTreeBranch(QStandardItem* pParent, const QStringList& vsData, const QString& sPath);
  void ShowHelp(const QString sKey);

  static QPointer<CHelpOverlay> m_pHelpOverlay;

  std::unique_ptr<Ui::CHelpOverlay> m_spUi;
  std::shared_ptr<CSettings>        m_spSettings;
  std::weak_ptr<CHelpFactory>       m_wpHelpFactory;
  QPointer<CHighlightedSearchableTextEdit> m_pHighlightedSearchableEdit;
  QPointer<CTextEditZoomEnabler>    m_pZoomEnabler;
  QPointer<CHelpButtonOverlay>      m_pHelpButton;
  QPointer<CHelpOverlayBackGround>  m_pBackground;
  QColor                            m_linkColor;
};

//----------------------------------------------------------------------------------------
//
class CHelpButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  explicit CHelpButtonOverlay(QWidget* pParent = nullptr);
  ~CHelpButtonOverlay() override;

public slots:
  void Resize() override;
};

#endif // HELPOVERLAY_H
