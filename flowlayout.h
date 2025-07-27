/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	flowlayot.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef FLOW_LAYOUT_H
#define FLOW_LAYOUT_H

#include <QLayout>

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent = nullptr);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

    void setHorizontalSpacing(int);
    int horizontalSpacing() const;

    void setVerticalSpacing(int);
    int verticalSpacing() const;
private:
    int doLayout(const QRect &rect, bool testOnly) const;

    QList<QLayoutItem *> _items;
    int _hSpacing, _vSpacing;
};

#endif // FLOW_LAYOUT_H
