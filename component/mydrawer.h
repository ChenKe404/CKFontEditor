#ifndef MYDRAWER_H
#define MYDRAWER_H

#include <drawer.h>
#include <texture.h>

struct MyFontDrawer : public font::Drawer
{
    void perchar(int x,int y, const font::Char* chr, const font::DataPtr& d) const override;
    void setPainter(QPainter*);
    void setMultiply(bool yes);
protected:
    QPainter* _p;
    QPixmap _pix;
    bool _multiply = false;
};

struct MyTextureDrawer : public MyFontDrawer
{
    void perchar(int x,int y, const font::Char* chr, const font::DataPtr& d) const override;
    void setTextures(const font::Texture* data);
private:
    const font::Texture* _data = nullptr;
};

struct MyDrawer
{
    void setFont(const font::File*);
    void setTextures(const font::Texture*);

    void setText(const QString& str);

    void setPainter(QPainter* p);

    color mixColor() const;
    void setMixColor(color);
    void setMultiply(bool);

    font::Box draw(
        int x, int y, int w = -1,int h = -1,
        const font::Drawer::Options& opts = {}
        );

    font::Box measure(
        int w = -1, int h = -1,
        const font::Drawer::Options& opts = {}
        );

    void useTextureDrawer(bool yes);
private:
    void updateCharset();
private:
    QString _text;

    const font::File* _fnt;
    const font::Texture* _ft = nullptr;

    bool _use_texture_drawer = false;
    font::CharPtrArray _chrs;

    MyFontDrawer _font_drawer;
    MyTextureDrawer _texture_drawer;
};


#endif // MYDRAWER_H
