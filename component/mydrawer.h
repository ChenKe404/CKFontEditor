#ifndef MYDRAWER_H
#define MYDRAWER_H

#include <drawer.h>
#include <font_texture.h>

struct MyFontDrawer : public FontDrawer
{
    void perchar(int x,int y, const Font::Char* chr, const Font::DataPtr& d) const override;
    void setPainter(QPainter*);
    void setMultiply(bool yes);
protected:
    QPainter* _p;
    QPixmap _pix;
    bool _multiply = false;
};

struct MyTextureDrawer : public MyFontDrawer
{
    void perchar(int x,int y, const Font::Char* chr, const Font::DataPtr& d) const override;
    void setFontTexture(const FontTexture* data);
private:
    const FontTexture* _data = nullptr;
};

struct MyDrawer
{
    void setFont(const Font*);
    void setFontTexture(const FontTexture*);

    void setText(const QString& str);

    void setPainter(QPainter* p);

    color mixColor() const;
    void setMixColor(color);
    void setMultiply(bool);

    FontDrawer::Box draw(
        int x, int y, int w = -1,int h = -1,
        const FontDrawer::Options& opts = {}
        );

    FontDrawer::Box measure(
        int w = -1, int h = -1,
        const FontDrawer::Options& opts = {}
        );

    void useTextureDrawer(bool yes);
private:
    void updateCharset();
private:
    QString _text;

    const Font* _fnt;
    const FontTexture* _ft = nullptr;

    bool _use_texture_drawer = false;
    Font::CharPtrList _chrs;

    MyFontDrawer _font_drawer;
    MyTextureDrawer _texture_drawer;
};


#endif // MYDRAWER_H
