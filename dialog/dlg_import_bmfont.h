/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	dlg_import_bmfont.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef DLG_IMPORT_BMFONT_H
#define DLG_IMPORT_BMFONT_H

#include <QDialog>

namespace Ui {
class DlgImportBMFont;
}

class DlgImportBMFont : public QDialog
{
    Q_OBJECT

public:
    explicit DlgImportBMFont(QWidget *parent = nullptr);
    ~DlgImportBMFont();

    const QString& filename();
    const QColor& transparent();
    bool bit32() const;
    int spcingX();
private:
    void onSelect();
private:
    Ui::DlgImportBMFont *ui;
    QString _filename;
};

#endif // DLG_IMPORT_BMFONT_H
