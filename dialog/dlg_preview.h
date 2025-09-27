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
#include <drawer.h>
#include "dlg_texture.h"

namespace Ui {
class DlgPreview;
}

struct Drawer : public FontDrawer
{
    void perchar(int x,int y, const Font::Char* chr, const Font::DataPtr& d) const override;
    void setPainter(QPainter*);
    void setMultiply(bool yes);
protected:
    QPainter* _p;
    QPixmap _pix;
    bool _multiply = false;
};

struct TextureDrawer : public Drawer
{
    void setTextureData(const TextureData* data);
    void perchar(int x,int y, const Font::Char* chr, const Font::DataPtr& d) const override;
private:
    const TextureData* _data = nullptr;
};

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
    Drawer _drawer;
    TextureDrawer _drawer;
    Drawer::Options _opts;
    Font::CharPtrList _chrs;

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

    bool _use_texture = false;
};

#endif // DLG_PREVIEW_H
