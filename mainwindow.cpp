/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	mainwindow.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include <fnt_adapter.h>
#include <dialog/dlg_import_bmfont.h>
#include <dialog/dlg_about.h>
#include <dialog/dlg_preview.h>
#include <dialog/dlg_info.h>
#include <QSettings>
#include <QTranslator>
#include <appmodel.h>

QString language_name(const QString& filename, QString& locName)
{
    static QRegularExpression regx("(.*)_(.+)_(.+)\\.qm");
    auto mt = regx.match(filename);
    if(!mt.hasCaptured(3) || mt.captured(1) != "ckfonteditor")
        return "";
    locName = mt.captured(2)+"_"+mt.captured(3);
    QLocale loc(locName);
    return loc.nativeLanguageName();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    _unsaved(false),_compress_way(CW_Unknown)
{
    ui->setupUi(this);
    g_appm._statusbar = ui->statusbar;

    _title = windowTitle();
    ui->btn_background->setColor(ui->w_list->charBackground());
    ui->btn_transparent->setColor(ui->w_list->fontTransparent());

    connect(ui->cob_rate,&QComboBox::currentIndexChanged,this,&MainWindow::onRate);
    connect(ui->btn_background,&ColorButton::colorChanged,this,&MainWindow::onBackground);
    connect(ui->btn_transparent,&ColorButton::colorChanged,this,&MainWindow::onTransparent);
    connect(ui->btn_preview,&QPushButton::clicked,this,&MainWindow::onPreview);

    connect(ui->act_open,&QAction::triggered,this,&MainWindow::onOpen);
    connect(ui->act_save,&QAction::triggered,this,&MainWindow::onSave);
    connect(ui->act_saveas,&QAction::triggered,this,&MainWindow::onSaveAs);
    connect(ui->act_bmfont,&QAction::triggered,this,&MainWindow::onImportBMFont);
    connect(ui->act_close,&QAction::triggered,this,&MainWindow::onClose);
    connect(ui->act_info,&QAction::triggered,this,&MainWindow::onInfo);
    connect(ui->act_sort_asc,&QAction::triggered,this,&MainWindow::onSortAsc);
    connect(ui->act_sort_desc,&QAction::triggered,this,&MainWindow::onSortDesc);
    connect(ui->act_about,&QAction::triggered,this,&MainWindow::onAbout);
    connect(ui->w_list,&FontListWidget::dataChanged,this,[this](){
        _unsaved = true;
        updateTitle();
    });


    QString locName;
    QTranslator tra;
    auto path = qApp->applicationDirPath()+"/translations/";
    QDir dir(path);
    for(auto& it : dir.entryList({"*.qm"},QDir::Files))
    {
        auto name = language_name(it,locName);
        if(name.isEmpty() || !tra.load(path+it))
            continue;
        auto act = new QAction(name);
        act->setData(locName);
        connect(act,&QAction::triggered,this,&MainWindow::onLanguage);
        ui->menu_language->addAction(act);
    }
    connect(ui->act_chinese,&QAction::triggered,this,&MainWindow::onLanguage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTitle()
{
    if(_filename.isEmpty())
    {
        if(_unsaved)
            setWindowTitle(_title+tr(" [新建的]"));
        else
            setWindowTitle(_title);
    }
    else
    {
        if(_unsaved)
            setWindowTitle(_title+tr(" [未保存的]")+" : \""+_filename+"\"");
        else
            setWindowTitle(_title+" : \""+_filename+"\"");
    }
}

void MainWindow::onOpen()
{
    if(!checkSave())
        return;
    QFileDialog dlg(this,tr("打开字体"),"",tr("CK字体 (*.ckf)"));
    dlg.setFileMode(QFileDialog::ExistingFile);
    if(dlg.exec() == QDialog::Accepted)
    {
        if(_compress_way < CW_AllFileYes)
            _compress_way = CW_Unknown;
        auto filename = dlg.selectedFiles().front();
        const auto path = filename.toLocal8Bit().toStdString();
        if(ui->w_list->load(path))
        {
            _filename = filename;
            _unsaved = false;
            updateTitle();
        }
    }
}

void MainWindow::onSave()
{
    if(!_unsaved && QFile::exists(_filename))
        return;

    if(_filename.isEmpty())
        onSaveAs();
    else
    {
        if(_compress_way == CW_Unknown)
        {
            DlgCompress dlg;
            dlg.exec();
            _compress_way = dlg.way();
        }
        const auto path = _filename.toLocal8Bit().toStdString();
        const bool compress = _compress_way == CW_ThisFileYes|| _compress_way == CW_AllFileYes;
        if(ui->w_list->save(path,compress))
        {
            _unsaved = false;
            updateTitle();
        }
    }
}

void MainWindow::onSaveAs()
{
    if(_compress_way < CW_AllFileYes && QMessageBox::question(this,tr("确认"),tr("要压缩吗?")) == QMessageBox::Yes)
        _compress_way = CW_ThisFileYes;
    QFileDialog dlg(this,tr("另存为字体"),"",tr("CK字体 (*.ckf)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec() == QDialog::Accepted)
    {
        auto filename = dlg.selectedFiles().front();
        const auto path = filename.toLocal8Bit().toStdString();
        const bool compress = _compress_way == CW_ThisFileYes|| _compress_way == CW_AllFileYes;
        if(ui->w_list->save(path,compress))
        {
            _filename = filename;
            _unsaved = false;
            updateTitle();
        }
    }
}

void MainWindow::onClose()
{
    if(!checkSave())
        return;
    ui->w_list->clear();
    _filename.clear();
    _unsaved = false;
    updateTitle();
}

void MainWindow::onImportBMFont()
{
    if(!checkSave())
        return;

    DlgImportBMFont dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        if(_compress_way < CW_AllFileYes)
            _compress_way = CW_Unknown;
        auto& filename = dlg.filename();
        auto& transparent = dlg.transparent();
        const auto path = filename.toLocal8Bit().toStdString();
        FntAdapter adp;
        adp.load(path,toCKColor(transparent),dlg.bit32());
        adp.header().spacingX = dlg.spcingX();
        if(ui->w_list->load(adp))
        {
            ui->btn_transparent->setColor(transparent);
            _filename.clear();
            _unsaved = true;
            updateTitle();
        }
    }
}

void MainWindow::onInfo()
{
    auto& font = ui->w_list->ckfont();
    Font::Header header = font.header();
    DlgInfo dlg(this,&header);
    if(dlg.exec())
    {
        font.setHeader(header);
        _unsaved = true;
        updateTitle();
    }
}

static void sort(Font& font,bool asc)
{
    Font::CharList chrs = font.chrs();
    std::map<uint32_t,Font::Data> map;
    Font::Data tmp;
    for(auto& it : chrs)
    {
        if(font.getData(it,tmp))
            map.insert(std::make_pair(it.code,std::move(tmp)));
    }
    std::sort(chrs.begin(),chrs.end(),[asc](auto& a,auto& b){
        return asc ? a.code < b.code : a.code > b.code;
    });
    Font::Header header = font.header();
    font.clear();
    font.setHeader(header);
    for(auto& it : chrs)
    {
        auto iter = map.find(it.code);
        if(iter == map.end())
            continue;
        font.insert(it,iter->second);
    }
}

void MainWindow::onSortAsc()
{
    sort(ui->w_list->ckfont(),true);
    _unsaved = true;
    updateTitle();

    emit ui->w_list->redrawItems();
    ui->w_list->repaint();
}

void MainWindow::onSortDesc()
{
    sort(ui->w_list->ckfont(),false);
    _unsaved = true;
    updateTitle();

    emit ui->w_list->redrawItems();
    ui->w_list->repaint();
}

void MainWindow::onPreview()
{
    auto& font = ui->w_list->ckfont();
    DlgPreview dlg(&font);
    dlg.exec();
}

void MainWindow::onAbout()
{
    DlgAbout dlg(this);
    dlg.exec();
}

void MainWindow::onBackground(const QColor& color)
{
    ui->w_list->setCharBackground(color);
    ui->w_list->redrawItems();
}

void MainWindow::onTransparent(const QColor& color)
{
    ui->w_list->setFontTransparent(color);
    ui->w_list->redrawItems();
}

void MainWindow::onRate(int idx)
{
    float rate = 1;
    switch (idx) {
    case 0:     // 1/4
        rate = 0.25;
        break;
    case 1:     // 1/2
        rate = 0.5;
        break;
    case 2:     // 1
        rate = 1;
        break;
    case 3:     // 2/1
        rate = 2;
        break;
    case 4:     // 4/1
        rate = 4;
        break;
    default: return;
    }
    ui->w_list->setCharRate(rate);
    ui->w_list->redrawItems();
}

void MainWindow::onLanguage()
{
    QSettings sets(QSettings::UserScope);
    auto act = (QAction*)sender();
    if(act == ui->act_chinese)
        sets.setValue("Language","zh_cn");
    else
        sets.setValue("Language",act->data().toString());
    Information(tr("重启后生效"));
}

bool MainWindow::checkSave()
{
    if(_unsaved && !Question(tr("当前更改还未保存,要放弃这些更改吗?")))
        return false;
    return true;
}
