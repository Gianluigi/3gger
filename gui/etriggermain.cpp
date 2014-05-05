#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include <QSettings>
#include <QCloseEvent>
#include "3gr_config.h"
#include "etriggermain.h"
#include "checkboxdelegate.h"
#include "ui_etriggermain.h"
#include "triggermodel.h"

eTriggerMain::eTriggerMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::eTriggerMain)
{
    ui->setupUi(this);
    //change title
    TriggerModel *model = new TriggerModel();
    //model->insertRow(0, QModelIndex());

    ui->tableTriggers->setModel(model);
    ui->tableTriggers->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->spinNumTrains->setValue(model->getCount());
    ui->spinTrainRate->setValue(1000.0f / (float)model->getInterval());

    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain(ORG_DOMAIN);
    QCoreApplication::setApplicationName(APP_NAME);

    connect(ui->tableTriggers->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(documentWasModified()));
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    qDebug() << "List of ports:";
    bool found = false;
    for (int i = 0; i < ports.size(); i++) {
        qDebug() << "port name:" << ports.at(i).portName;
        qDebug() << "friendly name:" << ports.at(i).friendName;
        qDebug() << "physical name:" << ports.at(i).physName;
        qDebug() << "enumerator name:" << ports.at(i).enumName;
        qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
        qDebug() << "===================================";
        if (ports.at(i).friendName == "FT232R USB UART" || //Mac OS X driver name
            ports.at(i).friendName.contains("USB Serial Port")) //Windows driver name
        {
            found = true;
            this->port = new QextSerialPort(ports.at(i).portName, QextSerialPort::EventDriven);
            port->setBaudRate(BAUD9600);
            port->setFlowControl(FLOW_OFF);
            port->setParity(PAR_NONE);
            port->setDataBits(DATA_8);
            port->setStopBits(STOP_2);
            port->setTimeout(500);
            port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
            connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            statusBar()->showMessage(tr("Usb connection successful."));
        }


    }
    if (!found)
    {
        QMessageBox::critical(this, "USB Connection Error", tr("Cannot open the USB connection !"));
        this->ui->pushStart->setEnabled(false);
        this->ui->pushStop->setEnabled(false);
        port = 0;
        //this->port = new QextSerialPort(ports.at(ports.size()-1).portName, QextSerialPort::EventDriven);
        //port->setBaudRate(BAUD9600);
        //port->setFlowControl(FLOW_OFF);
        //port->setParity(PAR_NONE);
        //port->setDataBits(DATA_8);
        //port->setStopBits(STOP_2);
        //port->setTimeout(500);
        //port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        //connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }

}


eTriggerMain::~eTriggerMain()
{
    delete ui;
    if (port)
    {
        port->close();
        delete port;
    }
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

void eTriggerMain::setTitle(QString title)
{
    setWindowTitle(tr("%1 %2 - %3 [*]").arg(APP_NAME).arg(APP_VERSION).arg(title));
}

void eTriggerMain::documentWasModified()
{
    setWindowModified(true);
}

void eTriggerMain::on_pushAdd_clicked()
{
    TriggerModel *model = static_cast<TriggerModel *> (ui->tableTriggers->model());
    model->insertRow(model->rowCount(QModelIndex()), QModelIndex());
    documentWasModified();
}

void eTriggerMain::on_pushRemove_clicked()
{
    TriggerModel *model = static_cast<TriggerModel *> (ui->tableTriggers->model());
    model->removeRow(ui->tableTriggers->currentIndex().row());
    documentWasModified();
}

void eTriggerMain::on_pushStart_clicked()
{
    TriggerModel *model = static_cast<TriggerModel *> (ui->tableTriggers->model());
    QString cmd = model->getProgramString();

    ui->lcdCounter->display(0);
    port->write(cmd.toAscii());    
    port->write("S");
    qDebug() << "Sending command:" << cmd;
}
void eTriggerMain::onReadyRead()
{
    int avail = port->bytesAvailable();

    if( avail > 0) {
        if (port->canReadLine())
        {
            QByteArray linedata = port->readLine(1024);
            if (linedata[0] == '4')
                QMessageBox::critical(this, tr("USB Error"), linedata);
        }
        else
        {
            QByteArray usbdata;
            usbdata.resize(avail);
            int read = port->read(usbdata.data(), usbdata.size());
            if( read > 0 ) {
                int count = ui->lcdCounter->intValue();
                for (int i=0; i<read; i++)
                    if (usbdata[i] == '*')
                        ++count;
                ui->lcdCounter->display(count);
            }
            qDebug() << "Received data: " << usbdata;
        }
    }
}

void eTriggerMain::on_actionOpen_triggered()
{
    if (!trySaveProtocol()) return;
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open protocol file"),  settings.value("mainWindow/LastOpenPath", QDir::homePath()).toString() , tr("XML files (*.xml);; All files (*.*)"));

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
    ui->spinNumTrains->setValue(model->getCount());
    ui->spinTrainRate->setValue(1000.0f / (float)model->getInterval());
    setWindowModified(false);
    setTitle(filename);
    connect(ui->tableTriggers->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(documentWasModified()));
    curFile = filename;
    settings.setValue("mainWindow/LastOpenPath", QFileInfo(file).absolutePath());
}

void eTriggerMain::on_actionSave_triggered()
{
    saveProtocol(curFile);
}
void eTriggerMain::closeEvent(QCloseEvent *event)
 {
     if (trySaveProtocol()) {
         event->accept();
     } else {
         event->ignore();
     }
 }
bool eTriggerMain::trySaveProtocol()
{
    if (isWindowModified())
    {
        int res = QMessageBox::question(this, tr("Current file modified"), tr("The current file has been modified, do you want to save it ?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);

        if (res == QMessageBox::Cancel)
            return false;
        else if (res == QMessageBox::Save)
            return saveProtocol(curFile);
    }
    return true;
}

bool eTriggerMain::saveProtocol(QString fileName)
{
    TriggerModel *model = static_cast<TriggerModel *>(ui->tableTriggers->model());

    QString xml = model->getModelXml();
    QSettings settings;
    if (fileName.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(this,
            tr("Save protocol"),
            settings.value("mainWindow/LastOpenPath", QDir::homePath()).toString(),
            tr("Xml Files (*.xml);; All Files (*.*)"));
    }
    if (fileName.isEmpty()) return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Save file error", tr("AN error occurred while saving the file !"));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << xml;
    setWindowModified(false);
    setTitle(fileName);
    curFile = fileName;
    settings.setValue("mainWindow/LastOpenPath", QFileInfo(file).absolutePath());
    return true;
}

void eTriggerMain::on_pushStop_clicked()
{
    port->write("A");
    ui->lcdCounter->display(0);
}

void eTriggerMain::on_spinNumTrains_valueChanged(int count)
{
     TriggerModel *model = static_cast<TriggerModel *>(ui->tableTriggers->model());
     model->setCount(count);
}

void eTriggerMain::on_spinTrainRate_valueChanged(double freq)
{
    int tr = (int) (1000.0f / freq);
    TriggerModel *model = static_cast<TriggerModel *>(ui->tableTriggers->model());
    model->setInterval(tr);
}

void eTriggerMain::on_actionNew_triggered()
{
    TriggerModel *model = new TriggerModel();

    if (!trySaveProtocol()) return;
    ui->tableTriggers->setModel(model);
    ui->tableTriggers->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->spinNumTrains->setValue(model->getCount());
    ui->spinTrainRate->setValue(1000.0f / (float)model->getInterval());
    setWindowModified(true);
    setTitle("new prototocol");
    curFile.clear();
}

void eTriggerMain::on_actionAbout_triggered()
{
   QMessageBox::about(this, tr("About %1").arg(APP_NAME),
        tr("<b>%1</b> v%2 - An application designed to send TTL triggers "
           "to external devices by means of a USB device with high precision timing.").arg(APP_NAME).arg(APP_VERSION));
}

void eTriggerMain::on_actionExit_triggered()
{
    close();
}

void eTriggerMain::on_actionSave_As_triggered()
{
    curFile.clear();
    saveProtocol(curFile);
}

void eTriggerMain::on_actionSerial_Port_2_triggered()
{
    if (port)
    {
        port->close();
        delete port;
        port = 0;
    }
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    qDebug() << "List of ports:";
    bool found = false;
    for (int i = 0; i < ports.size(); i++) {
        qDebug() << "port name:" << ports.at(i).portName;
        qDebug() << "friendly name:" << ports.at(i).friendName;
        qDebug() << "physical name:" << ports.at(i).physName;
        qDebug() << "enumerator name:" << ports.at(i).enumName;
        qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
        qDebug() << "===================================";
        if (ports.at(i).friendName == "FT232R USB UART" || //Mac OS X driver name
            ports.at(i).friendName.contains("USB Serial Port")) //Windows driver name
        {
            found = true;
            this->port = new QextSerialPort(ports.at(i).portName, QextSerialPort::EventDriven);
            port->setBaudRate(BAUD9600);
            port->setFlowControl(FLOW_OFF);
            port->setParity(PAR_NONE);
            port->setDataBits(DATA_8);
            port->setStopBits(STOP_2);
            port->setTimeout(500);
            port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
            connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            statusBar()->showMessage(tr("Usb connection successful."));
            this->ui->pushStart->setEnabled(true);
            this->ui->pushStop->setEnabled(true);
        }


    }
    if (!found)
    {
        QMessageBox::critical(this, "USB Connection Error", tr("Cannot open the USB connection !"));
        this->ui->pushStart->setEnabled(false);
        this->ui->pushStop->setEnabled(false);
    }

}
