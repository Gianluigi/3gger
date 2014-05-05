#ifndef ETRIGGERMAIN_H
#define ETRIGGERMAIN_H

#include <QtWidgets/QMainWindow>
#include <qextserialenumerator.h>
#include <qextserialport.h>
#include <QList>


namespace Ui {
    class eTriggerMain;
}

class eTriggerMain : public QMainWindow {
    Q_OBJECT
public:
    eTriggerMain(QWidget *parent = 0);
    ~eTriggerMain();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::eTriggerMain *ui;
    QextSerialPort *port;
    QString curFile;
    bool saveProtocol(QString filename);
    bool trySaveProtocol();
    void setTitle(QString title);

private slots:
    void on_actionSerial_Port_2_triggered();
    void on_actionSave_As_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionNew_triggered();
    void on_spinTrainRate_valueChanged(double );
    void on_spinNumTrains_valueChanged(int );
    void on_pushStop_clicked();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();
    void on_pushStart_clicked();
    void on_pushRemove_clicked();
    void on_pushAdd_clicked();
    void onReadyRead();
    void documentWasModified();
};

#endif // ETRIGGERMAIN_H
