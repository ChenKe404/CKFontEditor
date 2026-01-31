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
    // 设置码点范围
    void setPage(const Page* page);
    // 选择整页
    void selectPage(const Page* page);
    // 取消选择整页
    void unselectPage(const Page* page);
    // 选择码点范围
    void selectRange(uint32_t first, uint32_t last);
    // 取消选择码点范围
    void unselectRange(uint32_t first, uint32_t last);
    // 设置被选择的码点
    void setCharset(const Charset& charset);
    // 清除已选择的码点
    void clear();
    // 当前已选定的码点
    const Charset& charset() const;

    void setFont(const font::TrueType* tt);
    void updatePageState(const Page* page = nullptr);
    void updatePagesState();
signals:
    void selected(uint32_t codepoint);
    void unselected(uint32_t codepoint);
    void pageStateChanged(const Page* page, int state);
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
    const Page* _page;      // 码点页指针
    Charset _charset;       // 已选择的字符集
    bool _drag_select;      // 拖拽时第一个字符是否是被选择状态
    uint32_t _count = 0;    // 有效字符数
    int _page_state = 0;    // 0:全部未选; 1:全部已选; 2:部分已选
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
    using Page = CharsetBlocks::Page;
public:
    explicit CharsetSelector(QWidget *parent = nullptr);
    ~CharsetSelector();

    void setCharset(const Charset& charset);
    const Charset& charset() const;
    void setCanvasFont(const font::TrueType* font);
private:
    void updateBlockList();
    void updateLabel();
    void onOpen();
    void onAppend();
    void onClear();
private:
    Ui::CharsetSelector *ui;
    CharsetPager _pager;
    QMenu _menu;
};

#endif // CHARSET_SELECTOR_H
