#include "charset_selector.h"
#include "ui_charset_selector.h"

#include <QFontDatabase>
#include <QPaintEvent>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CharsetCanvas
///

CharsetCanvas::CharsetCanvas(QWidget* parent)
    : QWidget(parent), _backgrid(128,128), _tt(nullptr)
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
    _cache.clear();
    updateCache(_font_height,true);
    update();
}

void CharsetCanvas::selectRange(uint32_t begin, uint32_t end)
{
    if(begin < _begin)
        return;
    _selected.clear();
    end = std::min(begin + 256, end);
    uint32_t count = 0;
    for(auto ch=begin;ch<end;++ch) {
        if(!_tt->has(ch))
            continue;
        _selected.insert(ch);
        ++count;
    }
    if(count == _count)
        emit selectedAll();
    else if(count == 0)
        emit unselectedAll();
    update();
}

void CharsetCanvas::selectCharset(const std::vector<uint32_t> &codepoints)
{
    _selected.clear();
    for(auto& ch : codepoints) {
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

void CharsetCanvas::setFont(const font::TrueType *tt)
{
    _tt = tt;
    _cache.clear();
    updateCache(_font_height, true);
    update();
}

void CharsetCanvas::paintEvent(QPaintEvent *event)
{
    static QPen pen_black(QColor{0,0,0});
    static QPen pen_white(QColor{220,220,220});

    QPainter p(this);
    if(!p.isActive())
        return;
    p.setRenderHint(p.Antialiasing, false);

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
    double w = cs.width(), h = cs.height();
    auto maxW = w * 0.8, maxH = h * 0.7;
    if(maxW < 4 || maxH < 4)
        return;
    updateCache(maxH);

    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QString str;
    p.setPen(pen_black);

    double ratio = maxH / _bound_h;
    if(_bound_w * ratio > maxW)
        ratio = maxW / _bound_w;

    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _begin + r * 16 + c;

            QRectF rc(c*w, r*h, w, h );
            if(ch > _end) { // 字符不在范围内
                p.fillRect(rc,QColor(50,50,50));
                continue;
            }
            auto pix = get(ch);
            if(!pix) continue;

            if(_selected.count(ch)) // 已选
                p.fillRect(rc,QColor(200,200,200));
            else
                p.fillRect(rc,QColor(90,90,90));

            double w1=pix->width()*ratio, h1=pix->height()*ratio;
            auto x = rc.x() + (w - w1) * 0.5;
            auto y = rc.y() + (h - h1) * 0.5;
            p.drawPixmap(x,y, w1,h1, *pix);
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
    if(event->button() == Qt::LeftButton)
        toggle(event->pos(), false);
}

void CharsetCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
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

    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _begin + r * 16 + c;
            if(!_tt->has(ch))
                continue;

            QRectF rc(c*w, r*h, w, h );
            if(!point_in_rect(rc,pos))
                continue;

            const auto is_selected = _selected.count(ch) > 0;
            if(drag) {
                if(_drag_select) {
                    _selected.insert(ch);
                    if(!is_selected) emit selected(ch);
                } else {
                    _selected.erase(ch);
                    if(is_selected) emit unselected(ch);
                }
            } else {
                if(is_selected) {
                    _selected.erase(ch);
                    emit unselected(ch);
                    _drag_select = false;
                } else {
                    _selected.insert(ch);
                    emit selected(ch);
                    _drag_select = true;
                }
            }
            if(_selected.empty())
                emit unselectedAll();
            else if(_selected.size() == _count)
                emit selectedAll();
            update();
            return;
        }
    }
}

void CharsetCanvas::updateCache(int fontHeight, bool force)
{
    if(fontHeight < 4)
        return;
    if(!force && _font_height > 0 && (fontHeight < _font_height * 1.3 && fontHeight > _font_height * 0.75))
        return;
    _font_height = fontHeight;
    _font_scale = _tt->scaleForHeight(_font_height);
    _bound_w = 0; _bound_h = 0;
    _count = 0;
    for(auto ch=_begin;ch<_end;++ch) {
        if(!_tt->has(ch))
            continue;
        ++_count;
        int x,y,w,h;
        _tt->getBitmapBox(ch,_font_scale,x,y,w,h);
        if(w > _bound_w)
            _bound_w = w;
        if(h > _bound_h)
            _bound_h = h;
    }

    for(auto& it : _cache) {
        makePixmap(it.second,it.first);
    }
}

void CharsetCanvas::makePixmap(QPixmap &out, uint32_t codepoint)
{
    int x,y,w,h;
    _tt->getBitmapBox(codepoint,_font_scale,x,y,w,h);
    std::vector<uint8_t> buf(w*h);
    _tt->makeBitmap(codepoint, _font_scale, buf.data(), w, h, w,true);
    QImage img(w,h,QImage::Format_ARGB32);

    auto to_argb = [&buf,&img,w,h](){
        for(int r=0;r<h;++r) {
            QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(r));
            for(int c=0;c<w;++c) {
                auto alpha = *(buf.data() + r * w + c);
                line[c] = alpha < 1 ? 0 : qRgba(10,10,10,alpha);
            }
        }
    };

    to_argb();
    out = QPixmap::fromImage(img);
}

const QPixmap* CharsetCanvas::get(uint32_t codepoint)
{
    if(!_tt || !_tt->has(codepoint))
        return nullptr;
    auto iter = _cache.find(codepoint);
    if(iter != _cache.end())
        return &iter->second;
    auto& pix = _cache[codepoint];
    makePixmap(pix,codepoint);
    return &pix;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CharsetSelector
///

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
    auto act = new QAction(tr("从文件选择字符"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onOpen); _menu.addAction(act);
    act = new QAction(tr("从文件追加字符"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onAppend); _menu.addAction(act);
    act = new QAction(tr("清除选择"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onClear); _menu.addAction(act);

    connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& pos){
        _menu.popup(mapToGlobal(pos));
    });
    connect(ui->list_block,&QListWidget::currentRowChanged,this,[this](int row){
        auto item = ui->list_block->currentItem();
        if(!item) return;
        bool ok = false;
        auto page = (const CharsetBlocks::Page*)item->data(Qt::UserRole).toLongLong(&ok);
        if(!ok) return;
        ui->canvas->setRange(page->begin,page->end);
    });
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
            item->setSelected(true);
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

void CharsetSelector::setCanvasFont(const font::TrueType* font)
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

void CharsetSelector::onOpen()
{
    QFileDialog dlg(this,tr("从文本文件选择字符"),"","UTF-8 Text File (*.txt) ;; All File (*.*)");
    if(dlg.exec() != QDialog::Accepted)
        return;

}

void CharsetSelector::onAppend()
{
    QFileDialog dlg(this,tr("从文本文件追加字符"),"","UTF-8 Text File (*.txt) ;; All File (*.*)");
    if(dlg.exec() != QDialog::Accepted)
        return;
}

void CharsetSelector::onClear()
{

}
