#include <QtGui>

#include "checkboxdelegate.h"

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{


}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex & index ) const
{
    QCheckBox *editor = new QCheckBox(parent);
    return editor;
}
void CheckBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
        bool value = index.model()->data(index, Qt::EditRole).toBool();

        QCheckBox *CheckBox = static_cast<QCheckBox*>(editor);
        CheckBox->setChecked(value);
 }
void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
        bool value = index.model()->data(index, Qt::EditRole).toBool();

        QCheckBox *CheckBox = static_cast<QCheckBox*>(editor);
        model->setData(index, CheckBox->isChecked(), Qt::EditRole);
 }

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


/*void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
 {
    QStyledItemDelegate::paint(painter, option, index);

 }
*/
