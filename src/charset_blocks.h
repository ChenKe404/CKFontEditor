#ifndef CHARSET_BLOCKS_H
#define CHARSET_BLOCKS_H

#include <QThread>
#include <ckfont/truetype.h>

struct CharsetBlocks : public QObject {
    struct Block {
        uint32_t first, last;
        QString name;

        bool operator<(const Block& o) const { return first < o.first; }
    };
    struct Page {
        uint32_t first, last;
        const Block* block;

        bool operator<(const Page& o) const { return first < o.first; }
    };

    static const std::vector<Block>& get();
    static const std::vector<Page>& getPages();
    static const Block* find_block(uint32_t codepoint);
    static const Page* find_page(uint32_t codepoint);
};

class CharsetPager : public QThread {
    Q_OBJECT
public:
    bool dirty() const {
        return _dirty;
    }
    void setFont(const font::TrueType* tt);
    // 总有效字符数
    uint32_t total() const { return _total; }
signals:
    void progress(int step);
    void done(const std::vector<const CharsetBlocks::Page*>&);
protected:
    void run() override;
private:
    std::mutex _mtx;
    size_t _hash;
    uint32_t _total = 0;
    bool _dirty = true;
    const font::TrueType* _tt;
};
#endif // CHARSET_BLOCKS_H
