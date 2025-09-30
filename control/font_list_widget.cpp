/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	font_list_widget.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "font_list_widget.h"
#include <QPaintEvent>
#include <QScrollBar>
#include <QFileDialog>
#include <flowlayout.h>
#include <appmodel.h>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <dialog/dlg_add.h>

constexpr int SPACING = 2;
constexpr int PADDING = 2;

FontListWidget::FontListWidget(QWidget *parent)
    : QWidget{parent},FontItemDrawer(&_font),
    _th(this)
{
    auto layout = new FlowLayout;
    layout->setHorizontalSpacing(2);
    layout->setVerticalSpacing(2);
    layout->setContentsMargins({ 0,0,0,0 });

    _act_export = new QAction(tr("导出字符图像"));
    connect(_act_export,&QAction::triggered,this,&FontListWidget::onExport);
    _act_import = new QAction(tr("替换字符图像"));
    connect(_act_import,&QAction::triggered,this,&FontListWidget::onImport);
    _act_add = new QAction(tr("添加字符"));
    connect(_act_add,&QAction::triggered,this,&FontListWidget::onAdd);
    _act_delete = new QAction(tr("删除字符"));
    connect(_act_delete,&QAction::triggered,this,&FontListWidget::onDelete);
    _menu.addAction(_act_export);
    _menu.addAction(_act_import);
    _menu.addSeparator();
    _menu.addAction(_act_add);
    _menu.addAction(_act_delete);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setLayout(layout);
    connect(this,&QWidget::customContextMenuRequested,this,&FontListWidget::onContextMenuRequested);
    connect(&_th,&FontItemDrawerThread::done,this,QOverload<>::of(&FontListWidget::repaint));
}

#include <font_texture.h>

bool FontListWidget::load(const std::string &filename)
{
    if(!_font.load(filename))
    {
        g_appm.printStatus(tr("打开字体失败..."));
        return false;
    }

    onLoad();
    return true;
}

bool FontListWidget::load(const Font::Adapter &adp)
{
    if(!_font.load(adp))
    {
        g_appm.printStatus(tr("打开字体失败..."));
        return false;
    }

    onLoad();
    return true;
}

bool FontListWidget::save(const std::string &filename,bool compress)
{
    if(_font.save(filename,compress))
    {
        g_appm.printStatus(tr("保存成功"));
        return true;
    }
    else
    {
        g_appm.printStatus(tr("保存失败!"));
        return false;
    }
}

void FontListWidget::clear()
{
    _font.clear();

    // 只是隐藏
    auto layout = (FlowLayout*)this->layout();
    const auto count = layout->count();
    for(int i=0;i<count;++i)
    {
        layout->itemAt(i)->widget()->setVisible(false);
    }
}

Font &FontListWidget::ckfont()
{
    return _font;
}

void FontListWidget::removeItem(int index)
{
    auto layout = (FlowLayout*)this->layout();
    auto item = layout->itemAt(index);
    if(item)
    {
        item->widget()->setParent(nullptr);
        layout->removeItem(item);
    }
}

void FontListWidget::onLoad()
{
    emit loaded();
    g_appm.printStatus(tr("正在读取..."));
    redrawItems();
}

void FontListWidget::redrawItems()
{
    const auto& chrs = _font.chrs();
    auto layout = (FlowLayout*)this->layout();
    auto sub = (int)chrs.size() - layout->count();    // FontItem与字符数量一致
    while(sub < 0)  // 删除多余的
    {
        removeItem(layout->count()-1);
        ++sub;
    }
    while(sub > 0)  // 新增需要的
    {
        layout->addWidget(new FontItem);
        --sub;
    }

    _th.stop();

    const auto size = computeItemSize();
    const auto count = layout->count();
    int i = 0;
    auto iter = chrs.begin();
    while(iter != chrs.end() && i < count)
    {
        auto item = (FontItem*)layout->itemAt(i)->widget();
        item->setVisible(true);
        item->setFixedSize(size);
        item->set(*iter);

        _th.add(item);

        ++iter;
        ++i;
    }

    _th.start();

    g_appm.printStatus(tr("%1个字符").arg(count));
}

void FontListWidget::onContextMenuRequested(const QPoint &pos)
{
    _ctx_item = dynamic_cast<FontItem*>(childAt(pos));
    _act_export->setEnabled(_ctx_item != nullptr);
    _act_import->setEnabled(_ctx_item != nullptr);
    _act_delete->setEnabled(_ctx_item != nullptr);
    _menu.popup(mapToGlobal(pos));
}

void FontListWidget::onExport()
{
    QFileDialog dlg(this,tr("选择输出文件"),"","Bitmap (*.bmp)");
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec() == QFileDialog::Accept)
    {

    }
}

void FontListWidget::onImport()
{
    QFileDialog dlg(this,tr("选择图像文件"),"","Bitmap (*.bmp)");
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if(dlg.exec() == QFileDialog::Accept)
    {

    }
}

void FontListWidget::onAdd()
{
    DlgAdd dlg(this);
    dlg.exec();
}

void FontListWidget::onDelete()
{
    if(!_ctx_item) return;
    if(!Question(tr("你确定要删除这个字符吗?此操作不可恢复!")))
        return;
    _ctx_item->setVisible(false);
    _font.remove(_ctx_item->code());
    emit dataChanged();
}
