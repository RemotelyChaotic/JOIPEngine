#ifndef CANDROIDNAVIGATIONBAR_H
#define CANDROIDNAVIGATIONBAR_H

#include <QColor>
#include <QObject>

class CAndroidNavigationBar : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QColor color READ Color WRITE SetColor)

public:
  CAndroidNavigationBar(QObject* pParent);

  static bool IsAvailable();

  static QColor Color();
  static void SetColor(const QColor &color);

private:
  static QColor     m_color;
};

#endif // CANDROIDNAVIGATIONBAR_H
