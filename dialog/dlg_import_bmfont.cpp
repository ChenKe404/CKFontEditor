/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_import_bmfont.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_import_bmfont.h"
#include "ui_dlg_import_bmfont.h"

DlgImportBMFont::DlgImportBMFont(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DlgImportBMFont)
{
    ui->setupUi(this);
    connect(ui->btn_path,&QPushButton::clicked,this,&DlgImportBMFont::onSelect);
    ui->btn_color->setColor(qRgb(0,0,0));
}

DlgImportBMFont::~DlgImportBMFont()
{
    delete ui;
}

const QString &DlgImportBMFont::filename()
{
    return _filename;
}

const QColor &DlgImportBMFont::transparent()
{
    return ui->btn_color->color();
}

bool DlgImportBMFont::bit32() const
{
    return ui->chb_bit32->isChecked();
}

int DlgImportBMFont::spcingX()
{
    return ui->spb_spacingx->value();
}

void DlgImportBMFont::onSelect()
{
    QFileDialog dlg(this,tr("导入BMFont字体"),"",tr("BMFont字体 (*.fnt)"));
    dlg.setFileMode(QFileDialog::ExistingFile);
    if(dlg.exec() == QDialog::Accepted)
    {
        auto list = dlg.selectedFiles();
        _filename = list.front();
        ui->edt_path->setText(_filename);
    }
}
