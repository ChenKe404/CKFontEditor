/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	mainwindow.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dialog/dlg_compress.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateTitle();

    void onOpen();
    void onSave();
    void onSaveAs();
    void onClose();
    void onImportBMFont();
    void onInfo();
    void onSortAsc();
    void onSortDesc();
    void onPreview();
    void onAbout();
    void onBackground(const QColor&);
    void onTransparent(const QColor&);
    void onRate(int idx);
    void onLanguage();
private:
    bool checkSave();
private:
    Ui::MainWindow *ui;
    QString _title;
    QString _filename;
    bool _unsaved;
    CompressWay _compress_way;
};
#endif // MAINWINDOW_H
