#ifndef TRIGGERMODEL_H
#define TRIGGERMODEL_H

#include <QAbstractTableModel>

class TriggerModel : public QAbstractTableModel
{
Q_OBJECT
public:
    explicit TriggerModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QString getModelXml();
    QString getProgramString();
    void fillModelFromXml(QString xml);

    void setInterval(int interval) { train_interval = interval;};
    int getInterval() { return train_interval; };

    void setCount(int count) {train_count = count; };
    int getCount() { return train_count; }
signals:

public slots:

private:
    int train_interval;
    int train_count;
    //column count = num of outputs + name + length columns
    #define COLUMN_COUNT (10);
    typedef struct TriggerRecord {
        TriggerRecord() : name(tr("<undef>")), length(10), o1(false), o2(false), o3(false), o4(false),  o5(false), o6(false), o7(false), o8(false)
        {};
        QString name;
        bool o1;
        bool o2;
        bool o3;
        bool o4;
        bool o5;
        bool o6;
        bool o7;
        bool o8;
        int length;
    };

    QList<TriggerRecord *> dataSource;
};

#endif // TRIGGERMODEL_H
