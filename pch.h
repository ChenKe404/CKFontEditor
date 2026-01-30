/*
*******************************************************************************
    ChenKe404's CKF editor
*******************************************************************************
@project	ckfonteditor
@authors	chenke404
@file	pch.h

// SPDX-License-Identifier: MIT
// Copyright (c) 2025 chenke404
******************************************************************************
*/

#ifndef PCH_H
#define PCH_H

#include <QWidget>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>

#include <file.h>
using namespace ck;
using font::color;

inline color toCKColor(const QColor& c)
{ return font::argb(c.alpha(),c.red(),c.green(),c.blue()); }

inline color toCKColor(const QRgb& c)
{ return font::argb(qAlpha(c),qRed(c),qGreen(c),qBlue(c)); }

inline QColor toQColor(const color& c)
{ return QColor{ font::cr(c),font::cg(c),font::cb(c),font::ca(c) }; }

inline bool Question(const QString& text,const QString& caption = QObject::tr("请确认"))
{ return QMessageBox::question(nullptr,caption,text) == QMessageBox::Yes; }

inline bool Information(const QString& text,const QString& caption = QObject::tr("提示"))
{ return QMessageBox::information(nullptr,caption,text) == QMessageBox::Ok; }

inline bool Warning(const QString& text,const QString& caption = QObject::tr("警告"))
{ return QMessageBox::warning(nullptr,caption,text) == QMessageBox::Ok; }

#endif // PCH_H
