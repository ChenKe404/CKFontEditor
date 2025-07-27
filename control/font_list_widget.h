/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	font_list_widget.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef FONT_LIST_WIDGET_H
#define FONT_LIST_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QMenu>
#include <QThread>
#include <QGraphicsScene>
#include <component/font_item.h>

class FontListWidget : public QWidget, public FontItemDrawer
{
    Q_OBJECT
public:
    explicit FontListWidget(QWidget *parent = nullptr);

    bool load(const std::string& filename);
    bool load(const Font::Adapter& adp);
    bool save(const std::string& filename,bool compress);
    void clear();

    Font& ckfont();

    void redrawItems();
signals:
    void loaded();
    void dataChanged();
private:
    void removeItem(int index);
    void onLoad();
    void onContextMenuRequested(const QPoint&);
    void onExport();
    void onImport();
    void onAdd();
    void onDelete();
private:
    Font _font;
    FontItemDrawerThread _th;

    FontItem* _ctx_item;
    QAction* _act_export;
    QAction* _act_import;
    QAction* _act_add;
    QAction* _act_delete;
    QMenu _menu;
};

#endif // FONT_LIST_WIDGET_H
