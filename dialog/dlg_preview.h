/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_preview.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_PREVIEW_H
#define DLG_PREVIEW_H

#include <QDialog>
#include <component/canvas.h>
#include <component/mydrawer.h>
#include "dlg_texture.h"

namespace Ui {
class DlgPreview;
}

class PreviewWidget : public Canvas
{
    Q_OBJECT
public:
    PreviewWidget(QWidget* parent);

    void draw(QPainter& p, bool transformed) override;

    void setFont(Font*);
    void setText(const QString& text);
    void setMixAlpha(uint8_t alpha);
    void setMixColor(QColor color);
    void setAlign(uint8_t align);
    void setArea(QSize area);
    void setSpacing(int x,int y);
    void setBreakWord(bool yes);
    void setMultiply(bool yes);
    void setTextureData(TextureData&& data);
    void useTextureData(bool yes);
    void showTextBox(bool yes);
private:
    Font* _font = nullptr;
    MyDrawer _drawer;
    FontDrawer::Options _opts;

    QSize _area; // 文本域宽高
    bool _show_textbox;
    uint8_t _mix_alpha = 0;
    bool _use_texture_data;
    TextureData _data;
};

class DlgPreview : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPreview(Font*);
    ~DlgPreview();
private:
    void onSpacing();
    void onArea();
    void onAlign();
private:
    Ui::DlgPreview *ui;
};

#endif // DLG_PREVIEW_H
