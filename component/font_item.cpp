/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	font_item.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "font_item.h"
#include <QGraphicsPixmapItem>
#include <QPaintEvent>
#include <QPainter>

static auto g_fmt_tooltip = FontItem::tr("字符: %1\n大小: %2 x %3 px");

//////////////////////////////////
/// FontItem

void FontItem::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.drawPixmap(rect(),_pix);
}

void FontItem::set(const Font::Char &chr)
{
    _chr = chr;
    QString s;
    s.append((wchar_t)chr.code);
    setToolTip(g_fmt_tooltip.arg(s).arg(chr.width).arg(chr.height));
}

char32_t FontItem::code() const
{ return _chr.code; }


//////////////////////////////////
/// FontItemDrawer

FontItemDrawer::FontItemDrawer(Font *font)
    : _font(font),
    _transparent(0,0,0),
    _background(190,190,190),
    _rate(1)
{
    _fnt_code = qApp->font();
    _fnt_code.setPixelSize(9);
    _fnt_char = _fnt_code;
    _fnt_char.setPixelSize(11);
    {
        QFontMetrics fm(_fnt_code);
        _sz_code = {fm.maxWidth(), fm.height()};
    }
    {
        QFontMetrics fm(_fnt_char);
        _sz_char = {fm.maxWidth(), fm.height()};
    }
}

void FontItemDrawer::setFontTransparent(const QColor &transparent)
{
    _transparent = transparent;
}

QColor FontItemDrawer::fontTransparent() const
{
    return _transparent;
}

void FontItemDrawer::setCharBackground(const QColor &background)
{
    _background = background;
}

QColor FontItemDrawer::charBackground() const
{
    return _background;
}

void FontItemDrawer::setCharRate(float rate)
{
    _rate = rate;
    _rate = std::max(0.25f,_rate);
    _rate = std::min(4.0f,_rate);
}

float FontItemDrawer::charRate() const
{
    return _rate;
}

constexpr int MINIMUM_SIZE = 16;
QSize FontItemDrawer::computeItemSize()
{
    const auto& header = _font->header();
    int w = (int)round(header.maxWidth * _rate) + 6;
    int h = (int)round(header.lineHeight * _rate) + 6;
    if(w < MINIMUM_SIZE)
        w = MINIMUM_SIZE;
    if(h < MINIMUM_SIZE)
        h = MINIMUM_SIZE;

    w += _sz_char.width() + 6;
    h += _sz_code.height() + 16;

    _size = { w,h };
    return _size;
}

void FontItemDrawer::drawItem(FontItem* item)
{
    drawItem(item->_chr,item->_pix);
}

void FontItemDrawer::drawItem(const Font::Char &chr, QPixmap& out)
{
    out = QPixmap(_size);
    const QRect rc{ 0,0,_size.width(),_size.height() };

    QPainter p(&out);
    // 填色和边框
    p.fillRect(rc,qRgb(240,240,240));
    p.setPen(qRgb(170,170,170));
    p.drawRect(rc.adjusted(0,0,-1,-1));

    auto rc1 = rc.adjusted(2,2,-1,-1);
    int x = rc1.left(),y = rc1.top();

    // 文本
    p.setPen(qRgb(40,40,40));
    p.setFont(_fnt_code);
    p.drawText(x,y + _sz_code.height(),QString::number(chr.code));

    y += _sz_code.height();
    y += (int)round((rc1.bottom() - y - _sz_char.height()) * 0.5f);
    p.setFont(_fnt_char);
    QString s;
    s.append((wchar_t)chr.code);
    QFontMetrics fm(_fnt_char);
    x += (int)round((_sz_char.width() - fm.horizontalAdvance(s)) * 0.5f);
    p.drawText(x,y + _sz_char.height(),s);

    // 图像
    x = rc1.left() + _sz_char.width() + 2;
    y = rc1.top() + _sz_code.height() + 2;
    QRect rc2 {
        x,y,
        rc1.right() - x,
        rc1.bottom() - y
    };
    p.fillRect(rc2,_background);
    p.drawRect(rc2);

    QPixmap pix;
    drawChar(chr,pix);
    if(_rate != 1)
        pix = pix.scaled({ (int)floor(pix.width() * _rate),(int)floor(pix.height() * _rate) });
    x += (int)round((rc2.width() - pix.width()) * 0.5f);
    y += (int)round((rc2.height() - pix.height()) * 0.5f);
    p.drawPixmap(x,y,pix);
}

void FontItemDrawer::drawChar(const Font::Char &chr, QPixmap &out)
{
    const auto trans = toCKColor(_transparent);
    const auto bkg = toCKColor(_background);
    const auto bit32 = _font->header().flag & Font::FL_BIT32;

    const auto w = chr.width;
    const auto h = chr.height;

    std::vector<color> buf;
    buf.reserve(w * h);
    for(int y=0; y<h; ++y)
    {
        for(int x=0; x<w; ++x)
        {
            auto color = _font->getColor(chr,x,y);
            if(!bit32 && color == trans)
                buf.push_back(0);   // 24位色遇到透明颜色
            else
                buf.push_back(color);
        }
    }
    out = QPixmap::fromImage(QImage((uint8_t*)buf.data(),w,h,QImage::Format_ARGB32));
}

//////////////////////////////////
/// FontItemDrawerThread

FontItemDrawerThread::FontItemDrawerThread(FontItemDrawer *drawer)
    : _drawer(drawer),
    _running(false)
{}

FontItemDrawerThread::~FontItemDrawerThread()
{
    stop();
}

void FontItemDrawerThread::add(FontItem *item)
{
    _mtx.lock();
    _list.push_back(item);
    _mtx.unlock();
}

void FontItemDrawerThread::stop()
{
    if(_running)
    {
        _running = false;
        wait();
    }
}

void FontItemDrawerThread::run()
{
    _running = true;
    const bool has = !_list.empty();
    while(_running)
    {
        if(_list.empty()) break;

        _mtx.lock();
        auto item = _list.back();
        _mtx.unlock();

        _drawer->drawItem(item);

        _mtx.lock();
        _list.pop_back();
        _mtx.unlock();
    }
    if(has) emit done();
    _running = false;
}

