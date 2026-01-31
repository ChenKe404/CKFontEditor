#ifndef CHARSET_BLOCKS_H
#define CHARSET_BLOCKS_H

#include <QThread>
#include <ckfont/truetype.h>

struct CharsetBlocks : public QObject {
    struct Block {
        uint32_t begin, end;
        QString name;
    };
    struct Page {
        uint32_t begin, end;
        const Block* block;
    };

    static const std::vector<Block>& get();
    static const std::vector<Page>& getPages();
};

class CharsetPager : public QThread {
    Q_OBJECT
public:
    bool dirty() const {
        return _dirty;
    }
    void setFont(const font::TrueType* tt);
signals:
    void progress(int step);
    void done(const std::vector<const CharsetBlocks::Page*>&);
protected:
    void run() override;
private:
    std::mutex _mtx;
    size_t _hash;
    bool _dirty = true;
    const font::TrueType* _tt;
};
#endif // CHARSET_BLOCKS_H
