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
        const auto checked = state == Qt::Checked;
        ui->btn_texture->setEnabled(checked);
        ui->widget->useTextureData(checked);
    });
    connect(ui->btn_texture,&QPushButton::clicked,this,[this,font](){
        DlgTexture dlg(font,this);
        if(dlg.exec() == QDialog::Accepted)
            ui->widget->setTextureData(std::move(dlg.data()));
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
    case 0: align |= FontDrawer::AL_LEFT;
        break;
    case 1: align |= FontDrawer::AL_HCENTER;
        break;
    case 2: align |= FontDrawer::AL_RIGHT;
        break;
    }

    i = ui->cob_align_v->currentIndex();
    switch (i) {
    case 0: align |= FontDrawer::AL_TOP;
        break;
    case 1: align |= FontDrawer::AL_VCENTER;
        break;
    case 2: align |= FontDrawer::AL_BOTTOM;
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
    _opts.align = FontDrawer::AL_LEFT | FontDrawer::AL_BOTTOM;
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    _opts.spacingX = -1;
    _opts.spacingY = 0;
    setOrigin(0.5,0.5);
    _drawer.setFontTexture(&_data.ft);
}

void PreviewWidget::draw(QPainter& p, bool transformed)
{
    if(transformed)
    {
        _drawer.setPainter(&p);
        auto box = _drawer.draw(0,0,_area.width(),_area.height(),_opts);
        // 文本外框
        if(_show_textbox)
        {
            const auto bkg = _background;
            p.setPen({ 255-bkg.red(),255-bkg.green(),255-bkg.blue() });
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
        auto op = originPos();
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
    _drawer.setText(text);
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
    _drawer.setFontTexture(&_data.ft);
    repaint();
}

void PreviewWidget::useTextureData(bool yes)
{
    _drawer.useTextureDrawer(yes);
    repaint();
}

void PreviewWidget::showTextBox(bool yes)
{
    _show_textbox = yes;
    repaint();
}
