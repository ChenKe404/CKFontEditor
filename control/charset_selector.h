#ifndef CHARSET_SELECTOR_H
#define CHARSET_SELECTOR_H

#include <QThread>
#include <QWidget>
#include "../src/charset_blocks.h"

namespace Ui {
class CharsetSelector;
}

class CharsetCanvas : public QWidget
{
    Q_OBJECT
public:
    CharsetCanvas(QWidget* parent = nullptr);
    // 设置码点范围, 最多从begin到begin+256
    void setRange(uint32_t begin, uint32_t end);
    // 选择码点范围, 最多从begin到begin+256
    void selectRange(uint32_t begin, uint32_t end);
    // 选择码点, 在begin到begin+256范围内
    void selectCharset(const std::vector<uint32_t>& codes);
    // 当前已选定的码点
    const std::set<uint32_t>& selected() const;
    // 当前起始码点到结束码点, 总共可用字符数
    uint32_t available() const;
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    QSizeF cellSize() const;
    void toggle(const QPoint& pt,bool drag);
private:
    QPixmap _backgrid;      // 背景网格图片
    uint32_t _begin = 0, _end = 0;  // 起始码点, 中止码点
    std::set<uint32_t> _selected;   // 已选择的字符集
    std::set<uint32_t> _darged;     // 当前已拖拽字符集
};

class CharsetSelector : public QWidget
{
    Q_OBJECT
public:
    explicit CharsetSelector(QWidget *parent = nullptr);
    ~CharsetSelector();

    const std::set<uint32_t>& charset() const;
    void setCanvasFont(const QFont& font);
private:
    void updateBlockList();
private:
    Ui::CharsetSelector *ui;
    std::set<uint32_t> _chrset;
    CharsetPager _pager;
};

#endif // CHARSET_SELECTOR_H
