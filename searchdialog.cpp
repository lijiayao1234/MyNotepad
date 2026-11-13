#include "searchdialog.h"
#include "ui_searchdialog.h"

searchDialog::searchDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::searchDialog)
{
    ui->setupUi(this);
    ui->rbDown->setChecked(true);
    connect(ui->btCancel, &QPushButton::clicked, this, &QDialog::close);
}

searchDialog::~searchDialog() { delete ui; }

void searchDialog::on_btFindNext_clicked()
{
    emit findRequested(ui->searchText->text(), ui->rbDown->isChecked(), ui->rbCaseSensetive->isChecked());
}
