/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	appmodel.cpp

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#include "appmodel.h"

AppModel g_appm;

AppModel::AppModel() {}

void AppModel::printStatus(const QString &text)
{
    _statusbar->showMessage(text);
}
