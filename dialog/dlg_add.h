/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_add.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_ADD_H
#define DLG_ADD_H

#include <QDialog>

namespace Ui {
class DlgAdd;
}

class DlgAdd : public QDialog
{
    Q_OBJECT

public:
    explicit DlgAdd(QWidget *parent = nullptr);
    ~DlgAdd();

private:
    Ui::DlgAdd *ui;
};

#endif // DLG_ADD_H
