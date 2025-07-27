/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_preview.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_preview.h"
#include "ui_dlg_preview.h"

#include <QWheelEvent>

//////////////////////////////////
/// Drawer

void Drawer::perchar(int x,int y, const Font::Char*, const Font::DataPtr& d) const
{
    const auto bit32 = _font->header().flag & Font::FL_BIT32;
    auto trans = _font->header().transparent;

    std::vector<color> buf;
    buf.reserve(d.w() * d.h());
    for(int oy=0; oy<d.h(); ++oy)
    {
        for(int ox=0; ox<d.w(); ++ox)
        {
            auto color = d.get(ox,oy);
            if(!bit32 && color == trans)
                buf.push_back(0);
            else
                buf.push_back(mix(color,_mix,_multiply));
        }
    }
    _p->drawImage(QPoint{x,y}, QImage((uint8_t*)buf.data(),d.w(),d.h(),QImage::Format_ARGB32));
}

void Drawer::setPainter(QPainter *p)
{ _p = p; }

void Drawer::setMultiply(bool yes)
{ _multiply = yes; }

/////////////////////////////////////////
/// DlgPreview

DlgPreview::DlgPreview(Font *font) :
    QDialog(),
    ui(new Ui::DlgPreview)
{
    ui->setupUi(this);
    setMouseTracking(true);
    ui->widget->setFont(font);
    ui->btn_background->setColor(qRgb(0,0,0));
    ui->edt_text->appendPlainText("Line1: Simple Text\nLine2: Hello world");
    ui->spb_area_w->setValue(-1);
    ui->spb_area_h->setValue(-1);
    ui->spb_scale->setValue(1);
    ui->widget->setText(ui->edt_text->toPlainText());
    ui->chk_breakword->setChecked(false);
    ui->widget->setBreakWord(false);
    ui->spb_spacing_h->setValue(-1);
    ui->spb_spacing_v->setValue(0);

    ui->widget->setMultiply(ui->chk_multiply->isChecked());
    connect(ui->edt_text,&QPlainTextEdit::textChanged,this,[this](){
        ui->widget->setText(ui->edt_text->toPlainText());
    });
    connect(ui->btn_background,&ColorButton::colorChanged,ui->widget,&PreviewWidget::setBackground);
    connect(ui->btn_mix_color,&ColorButton::colorChanged,ui->widget,&PreviewWidget::setMixColor);
    connect(ui->cob_align_h,&QComboBox::currentIndexChanged,this,&DlgPreview::onAlign);
    connect(ui->cob_align_v,&QComboBox::currentIndexChanged,this,&DlgPreview::onAlign);
    connect(ui->spb_area_w,&QSpinBox::valueChanged,this,&DlgPreview::onArea);
    connect(ui->spb_area_h,&QSpinBox::valueChanged,this,&DlgPreview::onArea);
    connect(ui->spb_spacing_h,&QSpinBox::valueChanged,this,&DlgPreview::onSpacing);
    connect(ui->spb_spacing_v,&QSpinBox::valueChanged,this,&DlgPreview::onSpacing);
    connect(ui->spb_scale,&QDoubleSpinBox::valueChanged,ui->widget,&PreviewWidget::setScale);
    connect(ui->widget,&PreviewWidget::scale,ui->spb_scale,&QDoubleSpinBox::setValue);
    connect(ui->chk_textbox,&QCheckBox::clicked,ui->widget,&PreviewWidget::showTextBox);
    connect(ui->chk_breakword,&QCheckBox::clicked,ui->widget,&PreviewWidget::setBreakWord);
    connect(ui->chk_multiply,&QCheckBox::clicked,ui->widget,&PreviewWidget::setMultiply);
    connect(ui->sld_alpha,&QSlider::valueChanged,this,[this](int value){
        ui->widget->setMixAlpha(value);
        ui->lab_alpha->setText(QString::number(value));
    });
    ui->chk_textbox->setChecked(true);
    ui->widget->showTextBox(true);
}

DlgPreview::~DlgPreview()
{
    delete ui;
}

void DlgPreview::onSpacing()
{
    ui->widget->setSpacing(ui->spb_spacing_h->value(),ui->spb_spacing_v->value());
}

void DlgPreview::onArea()
{
    ui->widget->setArea({ ui->spb_area_w->value(),ui->spb_area_h->value() });
}

void DlgPreview::onAlign()
{
    uint8_t align = 0;
    auto i = ui->cob_align_h->currentIndex();
    switch (i) {
    case 0: align |= Drawer::AL_LEFT;
        break;
    case 1: align |= Drawer::AL_HCENTER;
        break;
    case 2: align |= Drawer::AL_RIGHT;
        break;
    }

    i = ui->cob_align_v->currentIndex();
    switch (i) {
    case 0: align |= Drawer::AL_TOP;
        break;
    case 1: align |= Drawer::AL_VCENTER;
        break;
    case 2: align |= Drawer::AL_BOTTOM;
        break;
    }
    ui->widget->setAlign(align);
}

/////////////////////////////////////////
/// PreviewWidget

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent),
    _show_textbox(false),
    _scale(1),_offset(0,0),
    _area(-1,-1),
    _drag(false)
{
    _opts.align = Drawer::AL_LEFT | Drawer::AL_BOTTOM;
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    _opts.spacingX = -1;
    _opts.spacingY = 0;
}

void PreviewWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    const auto rc = rect();
    QPainter p(this);
    p.setRenderHints(p.Antialiasing);
    const auto bkg = _background;
    p.fillRect(rc,bkg);
    p.translate(_offset);
    auto rc1 = rc.translated(rc.width() / 2,rc.height() / 2);

    // 水平原点线
    {
        p.setPen(qRgb(255,0,0));
        p.drawLine(QLine{
            QPoint{ rc.left() - (int)_offset.x(), rc1.top() },
            QPoint{ rc.right() - (int)_offset.x(), rc1.top() }
        });
    }
    // 垂直坐标线
    {
        p.setPen(qRgb(0,255,0));
        p.drawLine(QLine{
            QPoint{ rc1.left(), rc.top() - (int)_offset.y() },
            QPoint{ rc1.left(), rc.bottom() - (int)_offset.y() }
        });
    }

    // 文本外框
    if(_show_textbox)
    {
        auto box = _drawer.measure(_chrs,_area.width(),_area.height(),_opts);
        p.setPen({ 255-bkg.red(),255-bkg.green(),255-bkg.blue() });
        p.drawRect(QRectF{
            box.x * _scale + rc1.left(),
            box.y * _scale + rc1.top(),
            box.w * _scale,
            box.h * _scale
        });
    }

    // 文本域边框
    {
        auto x = (double)rc1.left();
        auto y = (double)rc1.top();
        auto w = _area.width() * _scale;
        auto h = _area.height() * _scale;
        p.setPen(qRgb(255,0,255));
        if(_area.width() > 0)
        {
            if(_area.height() > 0)
                p.drawRect(QRectF{ x,y,w,h });
            else
                p.drawLine(QLineF{ QPointF{ x,y }, QPointF{ x + w,y }, });
        }
        else if(_area.height() > 0)
        {
            p.drawLine(QLineF{ QPointF{ x,y }, QPointF{ x,y + h } });
        }
    }

    p.translate(-_offset);
    p.scale(_scale,_scale);
    p.translate((_offset + rc1.topLeft()) / _scale);

    if(!_chrs.empty())
    {
        _drawer.setPainter(&p);
        auto box = _drawer.draw(_chrs,0,0,_area.width(),_area.height(),_opts);
        p.drawRect(QRect{
            box.x,
            box.y,
            box.w,
            box.h
        });
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent *e)
{
    _start = e->position() - _offset;
    _drag = e->buttons() & Qt::LeftButton;
    if(_drag) setCursor(Qt::ClosedHandCursor);
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent *e)
{
    _drag = false;
    setCursor(Qt::OpenHandCursor);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(!_drag) return;
    const auto& rc = rect();
    const auto hw = rc.width() / 2 - 5;
    const auto hh = rc.height() / 2 - 5;
    _offset = e->pos() - _start;

    if(_offset.x() < -hw)
        _offset.setX(-hw);
    if(_offset.x() > hw)
        _offset.setX(hw);

    if(_offset.y() < -hh)
        _offset.setY(-hh);
    if(_offset.y() > hh)
        _offset.setY(hh);

    repaint();
}

void PreviewWidget::wheelEvent(QWheelEvent *e)
{
    auto delta = (e->angleDelta().y() / 45) * 0.04;
    setScale(_scale + delta);
    emit scale(_scale);
}

void PreviewWidget::setFont(Font *font)
{
    _font = font;
    _drawer.setFont(_font);
}

void PreviewWidget::setText(const QString &text)
{
    if(!_font) return;
    auto temp = text;
    temp.replace("\\n","\n");
    temp.replace("\\t","\t");
    _chrs = _font->css(temp.toStdU32String());
    repaint();
}

void PreviewWidget::setBackground(QColor color)
{
    _background = color;
    repaint();
}

void PreviewWidget::setMixAlpha(uint8_t alpha)
{
    _mix_alpha = alpha;
    auto clr = _drawer.mixColor();
    _drawer.setMixColor(argb(_mix_alpha,cr(clr),cg(clr),cb(clr)));
    repaint();
}

void PreviewWidget::setMixColor(QColor color)
{
    _drawer.setMixColor(argb(_mix_alpha,color.red(),color.green(),color.blue()));
    repaint();
}

void PreviewWidget::setAlign(uint8_t align)
{
    _opts.align = align;
    repaint();
}

void PreviewWidget::setScale(float scale)
{
    _scale = scale;
    if(_scale < 0.25)
        _scale = 0.25;
    if(_scale > 4)
        _scale = 4;
    repaint();
}

void PreviewWidget::setArea(QSize area)
{
    _area = area;
    repaint();
}

void PreviewWidget::setSpacing(int x, int y)
{
    _opts.spacingX = x;
    _opts.spacingY = y;
    repaint();
}

void PreviewWidget::setBreakWord(bool yes)
{
    _opts.breakWord = yes;
    repaint();
}

void PreviewWidget::setMultiply(bool yes)
{
    _drawer.setMultiply(yes);
    repaint();
}

void PreviewWidget::showTextBox(bool yes)
{
    _show_textbox = yes;
    repaint();
}
