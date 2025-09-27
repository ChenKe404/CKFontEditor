#include "canvas.h"

#include <QWheelEvent>

Canvas::Canvas(QWidget *parent)
    : QWidget{parent},
    _padding(5),
    _scale(1),_offset(0,0),
    _rx(0.5),_ry(0.5),
    _drag(false)
{}

void Canvas::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    const auto rc = rect();
    QPainter p(this);
    p.setRenderHints(p.Antialiasing);
    const auto bkg = _background;
    p.fillRect(rc,bkg);
    p.translate(_offset);
    auto op = originPos();

    // 水平原点线
    {
        p.setPen(qRgb(255,0,0));
        p.drawLine(QLine{
            QPoint{ rc.left() - (int)_offset.x(), op.y() },
            QPoint{ rc.right() - (int)_offset.x(), op.y() }
        });
    }
    // 垂直坐标线
    {
        p.setPen(qRgb(0,255,0));
        p.drawLine(QLine{
            QPoint{ op.x(), rc.top() - (int)_offset.y() },
            QPoint{ op.x(), rc.bottom() - (int)_offset.y() }
        });
    }

    draw(p,false);

    p.translate(-_offset);
    p.scale(_scale,_scale);
    p.translate((_offset + op) / _scale);

    draw(p,true);
}

void Canvas::mousePressEvent(QMouseEvent *e)
{
    _start = e->position() - _offset;
    _drag = e->buttons() & Qt::LeftButton;
    if(_drag) setCursor(Qt::ClosedHandCursor);
}

void Canvas::mouseReleaseEvent(QMouseEvent *e)
{
    _drag = false;
    setCursor(Qt::OpenHandCursor);
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    if(!_drag) return;
    auto rc = rect();
    rc = rc.adjusted(_padding,_padding,-_padding,-_padding);

    const auto lw = rc.width() * _rx;
    const auto lh = rc.height() * _ry ;
    const auto rw = rc.width() * (1 - _rx);
    const auto rh = rc.height() * (1 - _ry);

    _offset = e->pos() - _start;

    if(_offset.x() < -lw)
        _offset.setX(-lw);
    if(_offset.x() > rw)
        _offset.setX(rw);

    if(_offset.y() < -lh)
        _offset.setY(-lh);
    if(_offset.y() > rh)
        _offset.setY(rh);

    repaint();
}

void Canvas::wheelEvent(QWheelEvent *e)
{
    auto delta = (e->angleDelta().y() / 45) * 0.04;
    setScale(_scale + delta);
    emit scale(_scale);
}

void Canvas::setPadding(int padding)
{
    _padding = padding;
    repaint();
}

int Canvas::padding() const
{
    return _padding;
}

void Canvas::setOrigin(float rx, float ry)
{
    _rx = std::max(0.0f,std::min(1.0f,rx));
    _ry = std::max(0.0f,std::min(1.0f,ry));
    repaint();
}

QPointF Canvas::origin() const
{
    return { _rx,_ry };
}

QPoint Canvas::originPos() const
{
    auto rc = rect();
    auto pt = rc.topLeft();
    pt.rx() += rc.width() * _rx;
    pt.ry() += rc.height() * _ry;
    rc.adjust(_padding,_padding,-_padding,-_padding);

    if(pt.x() < rc.left())
        pt.rx() = rc.left();
    if(pt.x() > rc.right())
        pt.rx() = rc.right();

    if(pt.y() < rc.top())
        pt.ry() = rc.top();
    if(pt.y() > rc.bottom())
        pt.ry() = rc.bottom();

    return pt;
}

void Canvas::setBackground(QColor color)
{
    _background = color;
    repaint();
}

QColor Canvas::background() const
{
    return _background;
}

void Canvas::setScale(float scale)
{
    _scale = scale;
    if(_scale < 0.25)
        _scale = 0.25;
    if(_scale > 4)
        _scale = 4;
    repaint();
}
