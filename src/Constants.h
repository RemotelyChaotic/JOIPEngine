#ifndef CONSTANTS_H
#define CONSTANTS_H

// default colors
#define BLACK 0, 0, 0
#define BLACK_QML "black"

#define RED 255, 0, 0
#define RED_QML "red"

#define YELLOW 255, 255, 0
#define YELLOW_QML "yellow"

#define WHITE 255, 255, 255
#define WHITE_QML "white"

#define SILVER 192, 192, 192
#define SILVER_QML "silver"

// default keyboard shortcuts
#define KEY_COMMIT  \
  QList<QKeySequence>() << \
    QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_Enter)

#define KEY_ESCAPE  \
  QList<QKeySequence>() << QKeySequence(Qt::Key_Escape)

#endif // CONSTANTS_H
