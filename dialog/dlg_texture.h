#ifndef DLG_TEXTURE_H
#define DLG_TEXTURE_H

#include <QDialog>
#include <QPointer>
#include <ckfont/texture.h>
#include <component/canvas.h>

namespace Ui {
class DlgTexture;
}

struct TextureData {
    font::Texture ft;
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
    class Creator : public font::TextureCreator
    {
        using super = font::TextureCreator;
    public:
        using super::TextureCreator;
        void *newTexture() override;
        void perchar(const font::File& fnt,const Char &, const font::DataPtr &d, void *texture) override;
    };
public:
    explicit DlgTexture(const font::File* fnt, QWidget *parent = nullptr);
    ~DlgTexture();

    TextureData& data();
private:
    void onCreate();
private:
    Ui::DlgTexture *ui;
    const font::File* _font;
    TextureData _data;
};

#endif // DLG_TEXTURE_H
