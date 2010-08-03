#include <QDomElement>
#include <QDebug>
#include "triggermodel.h"

TriggerModel::TriggerModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    train_count = 1;
    train_interval = 1000;
}

int TriggerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return dataSource.size();
}

int TriggerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QVariant TriggerModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if (index.row() >= dataSource.size() || index.row() < 0)
         return QVariant();

     if (role == Qt::DisplayRole || role == Qt::EditRole) {
         TriggerRecord *row = dataSource.at(index.row());

         if (index.column() == 0)
             return row->name;
         /* else if (index.column() == 1)
             return row->o1;
         else if (index.column() == 2)
             return row->o2;
         else if (index.column() == 3)
             return row->o3;
         else if (index.column() == 4)
             return row->o4;
         else if (index.column() == 5)
             return row->o5;
         else if (index.column() == 6)
             return row->o6;
         else if (index.column() == 7)
             return row->o7;
         else if (index.column() == 8)
             return row->o8;
*/
         else if (index.column() == 9)
             return (double)row->length/10.0f;
         else
            return QVariant();
     }
     if ((role == Qt::CheckStateRole || role <=  Qt::EditRole)  && index.column() >= 1 && index.column() <= 8)
     {
         TriggerRecord *row = dataSource.at(index.row());
         if (index.column() == 1)
             return row->o1 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 2)
             return row->o2 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 3)
             return row->o3 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 4)
             return row->o4 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 5)
             return row->o5 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 6)
             return row->o6  ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 7)
             return row->o7 ? Qt::Checked : Qt::Unchecked;
         else if (index.column() == 8)
             return row->o8 ? Qt::Checked : Qt::Unchecked;
     }
     if (role == Qt::TextAlignmentRole && index.column() >= 1 && index.column()<= 8) return Qt::AlignCenter;

     return QVariant();
}

QVariant TriggerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Name");

            case 1:
                return tr("Out 1");
            case 2:
                return tr("Out 2");
            case 3:
                return tr("Out 3");
            case 4:
                return tr("Out 4");
            case 5:
                return tr("Out 5");
            case 6:
                return tr("Out 6");
            case 7:
                return tr("Out 7");
            case 8:
                return tr("Out 8");
            case 9:
                return tr("Length");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool TriggerModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        dataSource.insert(position, new TriggerRecord());
    }

    endInsertRows();
    return true;
}


bool TriggerModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; ++row) {
        dataSource.removeAt(position);
    }

    endRemoveRows();
    return true;
}

bool TriggerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
        if (index.isValid() && role == Qt::EditRole && (index.column() == 0 || index.column() == 9)) {
            int row = index.row();

            TriggerRecord *record = dataSource.value(row);

            if (index.column() == 0)
                record->name = value.toString();

            else if (index.column() == 9)
                record->length = value.toFloat() > 0.5f ? (int)(value.toFloat() * 10.0f) : 5;

            dataSource.replace(row, record);
            emit(dataChanged(index, index));

            return true;
        }
        if (index.isValid() && role == Qt::CheckStateRole && index.column() >= 1 && index.column() <= 8) {
            int row = index.row();

            TriggerRecord *record = dataSource.value(row);

            if (index.column() == 1)
                record->o1 = value.toBool();
            else if (index.column() == 2)
                record->o2 = value.toBool();
            else if (index.column() == 3)
                record->o3 = value.toBool();
            else if (index.column() == 4)
                record->o4 = value.toBool();
            else if (index.column() == 5)
                record->o5 = value.toBool();
            else if (index.column() == 6)
                record->o6 = value.toBool();
            else if (index.column() == 7)
                record->o7 = value.toBool();
            else if (index.column() == 8)
                record->o8 = value.toBool();

            dataSource.replace(row, record);
            emit(dataChanged(index, index));

            return true;
        }
        return false;
}

Qt::ItemFlags TriggerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() >= 1 && index.column() <= 8)
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    else
        return  Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
QString TriggerModel::getModelXml()
{
    QDomDocument doc("RtStim");
    QDomElement root = doc.createElement("rtstim");
    doc.appendChild(root);

    QDomElement setting = doc.createElement("settings");
    setting.setAttribute("tr", getInterval());
    setting.setAttribute("count", getCount());
    setting.setAttribute("size", 500);
    root.appendChild(setting);

    for (int i=0; i<dataSource.size(); i++)
    {
        TriggerRecord *r = dataSource.at(i);
        QDomElement tag = doc.createElement("stimulus");
        tag.setAttribute("name", r->name);
        tag.setAttribute("length", r->length);
        int code = r->o1 + r->o2 * 2 + r->o3 * 4 + r->o4 * 8 + r->o5 * 16 + r->o6 * 32 + r->o7 * 64 + r->o8 * 128;
        tag.setAttribute("code", code);

        root.appendChild(tag);
    }

    return doc.toString();
}

QString TriggerModel::getProgramString()
{
    QString output = "P";
    output.append(QString::number(dataSource.size()));
    output.append(".");

    for (int i=0; i<dataSource.size(); i++)
    {
        TriggerRecord *r = dataSource.at(i);

        int code = r->o1 + r->o2 * 2 + r->o3 * 4 + r->o4 * 8 + r->o5 * 16 + r->o6 * 32 + r->o7 * 64 + r->o8 * 128;
        output.append(QString::number(code));
        output.append(".");
        output.append(QString::number(r->length));
        output.append(".");
    }
    output.append(QString::number(getCount()));
    output.append(".");
    output.append(QString::number(getInterval()*10));
    output.append(".");

    return output;
}
void TriggerModel::fillModelFromXml(QString xml)
{
    
    QDomDocument doc("RtStim");

    doc.setContent(xml);
    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement root = doc.documentElement();
    
    QDomNode n = root.firstChild();
    dataSource.clear();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (e.tagName().toLower() == "settings")
            {
                setInterval(e.attribute("tr",   "1000").toInt());
                setCount(e.attribute("count", "1").toInt());
            }
            else if (e.tagName().toLower() == "stimulus")
            {
                QString name = e.attribute("name", "--- invalid attribute ---");
                int length = e.attribute("length", "0").toInt();
                int code = e.attribute("code", 0).toInt();

                if (name != "--- invalid attribute ---" && length > 0 && code > 0)
                {
                    TriggerRecord *r = new TriggerRecord();
                    r->name = name;
                    r->length = length;
                    r->o1 = code & 1;
                    r->o2 = code & 2;
                    r->o3 = code & 4;
                    r->o4 = code & 8;
                    r->o5 = code & 16;
                    r->o6 = code & 32;
                    r->o7 = code & 64;
                    r->o8 = code & 128;

                    dataSource.append(r);
                }
            }

            qDebug() << qPrintable(e.tagName()) << endl; // the node really is an element.
        }
        n = n.nextSibling();
    }


    dataChanged(createIndex(0,0), createIndex(dataSource.size()-1, 9));


}
