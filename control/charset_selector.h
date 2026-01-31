#ifndef CHARSET_SELECTOR_H
#define CHARSET_SELECTOR_H

#include <QMenu>
#include <QThread>
#include <QWidget>
#include "../src/charset_blocks.h"

namespace Ui {
class CharsetSelector;
}

using Charset = std::set<uint32_t>;

class CharsetCanvas : public QWidget
{
    Q_OBJECT
    using Page = CharsetBlocks::Page;
public:
    CharsetCanvas(QWidget* parent = nullptr);
    // 设置码点范围, 最多从begin到begin+256
    void setRange(uint32_t begin, uint32_t end);
    // 选择码点范围, 最多从begin到begin+256
    void selectRange(uint32_t begin, uint32_t end);
    // 选择码点, 在begin到begin+256范围内
    void selectCharset(const std::vector<uint32_t>& codepoints);
    // 当前已选定的码点
    const Charset& selected() const;
    // 当前起始码点到结束码点, 总共可用字符数
    uint32_t available() const;

    void setFont(const font::TrueType* tt);
signals:
    void selected(uint32_t codepoint);
    void unselected(uint32_t codepoint);
    void selectedAll();
    void unselectedAll();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    QSizeF cellSize() const;
    void toggle(const QPoint& pt,bool drag);
    // 传入当前需要的字符高度, 按需求更新缓存
    void updateCache(int fontHeight, bool force = false);
    void makePixmap(QPixmap& out, uint32_t codepoint);
    const QPixmap* get(uint32_t codepoint);
private:
    QPixmap _backgrid;      // 背景网格图片
    uint32_t _begin = 0, _end = 255;    // 起始码点, 结束码点
    Charset _selected;      // 已选择的字符集
    bool _drag_select;      // 拖拽时第一个字符是否是被选择状态
    uint32_t _count = 0;    // 有效字符数
    const font::TrueType* _tt;
    std::map<uint32_t,QPixmap> _cache;  // 字符图像缓存
    int _font_height = 0;       // 缓存字符图像的高度
    double _font_scale = 1;     // 缓存字符图像的缩放
    int _bound_w = 1;           // 字体包围宽度
    int _bound_h = 1;           // 字体包围高度
};

class CharsetSelector : public QWidget
{
    Q_OBJECT
public:
    explicit CharsetSelector(QWidget *parent = nullptr);
    ~CharsetSelector();

    const Charset& charset() const;
    void setCanvasFont(const font::TrueType* font);
private:
    void updateBlockList();
    void onOpen();
    void onAppend();
    void onClear();
private:
    Ui::CharsetSelector *ui;
    Charset _charset;
    CharsetPager _pager;
    QMenu _menu;
};

#endif // CHARSET_SELECTOR_H
