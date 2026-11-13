#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H
#include <QDialog>
namespace Ui { class ReplaceDialog; }

class ReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();
signals:
    void findRequested(const QString &text, bool forward, bool caseSensitive);
    void replaceRequested(const QString &target, const QString &replacement, bool all, bool caseSensitive, bool forward);
private slots:
    void on_rbFindNext_clicked();
    void on_rbReplace_clicked();
    void on_rbAllReplace_clicked();
private:
    Ui::ReplaceDialog *ui;
};
#endif
