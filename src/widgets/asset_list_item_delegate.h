#pragma once
#include <QStyledItemDelegate>

class AssetListItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};