#ifndef SEQUENCEPROPERTIESOVERLAY_H
#define SEQUENCEPROPERTIESOVERLAY_H

#include "Widgets/OverlayBase.h"

#include <QPointer>
#include <QUndoStack>

#include <memory>

namespace Ui {
  class CSequencePropertiesOverlay;
}
typedef std::shared_ptr<struct SSequenceFile> tspSequence;

class CSequencePropertiesOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CSequencePropertiesOverlay(QWidget* pParent = nullptr);
  ~CSequencePropertiesOverlay();

  void UpdateUi();

  void SetSequence(const tspSequence& spSeq);
  tspSequence Sequence() const;
  void SetSequenceName(const QString& sName);
  const QString& SequenceName() const;

  void SetUndoStack(QPointer<QUndoStack> pUndo);

signals:
  void SignalContentsChanged();

public slots:
  void Climb() override;
  void Resize() override;

protected slots:
  void on_pTimeEdit_timeChanged(const QTime &time);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CSequencePropertiesOverlay> m_spUi;
  tspSequence                                     m_spCurrentSequence;
  QPointer<QUndoStack>                            m_pUndoStack;
  QString                                         m_sCurrentSequenceName;
  QSize                                           m_preferredSize;
};

#endif // SEQUENCEPROPERTIESOVERLAY_H
