#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
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
    //model->insertRow(0, QModelIndex());

    ui->tableTriggers->setModel(model);
    ui->tableTriggers->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
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

void eTriggerMain::on_pushStart_clicked()
{

}

void eTriggerMain::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open protocol file", "");

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Open file error"), tr("Canot open the file, aborting !"));
        return;
    }
    TriggerModel *model = new TriggerModel();
    QTextStream in(&file);
    model->fillModelFromXml(in.readAll());
    file.close();

    ui->tableTriggers->setModel(model);
}

void eTriggerMain::on_actionSave_triggered()
{
    TriggerModel *model = static_cast<TriggerModel *>(ui->tableTriggers->model());

    QString xml = model->getModelXml();
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save protocol"),
        tr(""),
        tr("All Files (*);;Xml Files (*.xml)"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, "Save file error", tr("AN error occurred while saving the file !"));
            return;
        }
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << xml;
    }
}
