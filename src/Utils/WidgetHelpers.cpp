#include "WidgetHelpers.h"
#include "RaiiFunctionCaller.h"

#include <private/qfiledialog_p.h>

namespace
{
  class CFileDialog : public QFileDialog
  {
    Q_OBJECT
  public:
    CFileDialog(QWidget* pParent, Qt::WindowFlags f) : QFileDialog(pParent, f) {}
    explicit CFileDialog(QWidget* pParent = nullptr,
                         const QString& sCaption = QString(),
                         const QString& sDirectory = QString(),
                         const QString& sFilter = QString()) :
      QFileDialog(pParent, sCaption, sDirectory, sFilter)
    {}

  protected:
    bool eventFilter(QObject* pParent, QEvent* pEvt) override
    {
      if (nullptr != pParent && nullptr != pEvt)
      {
#if defined(Q_OS_ANDROID)
        if (pEvt->type() == QEvent::Resize)
        {
          if (nullptr != parentWidget())
          {
            setGeometry(parentWidget()->window()->geometry());
          }
        }
#endif
      }
      return false;
    }
  };

  //--------------------------------------------------------------------------------------
  // code modified from qfiledialog.cpp
  QUrl GetExistingFileOrDirectoryUrl(QWidget* pParent,
                                     const QString& sCaption,
                                     const QUrl& dir,
                                     QFileDialog::Options options,
                                     const QStringList& vsSupportedSchemes,
                                     bool bDirMode)
  {
    QPointer<QWidget> pParentWrapper(pParent);

#if defined(Q_OS_ANDROID)
    CFileDialog dialog(pParent, sCaption, QFileDialogPrivate::workingDirectory(dir).toLocalFile());
#else
    QFileDialog dialog(pParent, sCaption, QFileDialogPrivate::workingDirectory(dir).toLocalFile());
#endif
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    if (bDirMode)
    {
      dialog.setFileMode(options & QFileDialog::ShowDirsOnly ? QFileDialog::DirectoryOnly : QFileDialog::Directory);
    }
    else
    {
      dialog.setFileMode(QFileDialog::ExistingFile);
    }
QT_WARNING_POP
    dialog.setOptions(options);
    dialog.setSupportedSchemes(vsSupportedSchemes);

#if defined(Q_OS_ANDROID)
    // on Android we need to set geometry or else we wont be able to see the window
    dialog.setGeometry(pParent->window()->geometry());
    // we also don't want to use the native dialog, because the buttons are too large
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    if (nullptr != pParentWrapper)
    {
      pParent->window()->installEventFilter(&dialog);
    }
    QList<QWidget*> vpButtons = dialog.findChildren<QWidget*>();
    for (QWidget* pButton : qAsConst(vpButtons))
    {
      pButton->setProperty("styleSmall", true);
    }
#endif

    if (dialog.exec() == QDialog::Accepted)
    {
      if (nullptr != pParentWrapper)
      {
        pParent->window()->removeEventFilter(&dialog);
        return dialog.selectedUrls().value(0);
      }
    }
    if (nullptr != pParentWrapper)
    {
      pParent->window()->removeEventFilter(&dialog);
    }
    return QUrl();
  }
}

#include "WidgetHelpers.moc"

namespace widget_helpers
{
  //--------------------------------------------------------------------------------------
  //
  QUrl GetExistingDirectoryUrl(QWidget* pParent,
                               const QString& sCaption,
                               const QUrl& dir,
                               QFileDialog::Options options,
                               const QStringList& vsSupportedSchemes)
  {
    return GetExistingFileOrDirectoryUrl(pParent, sCaption, dir, options, vsSupportedSchemes, true);
  }

  //--------------------------------------------------------------------------------------
  //
  QUrl GetExistingFileUrl(QWidget* pParent,
                          const QString& sCaption,
                          const QUrl& dir,
                          QFileDialog::Options options,
                          const QStringList& vsSupportedSchemes)
  {
    return GetExistingFileOrDirectoryUrl(pParent, sCaption, dir, options, vsSupportedSchemes, false);
  }

  //--------------------------------------------------------------------------------------
  //
  QString GetExistingDirectory(QWidget* pParent,
                               const QString& sCaption,
                               const QString& dir,
                               QFileDialog::Options options)
  {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl = GetExistingDirectoryUrl(pParent, sCaption,
                                                     QUrl::fromLocalFile(dir),
                                                     options, schemes);
    return selectedUrl.toLocalFile();
  }

  //--------------------------------------------------------------------------------------
  //
  QString GetExistingFile(QWidget* pParent,
                          const QString& sCaption,
                          const QString& dir,
                          QFileDialog::Options options)
  {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl = GetExistingFileUrl(pParent, sCaption,
                                                QUrl::fromLocalFile(dir),
                                                options, schemes);
    return selectedUrl.toLocalFile();
  }

  //--------------------------------------------------------------------------------------
  //
  void RetainSizeAndHide(QWidget* pWidget)
  {
    if (nullptr != pWidget)
    {
      QSizePolicy policy = pWidget->sizePolicy();
      policy.setRetainSizeWhenHidden(true);
      pWidget->setSizePolicy(policy);
      pWidget->hide();
    }
  }
}
