#ifndef CRICHTEXTEDIT_H
#define CRICHTEXTEDIT_H

#include <mtextedit.h>
#include <mrichtextedit.h>

class CRichTextEdit : public MRichTextEdit
{
  Q_OBJECT
public:
  explicit CRichTextEdit(QWidget* pParent = nullptr);
  ~CRichTextEdit() override;

signals:
  void UndoTriggered();
  void RedoTriggered();
};

#endif // CRICHTEXTEDIT_H