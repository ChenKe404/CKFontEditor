/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	appmodel.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef APPMODEL_H
#define APPMODEL_H

#include <QStatusBar>

class AppModel
{
    AppModel(const AppModel&) = delete;
public:
    AppModel();
    void printStatus(const QString& text);
private:
    QStatusBar* _statusbar;
    friend class MainWindow;
};
extern AppModel g_appm;

#endif // APPMODEL_H
