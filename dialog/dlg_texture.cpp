#include "dlg_texture.h"
#include "ui_dlg_texture.h"

static QString fmt_lab_page;

DlgTexture::DlgTexture(const Font* fnt, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgTexture),
    _font(fnt)
{
    ui->setupUi(this);
    connect(ui->btn_create,&QPushButton::clicked,this,&DlgTexture::onCreate);
    connect(ui->w_preview,&Canvas::scale,ui->spb_scale,&QDoubleSpinBox::setValue);
    connect(ui->spb_scale,&QDoubleSpinBox::valueChanged,ui->w_preview,&Canvas::setScale);
    connect(ui->btn_background,&ColorButton::colorChanged,ui->w_preview,&Canvas::setBackground);
    connect(ui->spb_page,&QSpinBox::valueChanged,[this](int value){
        if(_data.imgs.empty())
            ui->spb_page->setValue(0);
        else if(value < 1)
            ui->spb_page->setValue(1);
        else if(value > _data.imgs.size())
            ui->spb_page->setValue(_data.imgs.size());
        else
        {
            ui->lab_page->setText(fmt_lab_page.arg(1).arg(_data.imgs.size()));
            ui->w_preview->setImage(_data.imgs[value-1]);
        }
    });
    connect(ui->cob_estimate,&QComboBox::activated,this,[this](int idx){
        if(idx == 0)
            return;
        auto w = ck::FontTextureCreator::estimate(*_font,ui->spb_spacing->value());
        if(idx == 2)
        {
            if(w < 128)
                w= 128;
            else if(w < 256)
                w = 256;
            else if(w < 512)
                w = 512;
            else
                w = ck::FontTextureCreator::estimate(*_font,ui->spb_spacing->value(),256,2048);
        }
        ui->spb_width->setValue(w);
        ui->spb_height->setValue(w);
    });
    fmt_lab_page = ui->lab_page->text();
    ui->lab_page->setText(fmt_lab_page.arg(0).arg(0));
}

DlgTexture::~DlgTexture()
{
    delete ui;
}

void DlgTexture::onCreate()
{
    Creator c(
        ui->spb_width->value(),
        ui->spb_height->value(),
        ui->spb_spacing->value()
        );
    auto& ft = _data.ft;
    c.start(*_font,ft);
    if(ft.chrs().empty())
    {
        ui->w_preview->setImage(nullptr);
        ui->lab_page->setText(fmt_lab_page.arg(0).arg(0));
        ui->spb_page->setValue(0);
    }
    else
    {
        _data.imgs.clear();
        _data.imgs.reserve(ft.pages().size());
        for(auto& it : ft.pages())
        {
            _data.imgs.emplace_back(QSharedPointer<QImage>((QImage*)it));
        }
        ui->w_preview->setImage(_data.imgs.front());
        ui->lab_page->setText(fmt_lab_page.arg(1).arg(_data.imgs.size()));
        ui->spb_page->setValue(1);
    }
}

TextureData &DlgTexture::data()
{
    return _data;
}

void *DlgTexture::Creator::newTexture()
{
    auto ret = new QImage(_width,_height, QImage::Format_ARGB32);
    ret->fill(0);
    return ret;
}

void DlgTexture::Creator::perchar(const Font& fnt,const Char & chr, const Font::DataPtr &d, void *texture)
{
    auto img = (QImage*)texture;

    const auto trans = fnt.header().transparent;
    const auto bit32 = fnt.header().flag & Font::FL_BIT32;

    const auto w = chr.width;
    const auto h = chr.height;

    for(int y=0; y<h; ++y)
    {
        for(int x=0; x<w; ++x)
        {
            auto color = fnt.getColor(chr,x,y);
            if(!bit32 && color == trans);
            else
                img->setPixelColor(chr.x + x,chr.y + y,toQColor(color));
        }
    }
}

//////////////////////////////////////
/// PreviewTextureWidget
PreviewTextureWidget::PreviewTextureWidget(QWidget *parent)
    : Canvas(parent), _img(nullptr)
{
    setOrigin(0,0);
}

void PreviewTextureWidget::draw(QPainter &p, bool transformed)
{
    if(!transformed || !_img)
        return;
    p.setPen(QColor(255,0,0));
    p.drawRect(_img->rect());
    p.drawImage(0,0,*_img);
}

void PreviewTextureWidget::setImage(const QSharedPointer<QImage>& img)
{
    _img = img;
    repaint();
}
