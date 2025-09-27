#ifndef DLG_TEXTURE_H
#define DLG_TEXTURE_H

#include <QDialog>
#include <QPointer>
#include <font_texture.h>
#include <component/canvas.h>

namespace Ui {
class DlgTexture;
}

struct TextureData {
    FontTexture ft;
    std::vector<QSharedPointer<QImage>> imgs;
};

class PreviewTextureWidget : public Canvas
{
    Q_OBJECT
public:
    PreviewTextureWidget(QWidget* parent);

    void draw(QPainter& p, bool transformed) override;
    void setImage(const QSharedPointer<QImage>& img);
private:
    QSharedPointer<QImage> _img;
};

class DlgTexture : public QDialog
{
    Q_OBJECT
    class Creator : public FontTextureCreator
    {
        using super = FontTextureCreator;
    public:
        using super::FontTextureCreator;
        void *newTexture() override;
        void perchar(const Font* fnt,const Char &, const Font::DataPtr &d, void *texture) const override;
    };
public:
    explicit DlgTexture(const Font* fnt, QWidget *parent = nullptr);
    ~DlgTexture();

    TextureData& data();
private:
    void onCreate();
private:
    Ui::DlgTexture *ui;
    const Font* _font;
    TextureData _data;
};

#endif // DLG_TEXTURE_H
