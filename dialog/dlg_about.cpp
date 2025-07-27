/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_about.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "dlg_about.h"
#include "ui_dlg_about.h"

DlgAbout::DlgAbout(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAbout)
{
    ui->setupUi(this);

    if(strlen(PRJ_ARCH) == 0)
        ui->lab_title->setText("CKFontEditor");
    else
        ui->lab_title->setText("CKFontEditor (" PRJ_ARCH ")");

    ui->lab_version->setText(
        tr("版本: %1    基于Qt%2.%3.%4")
            .arg(PRJ_VERSION)
            .arg(QT_VERSION_MAJOR)
            .arg(QT_VERSION_MINOR)
            .arg(QT_VERSION_PATCH)
        );

    ui->lab_build_date->setText(
        tr("构建时间: %1 %2")
            .arg(__DATE__)
            .arg(__TIME__)
        );

    ui->lab_licence->setText(
        tr("许可: %1")
            .arg(PRJ_LICENCE)
        );

    QString str;
    ui->lab_repos->setOpenExternalLinks(true);
    auto sp = QString(PRJ_REPOS).split('|');
    for(auto& it : sp)
    {
        if(it.isEmpty())
            continue;
        str.append(QString("<a href=\"%1\">%1</a><br/>").arg(it));
    }
    if(!str.isEmpty())
    {
        str.remove(str.size()-5,5);
    }
    ui->lab_repos->setText(str);
}

DlgAbout::~DlgAbout()
{
    delete ui;
}
