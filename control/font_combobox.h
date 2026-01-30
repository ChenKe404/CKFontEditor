#ifndef FONT_COMBOBOX_H
#define FONT_COMBOBOX_H

#include <QComboBox>
#include <truetype.h>

class QStandardItem;
class FontComboBox : public QComboBox
{
public:
    FontComboBox(QWidget* parent = nullptr);
protected:
    void addFont(const std::string& family,const font::TrueTypeManager::Meta& meta);
};

#endif // FONT_COMBOBOX_H
