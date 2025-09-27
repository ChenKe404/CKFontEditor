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
/// TextureDrawer

void TextureDrawer::setTextureData(const TextureData *data)
{
    _data = data;
}

void TextureDrawer::perchar(int x, int y, const Font::Char *chr, const Font::DataPtr &d) const
{
    auto c = (TextureData::Char*)chr;
    const auto& tex = _data->texs[c->page];

    auto img = tex->copy(c->x,c->y,c->width,c->height);
    for (int y = 0; y < img.height(); ++y)
    {
        QRgb* scanLine = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x)
        {
            auto& pix = scanLine[x];
            auto color = mix(toCKColor(pix),_mix,_multiply);
            pix = toQColor(color).rgba();
        }
    }
}


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
    connect(ui->chk_texture,&QCheckBox::checkStateChanged,this,[this](Qt::CheckState state){
        _use_texture = state == Qt::Checked;
        ui->btn_texture->setEnabled(_use_texture);
    });
    connect(ui->btn_texture,&QPushButton::clicked,this,[this,font](){
        DlgTexture dlg(font,this);
        if(dlg.exec() == QDialog::Accepted)
        {
            ui->widget->setTextureData(std::move(dlg.data()));
            ui->widget->useTextureData(ui->chk_texture->isChecked());
        }
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
    : Canvas(parent),
    _show_textbox(false),
    _area(-1,-1),
    _use_texture_data(false)
{
    _opts.align = Drawer::AL_LEFT | Drawer::AL_BOTTOM;
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    _opts.spacingX = -1;
    _opts.spacingY = 0;
    setOrigin(0.5,0.5);
}

void PreviewWidget::draw(QPainter& p, bool transformed)
{
    if(transformed)
    {
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
    else
    {
        const auto bkg = _background;
        auto op = originPos();
        // 文本外框
        if(_show_textbox)
        {
            auto box = _drawer.measure(_chrs,_area.width(),_area.height(),_opts);
            p.setPen({ 255-bkg.red(),255-bkg.green(),255-bkg.blue() });
            p.drawRect(QRectF{
                box.x * _scale + op.x(),
                box.y * _scale + op.y(),
                box.w * _scale,
                box.h * _scale
            });
        }

        // 文本域边框
        {
            auto x = (double)op.x();
            auto y = (double)op.y();
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
    }
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

void PreviewWidget::setTextureData(TextureData &&data)
{
    _data = std::forward<TextureData>(data);
}

void PreviewWidget::useTextureData(bool yes)
{
    _use_texture_data = yes;
    repaint();
}

void PreviewWidget::showTextBox(bool yes)
{
    _show_textbox = yes;
    repaint();
}
