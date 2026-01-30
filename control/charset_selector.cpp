#include "charset_selector.h"
#include "ui_charset_selector.h"

#include <QFontDatabase>
#include <QPaintEvent>

CharsetCanvas::CharsetCanvas(QWidget* parent)
    : QWidget(parent), _backgrid(128,128)
{
    QPainter p(&_backgrid);
    p.fillRect(QRect{ 0,0,128,128 },QColor{0,0,0});
    p.setPen(QPen{ QColor(100,100,100), 1 });
    for(int i=1;i<16;++i) {
        p.drawLine(i*8,0, 0,i*8);
    }
    for(int i=0;i<16;++i) {
        p.drawLine(i*8,128, 128,i*8);
    }
}

void CharsetCanvas::setRange(uint32_t begin, uint32_t end)
{
    _begin = begin;
    _end = std::min(end, _begin + 256);
    auto iter = _selected.find(_begin);
    if(iter != _selected.end())
        _selected.erase(_selected.begin(),iter);
    iter = _selected.find(_end);
    if(iter != _selected.end())
        _selected.erase(std::next(iter),_selected.end());
    update();
}

void CharsetCanvas::selectRange(uint32_t begin, uint32_t end)
{
    if(begin < _begin)
        return;
    _selected.clear();
    end = std::min(begin + 256, end);
    for(auto ch=begin;ch<=end;++ch) {
        _selected.insert(ch);
    }
    update();
}

void CharsetCanvas::selectCharset(const std::vector<uint32_t> &codes)
{
    _selected.clear();
    for(auto& ch : codes) {
        if(ch < _begin || ch >= _begin + 256)
            continue;
        _selected.insert(ch);
    }
    update();
}

const std::set<uint32_t> &CharsetCanvas::selected() const
{
    return _selected;
}

uint32_t CharsetCanvas::available() const
{
    uint32_t total = 0;
    auto fm = fontMetrics();
    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _begin + r * 16 + c;
            if(!fm.inFontUcs4(ch))
                continue;
        }
    }
    return total;
}

void CharsetCanvas::paintEvent(QPaintEvent *event)
{
    static QPen pen_black(QColor{0,0,0});
    static QPen pen_white(QColor{240,240,240});

    QPainter p(this);
    const auto& rc = event->rect();
    // 绘制背景网格
    {
        auto cw = ceil(rc.width() / 128.0), ch = ceil(rc.height() / 128.0);
        for(int r=0;r<ch;++r) {
            for(int c=0;c<cw;++c) {
                p.drawPixmap(c * 128,r * 128,_backgrid);
            }
        }
    }
    auto cs = cellSize();
    auto w = cs.width(), h = cs.height();
    auto fontsize = std::min(w,h) * 0.5;
    if(fontsize < 4)
        return;

    auto fnt = p.font();
    fnt.setPixelSize((int)fontsize);
    p.setFont(fnt);

    const auto fm = p.fontMetrics();
    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QString str;
    p.setPen(pen_black);

    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _begin + r * 16 + c;

            QRectF rc(c*w, r*h, w, h );
            if(ch > _end) { // 字符不在范围内
                p.fillRect(rc,QColor(50,50,50));
                continue;
            }
            if(!fm.inFontUcs4(ch))
                continue;

            if(_selected.count(ch)) // 已选
                p.fillRect(rc,QColor(200,200,200));
            else
                p.fillRect(rc,QColor(80,80,80));
            str.clear();
            str.append(QChar::fromUcs4(ch));
            p.drawText(rc,str,opt);
        }
    }

    p.setPen(pen_white);
    for(int r=1; r<16; ++r) {
        p.drawLine(0,r*h, rc.width(),r*h);
    }
    for(int c=1; c<16; ++c) {
        p.drawLine(c*w,0, c*w,rc.height());
    }
}

void CharsetCanvas::mousePressEvent(QMouseEvent *event)
{
    toggle(event->pos(), false);
}

void CharsetCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    _darged.clear();
}

void CharsetCanvas::mouseMoveEvent(QMouseEvent *event)
{
    toggle(event->pos(), true);
}

QSizeF CharsetCanvas::cellSize() const
{
    return { rect().width() / 16.0, rect().height() / 16.0};
}

static bool point_in_rect(const QRectF& rc, const QPoint& pos) {
    return pos.x() > rc.left() && pos.x() < rc.right() &&
           pos.y() > rc.top() && pos.y() < rc.bottom();
}

void CharsetCanvas::toggle(const QPoint &pos,bool drag)
{
    auto cs = cellSize();
    auto w = cs.width(), h = cs.height();
    const auto fm = fontMetrics();

    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _begin + r * 16 + c;
            if(!fm.inFontUcs4(ch))
                continue;

            QRectF rc(c*w, r*h, w, h );
            if(!point_in_rect(rc,pos))
                continue;

            if(drag && _darged.count(ch) > 0)
                return;
            _darged.insert(ch);

            auto iter = _selected.find(ch);
            if(iter == _selected.end())
                _selected.insert(ch);
            else
                _selected.erase(ch);
            update();
            return;
        }
    }
}

CharsetSelector::CharsetSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CharsetSelector)
{
    ui->setupUi(this);
    ui->pgb->setVisible(false);
    ui->pgb->setMinimum(0);
    ui->pgb->setMaximum(100);
    ui->list_block->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->list_block->setUniformItemSizes(true);
    ui->list_block->setViewMode(QListView::ListMode);
    ui->list_block->setWrapping(false);
    ui->list_block->setLayoutMode(QListView::Batched);
    ui->list_block->setBatchSize(50);  // 分批处理项

    connect(&_pager, &CharsetPager::progress, ui->pgb, &QProgressBar::setValue);
    connect(&_pager, &CharsetPager::done, this, [this](const std::vector<const CharsetBlocks::Page*>& pages){
        ui->pgb->setVisible(false);
        QString fmt("%1 ");
        for(auto & it : pages) {
            auto name = fmt.arg(it->begin,6,16,'0').toUpper() + it->block->name;
            auto item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, (uintptr_t)it);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsUserTristate);
            item->setCheckState(Qt::Unchecked);
            ui->list_block->addItem(item);
        }
        auto item = ui->list_block->item(0);
        if(item) {
            auto page = (const CharsetBlocks::Page*)item->data(Qt::UserRole).toLongLong();
            ui->canvas->setRange(page->begin,page->end);
        }
    });
}

CharsetSelector::~CharsetSelector()
{
    _pager.terminate();
    delete ui;
}

void CharsetSelector::setCanvasFont(const QFont &font)
{
    _pager.setFont(font);
    ui->canvas->setFont(font);
    updateBlockList();
}

struct ItemData : public QObject
{
    ItemData(QObject* parent = nullptr)
        : QObject()
    {}

    uint32_t begin, end;
    const CharsetBlocks::Block* block;
};

void CharsetSelector::updateBlockList()
{
    if(!_pager.dirty())
        return;
    ui->pgb->setValue(0);
    ui->pgb->setVisible(true);
    ui->list_block->clear();
    _pager.start();
}
