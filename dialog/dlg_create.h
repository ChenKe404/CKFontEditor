#ifndef DLG_CREATE_H
#define DLG_CREATE_H

#include <QDialog>
#include <ckfont/truetype.h>

namespace Ui {
class DlgCreate;
}

class DlgCreate : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCreate(QWidget *parent = nullptr);
    ~DlgCreate();

public slots:
    void accept() override;
private:
    void onPreview();
    void onOpen(const QString& filename);
    void onSelect();
    void onReset();
    void onFamilyChanged();
    void onStyleChanged();
private:
    Ui::DlgCreate *ui;
    font::TrueTypeManager::MetaMap _map;    // 当前打开的字体的元数据
    font::TrueType _tt;
};

#endif // DLG_CREATE_H
