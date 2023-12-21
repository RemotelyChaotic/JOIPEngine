#ifndef CWIDGETHELPERS_H
#define CWIDGETHELPERS_H

#include <QFileDialog>
#include <QWidget>

namespace widget_helpers
{
  QUrl GetExistingDirectoryUrl(QWidget* pParent,
                               const QString& sCaption,
                               const QUrl& dir,
                               QFileDialog::Options options,
                               const QStringList& vsSupportedSchemes);
  QUrl GetExistingFileUrl(QWidget* pParent,
                          const QString& sCaption,
                          const QUrl& dir,
                          QFileDialog::Options options,
                          const QStringList& vsSupportedSchemes);
  QString GetExistingDirectory(QWidget* pParent,
                               const QString& sCaption,
                               const QString& dir,
                               QFileDialog::Options options);
  QString GetExistingFile(QWidget* pParent,
                          const QString& sCaption,
                          const QString& dir,
                          QFileDialog::Options options);
  void RetainSizeAndHide(QWidget* pWidget);
}

#endif // CWIDGETHELPERS_H
