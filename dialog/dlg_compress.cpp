/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_compress.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_compress.h"
#include "ui_dlg_compress.h"

DlgCompress::DlgCompress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgCompress),
    _way(CW_Unknown)
{
    ui->setupUi(this);

    _group.addButton(ui->rad_always_yes_this);
    _group.addButton(ui->rad_always_no_this);
    _group.addButton(ui->rad_always_yes_all);
    _group.addButton(ui->rad_always_no_all);
}

DlgCompress::~DlgCompress()
{
    delete ui;
}

void DlgCompress::accept()
{
    auto chk = _group.checkedButton();
    if(chk == ui->rad_always_yes_this)
        _way = CW_ThisFileYes;
    else if(chk == ui->rad_always_no_this)
        _way = CW_ThisFileNo;
    else if(chk == ui->rad_always_yes_all)
        _way = CW_AllFileYes;
    else if(chk == ui->rad_always_no_all)
        _way = CW_AllFileNo;
    else
        _way = CW_ThisFileYes;
    QDialog::accept();
}

void DlgCompress::reject()
{
    _way = CW_Unknown;
    QDialog::reject();
}

CompressWay DlgCompress::way() const
{
    return _way;
}
