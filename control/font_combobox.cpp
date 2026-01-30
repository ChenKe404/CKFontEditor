#include "font_combobox.h"
#include <QFontDatabase>
#include <QStandardItemModel>
#include <truetype.h>

FontComboBox::FontComboBox(QWidget *parent)
    : QComboBox(parent)
{
    auto m = (QStandardItemModel*)model();
    for(auto& it : font::TrueTypeManager::all()){
        const auto& family = it.first;
        addItem(family.c_str(),(std::uintptr_t)&it.second);
    }
}

void FontComboBox::addFont(const std::string &family, const font::TrueTypeManager::Meta &meta)
{
    const auto name = family + " " + meta.style;

    auto item = new QStandardItem(name.c_str());
    item->setData((std::uintptr_t)&meta);
    auto font = QFontDatabase::font(family.c_str(),meta.style.c_str(),12);
    if(font == QFont()) {
        auto path = font::TrueTypeManager::getPath(meta);
        if(QFontDatabase::addApplicationFont(path.c_str()) < 0)
            return;
    }
    font = QFontDatabase::font(family.c_str(),meta.style.c_str(),12);
    if(font == QFont())
        return;

    item->setFont(font);
    // m->setItem(count(),item);
}
