/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_info.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_info.h"
#include "ui_dlg_info.h"
#include <QRegularExpression>

DlgInfo::DlgInfo(QWidget *parent,Font::Header* header) :
    QDialog(parent),
    ui(new Ui::DlgInfo),
    _header(header)
{
    ui->setupUi(this);
    ui->edt_count->setText(QString::number(header->count));

    QString lang;
    for(int i=0;i<4;++i)
    {
        char c = header->lang[i];
        lang.append(c==0 ? '?' : c);
    }
    ui->edt_lang->setText(lang);
    QRegularExpression regex("[A-Za-z?]{0,4}$");
    ui->edt_lang->setValidator(new QRegularExpressionValidator(regex,this));

    ui->spb_lineheight->setValue(header->lineHeight);
    ui->spb_lineheight->setMinimum(1);

    ui->spb_maxwidth->setValue(header->maxWidth);
    ui->spb_maxwidth->setMinimum(1);

    ui->spb_spacing->setValue(header->spacingX);
    // ui->chk_compressed->setChecked(header->compressed);
    auto fmt = ui->lab_padding->text();
    auto padding = header->padding;
    ui->lab_padding->setText(fmt.arg(padding[0]).arg(padding[1]).arg(padding[2]).arg(padding[3]));

    ui->btn_transparent->setColor(toQColor(header->transparent));
}

DlgInfo::~DlgInfo()
{
    delete ui;
}

void DlgInfo::accept()
{
    QString lang = ui->edt_lang->text();
    memset(_header->lang,0,4);
    for(int i=0;i<4 && i<lang.size();++i)
    { _header->lang[i] = lang.at(i).toLatin1(); }

    _header->lineHeight = ui->spb_lineheight->value();
    _header->maxWidth = ui->spb_maxwidth->value();
    _header->transparent = toCKColor(ui->btn_transparent->color());
    _header->spacingX = ui->spb_spacing->value();
    // _header->compressed = ui->chk_compressed->isChecked();
    QDialog::accept();
}
