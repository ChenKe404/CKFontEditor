/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	flowlayout.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "flowlayout.h"

FlowLayout::FlowLayout(QWidget *parent)
    : QLayout(parent),
    _hSpacing(2),_vSpacing(2)
{}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
    {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item)
{
    _items.append(item);
}

int FlowLayout::count() const
{
    return _items.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return _items.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < _items.size())
        return _items.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return { };
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const auto item : _items)
    {
        size = size.expandedTo(item->minimumSize());
    }

    const auto& margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

void FlowLayout::setHorizontalSpacing(int spcing)
{
    _hSpacing = spcing;
}

int FlowLayout::horizontalSpacing() const
{
    return _hSpacing;
}

void FlowLayout::setVerticalSpacing(int spacing)
{
    _vSpacing = spacing;
}

int FlowLayout::verticalSpacing() const
{
    return _hSpacing;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (auto item : std::as_const(_items))
    {
        const QWidget *wid = item->widget();
        if(!wid->isVisible()) continue;
        int nextX = x + item->sizeHint().width() + _hSpacing;
        if (nextX - _hSpacing > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + _vSpacing;
            nextX = x + item->sizeHint().width() + _hSpacing;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}
