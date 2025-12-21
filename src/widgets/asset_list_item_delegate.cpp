#include "asset_list_item_delegate.h"
#include "asset_line_edit.h"

QWidget* AssetListItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	(void)parent;
	(void)option;
	(void)index;

	return new AssetLineEdit(parent);
}
