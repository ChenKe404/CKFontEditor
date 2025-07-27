/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	font_item.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef FONT_ITEM_H
#define FONT_ITEM_H

#include <QWidget>
#include <QThread>
#include <font.h>

class FontItem : public QWidget
{
    Q_OBJECT
public:
    void paintEvent(QPaintEvent*) override;

    void set(const Font::Char&);    // 设置当前的字符
    char32_t code() const;
private:
    friend class FontItemDrawer;
    Font::Char _chr;
    QPixmap _pix;
};

class FontItemDrawer
{
public:
    FontItemDrawer(Font* font);
    // 字体透明色
    void setFontTransparent(const QColor&);
    QColor fontTransparent() const;

    // 字符背景色
    void setCharBackground(const QColor&);
    QColor charBackground() const;

    // 字符缩放比例
    void setCharRate(float rate);
    float charRate() const;

    QSize computeItemSize();
    void drawItem(FontItem* item);
    void drawItem(const Font::Char& chr, QPixmap& out);
    void drawChar(const Font::Char& chr, QPixmap& out);
private:
    QFont _fnt_code,_fnt_char;
    QSize _sz_code, _sz_char;
    Font* _font;
    QColor _transparent,_background;
    float _rate;
    QSize _size;
};

// 字体项绘制线程
class FontItemDrawerThread : public QThread
{
    Q_OBJECT
public:
    FontItemDrawerThread(FontItemDrawer*);
    ~FontItemDrawerThread();
    void add(FontItem*);    // 线程应该加入所有字体项之后再start
    void stop();    // 停止当前工作, 阻塞的
signals:
    void done();    // 当run函数结束并且有item被绘制时发出
protected:
    void run() override;
private:
    bool _running;
    std::mutex _mtx;
    FontItemDrawer* _drawer;
    QList<FontItem*> _list;
};

#endif // FONT_ITEM_H
