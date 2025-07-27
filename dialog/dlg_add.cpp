/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_add.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_add.h"
#include "ui_dlg_add.h"

DlgAdd::DlgAdd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAdd)
{
    ui->setupUi(this);
}

DlgAdd::~DlgAdd()
{
    delete ui;
}
