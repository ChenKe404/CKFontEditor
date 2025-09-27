#include "mydrawer.h"

//////////////////////////////////
/// MyFontDrawer

void MyFontDrawer::perchar(int x,int y, const Font::Char*, const Font::DataPtr& d) const
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

void MyFontDrawer::setPainter(QPainter *p)
{ _p = p; }

void MyFontDrawer::setMultiply(bool yes)
{ _multiply = yes; }

/////////////////////////////////////////
/// MyTextureDrawer

void MyTextureDrawer::perchar(int x, int y, const Font::Char *chr, const Font::DataPtr &d) const
{
    auto c = (FontTexture::Char*)chr;
    const auto& tex = (QImage*)_data->pages()[c->page];

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
    _p->drawImage(x,y,img);
}

void MyTextureDrawer::setFontTexture(const FontTexture *data)
{
    _data = data;
}

/////////////////////////////////////////
/// MyDrawer

void MyDrawer::setFont(const Font * fnt)
{
    _fnt = fnt;
    _font_drawer.setFont(fnt);
    _texture_drawer.setFont(fnt);
}

void MyDrawer::setFontTexture(const FontTexture * ft)
{
    _ft = ft;
    _texture_drawer.setFontTexture(ft);
    updateCharset();
}

void MyDrawer::setText(const QString &str)
{
    _text = str;
    updateCharset();
}

void MyDrawer::setPainter(QPainter *p)
{
    _font_drawer.setPainter(p);
    _texture_drawer.setPainter(p);
}

color MyDrawer::mixColor() const
{
    return _font_drawer.mixColor();
}

void MyDrawer::setMixColor(color clr)
{
    _font_drawer.setMixColor(clr);
    _texture_drawer.setMixColor(clr);
}

void MyDrawer::setMultiply(bool yes)
{
    _font_drawer.setMultiply(yes);
    _texture_drawer.setMultiply(yes);
}

FontDrawer::Box MyDrawer::draw(int x, int y, int w, int h, const FontDrawer::Options &opts)
{
    if(_use_texture_drawer)
        return _texture_drawer.draw(_chrs,x,y,w,h,opts);
    else
        return _font_drawer.draw(_chrs,x,y,w,h,opts);
}

FontDrawer::Box MyDrawer::measure(int w, int h, const FontDrawer::Options &opts)
{
    if(_use_texture_drawer)
        return _texture_drawer.measure(_chrs,w,h,opts);
    else
        return _font_drawer.measure(_chrs,w,h,opts);
}

void MyDrawer::useTextureDrawer(bool yes)
{
    _use_texture_drawer = yes;
    updateCharset();
}

void MyDrawer::updateCharset()
{
    if(!_use_texture_drawer)
        _chrs = _fnt->css(_text.toStdU32String());
    else
    {
        auto chrs = _ft->css(_text.toStdU32String());
        _chrs.clear();
        for(auto& it : chrs)
        {
            _chrs.push_back(it);
        }
    }
}

