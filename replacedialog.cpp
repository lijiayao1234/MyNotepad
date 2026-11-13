#include "replacedialog.h"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    ui->rbDown->setChecked(true);
    connect(ui->rbCancel, &QPushButton::clicked, this, &QDialog::close);
}

ReplaceDialog::~ReplaceDialog() { delete ui; }

void ReplaceDialog::on_rbFindNext_clicked()
{
    emit findRequested(ui->SearchText->text(), ui->rbDown->isChecked(), ui->rbCaseSensetive->isChecked());
}

void ReplaceDialog::on_rbReplace_clicked()
{
    emit replaceRequested(ui->SearchText->text(), ui->ReplaceText->text(), false, ui->rbCaseSensetive->isChecked(), ui->rbDown->isChecked());
}

void ReplaceDialog::on_rbAllReplace_clicked()
{
    emit replaceRequested(ui->SearchText->text(), ui->ReplaceText->text(), true, ui->rbCaseSensetive->isChecked(), ui->rbDown->isChecked());
}
