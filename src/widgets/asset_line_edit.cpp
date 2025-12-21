#include "asset_line_edit.h"

AssetLineEdit::AssetLineEdit(QWidget* parent) : QLineEdit(parent) {
}

AssetLineEdit::AssetLineEdit(const QString& contents, QWidget* parent) : QLineEdit(contents, parent) {
}

void AssetLineEdit::showEvent(QShowEvent* event) {
	(void)event;

	size_t dotPosition = text().toStdString().find('.');
	if (dotPosition != std::string::npos) {
		deselect();
		setSelection(0, static_cast<int>(dotPosition));
	}
}
