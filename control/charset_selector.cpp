#include "charset_selector.h"
#include "ui_charset_selector.h"

#include <QFontDatabase>
#include <QPaintEvent>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CharsetCanvas
///

CharsetCanvas::CharsetCanvas(QWidget* parent)
    : QWidget(parent), _backgrid(128,128), _page(nullptr), _tt(nullptr)
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

void CharsetCanvas::setPage(const Page* page)
{
    if(!page)
        return;
    _page = page;
    _cache.clear();
    updatePageState();
    updateCache(_font_height,true);
    update();
}

void CharsetCanvas::selectPage(const Page *page)
{
    selectRange(page->first, page->last);
}

void CharsetCanvas::unselectPage(const Page *page)
{
    unselectRange(page->first, page->last);
}

void CharsetCanvas::selectRange(uint32_t first, uint32_t last)
{
    if(!_tt)
        return;
    for(auto ch=first;ch<=last;++ch) {
        if(!_tt->has(ch))
            continue;
        _charset.insert(ch);
    }
    updatePageState();
    update();
}

void CharsetCanvas::unselectRange(uint32_t first, uint32_t last)
{
    if(!_tt)
        return;
    for(auto ch=first;ch<=last;++ch) {
        _charset.erase(ch);
    }
    updatePageState();
    update();
}

void CharsetCanvas::setCharset(const Charset &charset)
{
    _charset = charset;
    update();
}

void CharsetCanvas::clear()
{
    _charset.clear();
    update();
}

const Charset &CharsetCanvas::charset() const
{
    return _charset;
}

void CharsetCanvas::setFont(const font::TrueType *tt)
{
    _tt = tt;
    _cache.clear();
    updateCache(_font_height, true);
    update();
}

void CharsetCanvas::updatePageState(const Page* page)
{
    if(!page) page = _page;
    if(!page) return;
    auto old = _page_state;
    _page_state = -1;
    for(auto ch=page->first; ch<=page->last; ++ch) {
        if(!_tt->has(ch))
            continue;
        auto selected = _charset.count(ch) > 0;
        if(_page_state == -1)
            _page_state = selected ? 1 : 0;
        else if((_page_state == 0 && selected) || (_page_state == 1 && !selected)) {
            _page_state = 2;
            break;
        }
    }
    if(_page_state == -1)
        _page_state = 0;
    if(old != _page_state)
        emit pageStateChanged(page, _page_state);
}

void CharsetCanvas::updatePagesState()
{
    std::set<const Page*> pages;
    for(auto& ch : _charset) {
        auto page = CharsetBlocks::find_page(ch);
        if(!page) continue;
        pages.insert(page);
    }
    pages.erase(_page);
    for(auto& it : pages) {
        _page_state = 0;
        updatePageState(it);
    }
    _page_state = 0;
    updatePageState();
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

    if(_page) {
        for(int r=0; r<16; ++r) {
            for(int c=0; c<16; ++c) {
                auto ch = _page->first + r * 16 + c;

                QRectF rc(c*w, r*h, w, h );
                if(ch > _page->last) { // 字符不在范围内
                    p.fillRect(rc,QColor(50,50,50));
                    continue;
                }
                auto pix = get(ch);
                if(!pix) continue;

                if(_charset.count(ch)) // 已选
                    p.fillRect(rc,QColor(200,200,200));
                else
                    p.fillRect(rc,QColor(90,90,90));

                double w1=pix->width()*ratio, h1=pix->height()*ratio;
                auto x = rc.x() + (w - w1) * 0.5;
                auto y = rc.y() + (h - h1) * 0.5;
                p.drawPixmap(x,y, w1,h1, *pix);
            }
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
    if(!_page)
        return;
    auto cs = cellSize();
    auto w = cs.width(), h = cs.height();

    for(int r=0; r<16; ++r) {
        for(int c=0; c<16; ++c) {
            auto ch = _page->first + r * 16 + c;
            if(!_tt->has(ch))
                continue;

            QRectF rc(c*w, r*h, w, h );
            if(!point_in_rect(rc,pos))
                continue;

            const auto is_selected = _charset.count(ch) > 0;
            if(drag) {
                if(_drag_select) {
                    _charset.insert(ch);
                    if(!is_selected) emit selected(ch);
                } else {
                    _charset.erase(ch);
                    if(is_selected) emit unselected(ch);
                }
            } else {
                if(is_selected) {
                    _charset.erase(ch);
                    emit unselected(ch);
                    _drag_select = false;
                } else {
                    _charset.insert(ch);
                    emit selected(ch);
                    _drag_select = true;
                }
            }
            updatePageState();
            update();
            return;
        }
    }
}

void CharsetCanvas::updateCache(int fontHeight, bool force)
{
    if(!_page || fontHeight < 4)
        return;
    if(!force && _font_height > 0 && (fontHeight < _font_height * 1.3 && fontHeight > _font_height * 0.75))
        return;
    _font_height = fontHeight;
    _font_scale = _tt->scaleForHeight(_font_height);
    _bound_w = 0; _bound_h = 0;
    _count = 0;
    for(auto ch=_page->first; ch<=_page->last; ++ch) {
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
    if(!_tt)
        return;
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
    ui->list_page->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->list_page->setUniformItemSizes(true);
    ui->list_page->setViewMode(QListView::ListMode);
    ui->list_page->setWrapping(false);
    auto act = new QAction(tr("从文件选择字符"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onOpen); _menu.addAction(act);
    act = new QAction(tr("从文件追加字符"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onAppend); _menu.addAction(act);
    act = new QAction(tr("清除选择"));
    connect(act,&QAction::triggered,this,&CharsetSelector::onClear); _menu.addAction(act);

    connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& pos){
        _menu.popup(mapToGlobal(pos));
    });
    // 切换页
    connect(ui->list_page,&QListWidget::currentRowChanged,this,[this](int row){
        auto item = ui->list_page->currentItem();
        if(!item) return;
        bool ok = false;
        auto page = (const CharsetBlocks::Page*)item->data(Qt::UserRole).toLongLong(&ok);
        if(!ok) return;
        ui->canvas->setPage(page);
    });
    // 页全选/全不选
    connect(ui->list_page,&QListWidget::itemChanged,this,[this](const QListWidgetItem* item){
        if(!item) return;
        bool ok = false;
        auto page = (const CharsetBlocks::Page*)item->data(Qt::UserRole).toLongLong(&ok);
        if(!ok) return;
        if(item->checkState() & Qt::Checked)
            ui->canvas->selectPage(page);
        else
            ui->canvas->unselectPage(page);
    });
    connect(ui->canvas, &CharsetCanvas::pageStateChanged,this,[this](const Page* page,int state){
        QListWidgetItem* item = nullptr;
        auto count = ui->list_page->count();
        for(int i=0;i<count;++i) {
            auto _item = ui->list_page->item(i);
            if(!_item) continue;
            if((Page*)_item->data(Qt::UserRole).toLongLong() == page){
                item = _item;
                break;
            }
        }
        if(!item) return;

        ui->list_page->blockSignals(true);
        switch (state) {
        case 0: item->setCheckState(Qt::Unchecked); break;
        case 1: item->setCheckState(Qt::Checked); break;
        case 2: item->setCheckState(Qt::PartiallyChecked); break;
        }
        ui->list_page->blockSignals(false);
    });

    // page加载进度
    connect(&_pager, &CharsetPager::progress, ui->pgb, &QProgressBar::setValue);
    // page加载完成
    connect(&_pager, &CharsetPager::done, this, [this](const std::vector<const CharsetBlocks::Page*>& pages){
        ui->pgb->setVisible(false);
        ui->list_page->clear();
        QString fmt("%1 ");
        for(auto & it : pages) {
            auto name = fmt.arg(it->first,6,16,'0').toUpper() + it->block->name;
            auto item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, (uintptr_t)it);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            ui->list_page->addItem(item);
        }
        auto item = ui->list_page->item(0);
        if(item) {
            item->setSelected(true);
            auto page = (const CharsetBlocks::Page*)item->data(Qt::UserRole).toLongLong();
            ui->canvas->setPage(page);
            ui->canvas->updatePagesState();
        }
    });
}

CharsetSelector::~CharsetSelector()
{
    _pager.terminate();
    delete ui;
}

void CharsetSelector::setCharset(const Charset &charset)
{
    ui->canvas->setCharset(charset);
    updateLabel();
}

const Charset &CharsetSelector::charset() const
{
    return ui->canvas->charset();
}

void CharsetSelector::setCanvasFont(const font::TrueType* font)
{
    _pager.setFont(font);
    ui->canvas->setFont(font);
    updateLabel();
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
    ui->list_page->clear();
    _pager.start();
}

void CharsetSelector::updateLabel()
{
    static QString fmt;
    if(fmt.isEmpty())
        fmt = ui->lab_selected->text();
    auto str = fmt.arg(ui->canvas->charset().size()).arg(_pager.total());
    ui->lab_selected->setText(str);
}

void CharsetSelector::onOpen()
{
    QFileDialog dlg(this,tr("从文本文件选择字符"),"","UTF-8 Text File (*.txt) ;; All File (*.*)");
    if(dlg.exec() != QDialog::Accepted)
        return;
    QFile file(dlg.selectedFiles().front());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Warning(tr("文件无法打开!"));
        return;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    auto content = in.readAll().toUcs4();
    file.close();
    ui->canvas->setCharset({ content.begin(),content.end() });
    ui->canvas->updatePagesState();
    updateLabel();
}

void CharsetSelector::onAppend()
{
    QFileDialog dlg(this,tr("从文本文件追加字符"),"","UTF-8 Text File (*.txt) ;; All File (*.*)");
    if(dlg.exec() != QDialog::Accepted)
        return;
    QFile file(dlg.selectedFiles().front());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Warning(tr("文件无法打开!"));
        return;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    auto content = in.readAll().toUcs4();
    file.close();
    auto charset = ui->canvas->charset();
    charset.insert(content.begin(),content.end());
    ui->canvas->setCharset(charset);
    ui->canvas->updatePagesState();
    updateLabel();
}

void CharsetSelector::onClear()
{
    ui->canvas->clear();
    int count = ui->list_page->count();
    for(int i=0; i<count; ++i) {
        ui->list_page->item(i)->setCheckState(Qt::Unchecked);
    }
    updateLabel();
}
