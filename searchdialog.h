#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H
#include <QDialog>
namespace Ui { class searchDialog; }

class searchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit searchDialog(QWidget *parent = nullptr);
    ~searchDialog();
signals:
    void findRequested(const QString &text, bool forward, bool caseSensitive);
private slots:
    void on_btFindNext_clicked();
    // void on_btCancel_clicked();  <-- 删除或注释掉这一行
private:
    Ui::searchDialog *ui;
};
#endif
