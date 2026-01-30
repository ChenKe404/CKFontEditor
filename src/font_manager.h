#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

class FontManager
{
    struct Meta{
        uint32_t index;
        QString family;
        QString style;
    };
public:
    FontManager();
    static void refresh();
};

#endif // FONT_MANAGER_H
