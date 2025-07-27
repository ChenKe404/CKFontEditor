/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	color_button.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef COLOR_BUTTON_H
#define COLOR_BUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget* parent = nullptr);
    void setColor(const QColor& color);
    const QColor& color();
signals:
    void colorChanged(const QColor&);
protected:
    void paintEvent(QPaintEvent *) override;
public:
    void onClicked();
private:
    QColor _color;
};

#endif // COLOR_BUTTON_H
