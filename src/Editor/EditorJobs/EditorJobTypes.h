#ifndef EDITORJOBTYPES_H
#define EDITORJOBTYPES_H

#include "Systems/IRunnableJob.h"
#include <QString>

namespace editor_job
{
  const QString c_sExport = "Export";
}

//----------------------------------------------------------------------------------------
//
class IEditorJob : public IRunnableJob
{
public:
  ~IEditorJob() override {};

  virtual QString ReturnValue() const = 0;

protected:
  IEditorJob() : IRunnableJob() {}

private:
  Q_DISABLE_COPY(IEditorJob)
};


#endif // EDITORJOBTYPES_H
