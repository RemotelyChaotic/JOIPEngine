#ifndef IEDITORTOOL_H
#define IEDITORTOOL_H

#include <QString>

class IEditorToolBox
{
public:
  virtual QStringList Tools() const = 0;
  virtual void ToolTriggered(const QString& sTool) = 0;

protected:
  virtual ~IEditorToolBox() {}
};

#endif // IEDITORTOOL_H
