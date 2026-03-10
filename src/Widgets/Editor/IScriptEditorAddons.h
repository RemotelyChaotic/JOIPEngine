#ifndef ISCRIPTEDITORADDONS_H
#define ISCRIPTEDITORADDONS_H

#include <QWidget>

class IScriptEditorAddon
{
protected:
  IScriptEditorAddon() = default;

public:
  virtual ~IScriptEditorAddon() = default;

  virtual qint32 AreaHeight() const = 0;
  virtual qint32 AreaWidth() const = 0;
  virtual void Reset() = 0;
  virtual void Update(const QRect& rect, qint32 iDy) = 0;
};

#endif // ISCRIPTEDITORADDONS_H
