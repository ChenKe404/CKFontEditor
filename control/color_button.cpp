/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	color_button.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "color_button.h"
#include <QColorDialog>

ColorButton::ColorButton(QWidget *parent)
    : QPushButton(parent)
{
    connect(this,&QPushButton::clicked,this,&ColorButton::onClicked);
    setCursor(Qt::PointingHandCursor);
}

void ColorButton::setColor(const QColor &color)
{
    if(_color != color)
    {
        _color = color;
        emit colorChanged(color);
    }
}

const QColor &ColorButton::color()
{
    return _color;
}

void ColorButton::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    auto rc = rect();
    rc.adjust(0,0,-1,-1);
    p.fillRect(rc,_color);
    p.setPen(QColor(140,140,255));
    p.drawRect(rc);
}

void ColorButton::onClicked()
{
    QColorDialog dlg(_color);
    if(dlg.exec() == QColorDialog::Accepted)
    {
        setColor(dlg.selectedColor());
    }
}
