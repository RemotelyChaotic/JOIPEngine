#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QStyle>

class CFlowLayout : public QLayout
{
public:
    explicit CFlowLayout(QWidget* pParent, int iMargin = -1, int iHSpacing = -1, int iVSpacing = -1);
    explicit CFlowLayout(int iMargin = -1, int iHSpacing = -1, int iVSpacing = -1);
    ~CFlowLayout() override;

    void addItem(QLayoutItem* pItem) override;
    qint32 horizontalSpacing() const;
    qint32 verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem*itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;

    void insertItem(int index, QLayoutItem* pItem);
    void insertWidget(int index, QWidget* pWidget);
private:
    qint32 doLayout(const QRect& rect, bool testOnly) const;
    qint32 smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem*> m_vpItemList;
    qint32 m_iHSpace;
    qint32 m_iVSpace;
};

#endif // FLOWLAYOUT_H
