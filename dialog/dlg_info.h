/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_info.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_INFO_H
#define DLG_INFO_H

#include <QDialog>

namespace Ui {
class DlgInfo;
}

class DlgInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DlgInfo(QWidget *parent,Font::Header* header);
    ~DlgInfo();

private:
    void accept() override;
private:
    Ui::DlgInfo *ui;
    Font::Header* _header;
};

#endif // DLG_INFO_H
