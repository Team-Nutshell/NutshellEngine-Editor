#include "shader_file_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../undo_commands/select_asset_entities_command.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <fstream>

ShaderFileWidget::ShaderFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_shader_file"))));
	layout()->addWidget(new SeparatorLine());
	codeEditorWidget = new CodeEditorWidget();
	layout()->addWidget(codeEditorWidget);
	compileButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("assets_shader_compile")));
	layout()->addWidget(compileButton);

	connect(codeEditorWidget, &CodeEditorWidget::valueChanged, this, &ShaderFileWidget::onValueChanged);
	connect(compileButton, &QPushButton::clicked, this, &ShaderFileWidget::onCompileButtonClicked);
}

void ShaderFileWidget::setPath(const std::string& path) {
	m_shaderFilePath = path;

	std::fstream textFile(path, std::ios::in);
	text = std::string((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());
	
	updateWidgets();
}

std::string ShaderFileWidget::getPath() {
	return m_shaderFilePath;
}

void ShaderFileWidget::updateWidgets() {
	if (codeEditorWidget->toPlainText().toStdString() != text) {
		int previousScrollBarPosition = codeEditorWidget->verticalScrollBar()->sliderPosition();
		codeEditorWidget->setText(QString::fromStdString(text));
		codeEditorWidget->verticalScrollBar()->setSliderPosition(previousScrollBarPosition);
	}
}

void ShaderFileWidget::save() {
	std::fstream textFile(m_shaderFilePath, std::ios::out | std::ios::trunc);
	textFile << codeEditorWidget->toPlainText().toStdString();
	textFile.close();
}

void ShaderFileWidget::onValueChanged() {
	m_globalInfo.actionUndoStack->push(new ChangeShaderFile(m_globalInfo, codeEditorWidget->toPlainText().toStdString(), m_shaderFilePath));
}

void ShaderFileWidget::onCompileButtonClicked() {
	std::string shaderPath = AssetHelper::absoluteToRelative(m_shaderFilePath, m_globalInfo.projectDirectory);
	m_globalInfo.rendererResourceManager.loadFragmentShader(m_shaderFilePath, shaderPath);
}

ChangeShaderFile::ChangeShaderFile(GlobalInfo& globalInfo, std::string newText, const std::string& filePath) : m_globalInfo(globalInfo) {
	setText(QString::fromStdString(m_globalInfo.localization.getString("undo_change_text_file", { AssetHelper::absoluteToRelative(filePath, m_globalInfo.projectDirectory) })));

	m_shaderFileWidget = m_globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->shaderFileWidget;
	m_oldText = m_shaderFileWidget->text;
	m_newText = newText;
	m_filePath = filePath;
}

void ChangeShaderFile::undo() {
	if (!m_shaderFileWidget->isVisible() || (m_shaderFileWidget->getPath() != m_filePath)) {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));
	}

	m_shaderFileWidget->text = m_oldText;
	m_shaderFileWidget->updateWidgets();

	m_shaderFileWidget->save();
}

void ChangeShaderFile::redo() {
	if (!m_shaderFileWidget->isVisible() || (m_shaderFileWidget->getPath() != m_filePath)) {
		m_globalInfo.selectionUndoStack->push(new SelectAssetEntitiesCommand(m_globalInfo, SelectionType::Asset, m_filePath, NO_ENTITY, {}));
	}

	m_shaderFileWidget->text = m_newText;
	m_shaderFileWidget->updateWidgets();

	m_shaderFileWidget->save();
}
