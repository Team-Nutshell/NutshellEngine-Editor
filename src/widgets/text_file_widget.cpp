#include "text_file_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include <QVBoxLayout>
#include <fstream>

TextFileWidget::TextFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_text_file"))));
	layout()->addWidget(new SeparatorLine());
	textEditWidget = new TextWidget();
	layout()->addWidget(textEditWidget);

	connect(textEditWidget, &TextWidget::valueChanged, this, &TextFileWidget::onValueChanged);
}

void TextFileWidget::setPath(const std::string& path) {
	m_textFilePath = path;

	std::fstream textFile(path, std::ios::in);
	text = std::string((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());
	
	updateWidgets();
}

void TextFileWidget::updateWidgets() {
	textEditWidget->setText(QString::fromStdString(text));
}

void TextFileWidget::save() {
	std::fstream textFile(m_textFilePath, std::ios::out | std::ios::trunc);
	textFile << textEditWidget->toPlainText().toStdString();
}

void TextFileWidget::onValueChanged() {
	m_globalInfo.actionUndoStack->push(new ChangeTextFile(m_globalInfo, textEditWidget->toPlainText().toStdString(), m_textFilePath));
}

ChangeTextFile::ChangeTextFile(GlobalInfo& globalInfo, std::string newText, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_text_file", { AssetHelper::absoluteToRelative(filePath, m_globalInfo.projectDirectory) })));

	m_textFileWidget = m_globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->textFileWidget;
	m_oldText = m_textFileWidget->text;
	m_newText = newText;
	m_filePath = filePath;
}

void ChangeTextFile::undo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_textFileWidget->text = m_oldText;
	m_textFileWidget->updateWidgets();

	m_textFileWidget->save();
}

void ChangeTextFile::redo() {
	m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));

	m_textFileWidget->text = m_newText;
	m_textFileWidget->updateWidgets();

	m_textFileWidget->save();
}
