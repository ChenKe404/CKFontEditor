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

#include <font.h>
using namespace ck;

inline color toCKColor(const QColor& c)
{ return ck::argb((uint8_t)c.alpha(),(uint8_t)c.red(),(uint8_t)c.green(),(uint8_t)c.blue()); }

inline QColor toQColor(const color& c)
{ return qRgba(cr(c),cg(c),cb(c),ca(c)); }

inline bool Question(const QString& text,const QString& caption = QObject::tr("请确认"))
{ return QMessageBox::question(nullptr,caption,text) == QMessageBox::Yes; }

inline bool Information(const QString& text,const QString& caption = QObject::tr("提示"))
{ return QMessageBox::information(nullptr,caption,text) == QMessageBox::Ok; }

inline bool Warning(const QString& text,const QString& caption = QObject::tr("警告"))
{ return QMessageBox::warning(nullptr,caption,text) == QMessageBox::Ok; }

#endif // PCH_H
