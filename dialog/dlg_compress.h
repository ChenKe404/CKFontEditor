/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_compress.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_COMPRESS_H
#define DLG_COMPRESS_H

#include <QButtonGroup>
#include <QDialog>

namespace Ui {
class DlgCompress;
}

enum CompressWay
{
    CW_Unknown,
    CW_ThisFileYes,
    CW_ThisFileNo,
    CW_AllFileYes,
    CW_AllFileNo
};

class DlgCompress : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCompress(QWidget *parent = nullptr);
    ~DlgCompress();

    void accept() override;
    void reject() override;

    CompressWay way() const;
private:
    Ui::DlgCompress *ui;
    QButtonGroup _group;
    CompressWay _way;
};

#endif // DLG_COMPRESS_H
