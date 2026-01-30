#include "dlg_create.h"
#include "ui_dlg_create.h"

#include <QFontDialog>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QRawFont>
#include <QSettings>
#include <ckfont/texture.h>

using Convertor = font::TrueTypeConvertor;

#define CGR_CREATE "Create/"

class SpnValidator : public QIntValidator
{
public:
    void fixup(QString & str) const override {
        if(str.isEmpty()) str = "0";
        else QIntValidator::fixup(str);
    }
};

DlgCreate::DlgCreate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgCreate)
{
    ui->setupUi(this);

    connect(ui->btn_browse,&QPushButton::clicked,this,&DlgCreate::onSelect);
    connect(ui->ckb_bold,&QCheckBox::clicked,this,[this](bool){
        // onFontChanged(ui->cob_family->currentText());
    });
    connect(ui->ckb_italic,&QCheckBox::clicked,this,[this](bool){
        // onFontChanged(ui->cob_family->currentText());
    });
    connect(ui->cob_family,&QComboBox::currentIndexChanged,this,&DlgCreate::onFamilyChanged);
    connect(ui->cob_style,&QComboBox::currentIndexChanged,this,&DlgCreate::onStyleChanged);
    connect(ui->brn_reset,&QPushButton::clicked,this,&DlgCreate::onReset);

    auto validator = new SpnValidator;
    validator->setRange(0,50);
    ui->edt_pad_left->setValidator(validator);
    ui->edt_pad_top->setValidator(validator);
    ui->edt_pad_right->setValidator(validator);
    ui->edt_pad_bottom->setValidator(validator);
    ui->edt_spac_h->setValidator(validator);
    ui->edt_spac_v->setValidator(validator);

    QSettings sets(QSettings::UserScope);
    auto path = sets.value(CGR_CREATE "FontPath").toString();
    if(path.isEmpty())
        onReset();
    else
        onOpen(path);
    ui->cob_family->setCurrentText(sets.value(CGR_CREATE "FontFamily").toString());
    ui->cob_style->setCurrentText(sets.value(CGR_CREATE "FontStyle").toString());
    ui->ckb_bold->setChecked(sets.value(CGR_CREATE "Bold").toBool());
    ui->ckb_italic->setChecked(sets.value(CGR_CREATE "Italic").toBool());
    ui->spb_fontsize->setValue(sets.value(CGR_CREATE "FontSize").toInt());
    ui->spb_aa->setValue(sets.value(CGR_CREATE "AA").toInt());
    ui->ckb_smooth->setChecked(sets.value(CGR_CREATE "Smooth").toBool());
    ui->ckb_match_height->setChecked(sets.value(CGR_CREATE "MatchCellHeight").toBool());
    ui->edt_pad_left->setText(sets.value(CGR_CREATE "PaddingLeft","0").toString());
    ui->edt_pad_top->setText(sets.value(CGR_CREATE "PaddingTop","0").toString());
    ui->edt_pad_right->setText(sets.value(CGR_CREATE "PaddingRight","0").toString());
    ui->edt_pad_bottom->setText(sets.value(CGR_CREATE "PaddingBottom","0").toString());
    ui->edt_spac_h->setText(sets.value(CGR_CREATE "SpacingH","0").toString());
    ui->edt_spac_v->setText(sets.value(CGR_CREATE "SpacingV","0").toString());
    ui->spb_width->setValue(sets.value(CGR_CREATE "Width").toInt());
    ui->spb_height->setValue(sets.value(CGR_CREATE "Height").toInt());
    ui->ckb_zero_offset->setChecked(sets.value(CGR_CREATE "ZeroOffset").toBool());
    ui->ckb_8bit->setChecked(sets.value(CGR_CREATE "Depth8Bit").toBool());
    ui->ckb_alpha->setChecked(sets.value(CGR_CREATE "Alpha").toBool());
}

DlgCreate::~DlgCreate()
{
    QSettings sets(QSettings::UserScope);
    sets.setValue(CGR_CREATE "FontPath",ui->edt_font->text());
    sets.setValue(CGR_CREATE "FontFamily",ui->cob_family->currentText());
    sets.setValue(CGR_CREATE "Bold",ui->ckb_bold->isChecked());
    sets.setValue(CGR_CREATE "Italic",ui->ckb_italic->isChecked());
    sets.setValue(CGR_CREATE "FontSize",ui->spb_fontsize->value());
    sets.setValue(CGR_CREATE "AA",ui->spb_aa->value());
    sets.setValue(CGR_CREATE "Smooth",ui->ckb_smooth->isChecked());
    sets.setValue(CGR_CREATE "MatchCellHeight",ui->ckb_match_height->isChecked());
    sets.setValue(CGR_CREATE "PaddingLeft",ui->edt_pad_left->text());
    sets.setValue(CGR_CREATE "PaddingTop",ui->edt_pad_top->text());
    sets.setValue(CGR_CREATE "PaddingRight",ui->edt_pad_right->text());
    sets.setValue(CGR_CREATE "PaddingBottom",ui->edt_pad_bottom->text());
    sets.setValue(CGR_CREATE "SpacingH",ui->edt_spac_h->text());
    sets.setValue(CGR_CREATE "SpacingV",ui->edt_spac_v->text());
    sets.setValue(CGR_CREATE "ZeroOffset",ui->ckb_zero_offset->isChecked());
    sets.setValue(CGR_CREATE "Depth8Bit",ui->ckb_8bit->isChecked());
    sets.setValue(CGR_CREATE "Alpha",ui->ckb_alpha->isChecked());
    sets.setValue(CGR_CREATE "Width",ui->spb_width->value());
    sets.setValue(CGR_CREATE "Height",ui->spb_height->value());
    delete ui;
}

void DlgCreate::accept()
{
    auto ttf = ui->edt_font->text().toLocal8Bit().toStdString();
    if(ttf.empty()) {
        Warning(tr("字体路径不能为空!"));
        return;
    }
    // auto charset = ui->edt_charset->toPlainText().toStdU32String();
    // if(charset.empty()) {
    //     Warning(tr("字符集不能为空!"));
    //     return;
    // }
    QFileDialog dlg(this,tr("保存fnt字体到"),"","Fnt (*.fnt)");
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec() != QDialog::Accepted)
        return;
    auto fnt = dlg.selectedFiles().front().toLocal8Bit().toStdString();

    Convertor::Settings sets;
    sets.bold = ui->ckb_bold->isChecked();
    sets.italic = ui->ckb_italic->isChecked();
    sets.fontSize = ui->spb_fontsize->value();
    sets.smooth = ui->ckb_smooth->isChecked();
    sets.aa = ui->spb_aa->value();
    Convertor conv(sets);
    // if(conv.convert(ttf,fnt,(uint32_t*)charset.data(),charset.size()))
    //     Information(tr("导出成功!"));
    // else
    //     Warning(tr("导出失败!"));
    QDialog::accept();
}

void DlgCreate::onPreview()
{

}

void DlgCreate::onOpen(const QString &filename)
{
    const auto file = filename.toLocal8Bit().toStdString();
    _map = font::TrueTypeManager::add(file);
    ui->edt_font->setText(filename);
    ui->edt_font->setCursorPosition(0);
    ui->cob_family->clear();
    for(auto& it : _map) {
        ui->cob_family->addItem(it.first.c_str(),(std::uintptr_t)&it.second);
    }
    ui->cob_family->setCurrentIndex(0);
    onFamilyChanged();
}

void DlgCreate::onSelect()
{
    auto pathlist = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
    QString dir;
    if(!pathlist.empty()) dir = pathlist.front();
    QFileDialog dlg(this,tr("打开字体"),dir,"Font (*.ttf *.otf *.ttc)");
    dlg.setFileMode(QFileDialog::ExistingFile);
    if(dlg.exec() != QDialog::Accepted)
        return;
    onOpen(dlg.selectedFiles().front());
}

void DlgCreate::onReset()
{
    ui->edt_font->clear();
    ui->cob_family->clear();
    auto& all = font::TrueTypeManager::all();
    for(auto& it : font::TrueTypeManager::all()){
        ui->cob_family->addItem(it.first.c_str(),(std::uintptr_t)&it.second);
    }
    ui->cob_family->setCurrentIndex(0);
    onFamilyChanged();
}

using Meta = font::TrueTypeManager::Meta;
using MetaArray = font::TrueTypeManager::MetaArray;
void DlgCreate::onFamilyChanged()
{
    if(ui->cob_family->currentIndex() < 0)
        return;
    bool ok = false;
    auto arr = (MetaArray*)ui->cob_family->currentData().toLongLong(&ok);
    if(!ok) return;
    ui->cob_style->clear();
    for(const Meta& it : *arr) {
        ui->cob_style->addItem(it.style.c_str(),(std::uintptr_t)&it);
    }
}

void DlgCreate::onStyleChanged()
{
    if(ui->cob_style->currentIndex() < 0)
        return;
    bool ok = false;
    auto meta = (Meta*)ui->cob_style->currentData().toLongLong(&ok);
    if(!ok) return;
    QString style(meta->style.c_str());
    auto font = QFontDatabase::font(ui->cob_family->currentText(),style,12);
    if(font == QFont()) {
        auto path = font::TrueTypeManager::getPath(*meta);;
        if(path.empty()) return;
        QFontDatabase::addApplicationFont(path.c_str());
    }
    font = QFontDatabase::font(ui->cob_family->currentText(),style,12);
    if(font == QFont())

    ui->edt_preview->setFont(font);
    ui->w_selector->setCanvasFont(font);
}
