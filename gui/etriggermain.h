#ifndef ETRIGGERMAIN_H
#define ETRIGGERMAIN_H

#include <QMainWindow>

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

private:
    Ui::eTriggerMain *ui;

private slots:
    void on_pushRemove_clicked();
    void on_pushAdd_clicked();
};

#endif // ETRIGGERMAIN_H
