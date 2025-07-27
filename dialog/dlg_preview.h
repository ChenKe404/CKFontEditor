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
#include <drawer.h>

namespace Ui {
class DlgPreview;
}

struct Drawer : public FontDrawer
{
    void perchar(int x,int y, const Font::Char* chr, const Font::DataPtr& d) const override;
    void setPainter(QPainter*);
    void setMultiply(bool yes);
private:
    QPainter* _p;
    QPixmap _pix;
    bool _multiply = false;
};

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget* parent);

    void paintEvent(QPaintEvent*e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent*e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    void setFont(Font*);
    void setText(const QString& text);
    void setBackground(QColor color);
    void setMixAlpha(uint8_t alpha);
    void setMixColor(QColor color);
    void setAlign(uint8_t align);
    void setScale(float scale);
    void setArea(QSize area);
    void setSpacing(int x,int y);
    void setBreakWord(bool yes);
    void setMultiply(bool yes);
    void showTextBox(bool yes);
signals:
    void scale(float scale);
private:
    Font* _font = nullptr;
    Drawer _drawer;
    Drawer::Options _opts;
    Font::CharPtrList _chrs;

    QColor _background;
    QSize _area; // 文本域宽高
    bool _show_textbox;
    uint8_t _mix_alpha = 0;

    float _scale;   // 缩放
    QPointF _start,_offset;  // 拖拽开始/拖拽偏移
    bool _drag;
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
