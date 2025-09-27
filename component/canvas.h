#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent*e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent*e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    virtual void draw(QPainter& p, bool transformed) {};

    void setPadding(int padding);
    int padding() const;

    // 设置坐标轴原点
    // @rx,ry 原点在画布的水平/垂直比例
    void setOrigin(float rx,float ry);
    QPointF origin() const;
    QPoint originPos() const;

    void setBackground(QColor color);
    QColor background() const;

    void setScale(float scale);
signals:
    void scale(float scale);
protected:
    QColor _background;
    int _padding;
    float _rx,_ry;
    float _scale;   // 缩放
    QPointF _start,_offset;  // 拖拽开始/拖拽偏移
    bool _drag;
};

#endif // CANVAS_H
