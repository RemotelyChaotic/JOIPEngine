#ifndef DATABASENOTIFIER_H
#define DATABASENOTIFIER_H

#include <QObject>

class CDatabaseNotifier : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CDatabaseNotifier)

public:
  CDatabaseNotifier();
  ~CDatabaseNotifier() override;

signals:
  void SignalProjectAdded(qint32 iId);
  void SignalProjectRenamed(qint32 iId);
  void SignalProjectRemoved(qint32 iId);
  void SignalSceneAdded(qint32 iProjId, qint32 iId);
  void SignalSceneDataChanged(qint32 iProjId, qint32 iId);
  void SignalSceneRenamed(qint32 iProjId, qint32 iId);
  void SignalSceneRemoved(qint32 iProjId, qint32 iId);
  void SignalReloadFinished();
  void SignalReloadStarted();
  void SignalResourceAdded(qint32 iProjId, const QString& sName);
  void SignalResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SignalResourceRemoved(qint32 iProjId, const QString& sName);
  void SignalTagAdded(qint32 iProjId, const QString& sResource, const QString& sName);
  void SignalTagRemoved(qint32 iProjId, const QString& sResource, const QString& sName);
  void SignalAchievementAdded(qint32 iProjId, const QString& sName);
  void SignalAchievementRemoved(qint32 iProjId, const QString& sName);
  void SignalAchievementDataChanged(qint32 iProjId, const QString& sName);
  void SignalAchievementRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
};

#endif // DATABASENOTIFIER_H
