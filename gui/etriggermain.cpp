#include "etriggermain.h"
#include "triggermodel.h"
#include "checkboxdelegate.h"
#include "ui_etriggermain.h"

eTriggerMain::eTriggerMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::eTriggerMain)
{
    ui->setupUi(this);
    TriggerModel *model = new TriggerModel();
    model->insertRow(0, QModelIndex());

    ui->tableTriggers->setModel(model);
    /*ui->tableTriggers->setItemDelegateForColumn(1, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(2, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(3, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(4, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(5, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(6, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(7, new CheckBoxDelegate());
    ui->tableTriggers->setItemDelegateForColumn(8, new CheckBoxDelegate());
*/
}

eTriggerMain::~eTriggerMain()
{
    delete ui;
}

void eTriggerMain::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void eTriggerMain::on_pushAdd_clicked()
{
    TriggerModel *model = static_cast<TriggerModel *> (ui->tableTriggers->model());
    model->insertRow(model->rowCount(QModelIndex()), QModelIndex());
}

void eTriggerMain::on_pushRemove_clicked()
{

}
