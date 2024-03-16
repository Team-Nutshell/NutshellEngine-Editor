#include "renderable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../undo_commands/change_entity_component_command.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>

RenderableComponentWidget::RenderableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Renderable"));
	modelPathWidget = new FileSelectorWidget(m_globalInfo, "No model selected", "Select a model");
	layout()->addWidget(modelPathWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(modelPathWidget, &FileSelectorWidget::fileSelected, this, &RenderableComponentWidget::onStringUpdated);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &RenderableComponentWidget::onSelectEntity);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRenderableSignal, this, &RenderableComponentWidget::onAddEntityRenderable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRenderableSignal, this, &RenderableComponentWidget::onRemoveEntityRenderable);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRenderableSignal, this, &RenderableComponentWidget::onChangeEntityRenderable);
}

void RenderableComponentWidget::updateWidgets(const Renderable& renderable) {
	if (renderable.modelPath != "") {
		std::string modelPath = renderable.modelPath;
		std::replace(modelPath.begin(), modelPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(modelPath).is_absolute()) {
				if (modelPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					modelPath = modelPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		modelPathWidget->filePathLabel->setText(QString::fromStdString(modelPath.substr(modelPath.rfind('/') + 1)));
		modelPathWidget->filePathLabel->setToolTip(QString::fromStdString(modelPath));
	}
	else {
		modelPathWidget->filePathLabel->setText("No model selected");
		modelPathWidget->filePathLabel->setToolTip("");
	}
}

void RenderableComponentWidget::onSelectEntity() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value());
	}
	else {
		hide();
	}
}

void RenderableComponentWidget::onAddEntityRenderable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		Renderable renderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();
		updateWidgets(renderable);
		show();
	}
}

void RenderableComponentWidget::onRemoveEntityRenderable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}
}

void RenderableComponentWidget::onChangeEntityRenderable(EntityID entityID, const Renderable& renderable) {
	if (sender() != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(renderable);
		}
	}
}

void RenderableComponentWidget::onStringUpdated(const std::string& string) {
	Renderable newRenderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();
	if (sender() == modelPathWidget) {
		std::string fullModelPath = string;
		newRenderable.modelPath = fullModelPath;
		std::replace(newRenderable.modelPath.begin(), newRenderable.modelPath.end(), '\\', '/');
		if (newRenderable.modelPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
			newRenderable.modelPath = newRenderable.modelPath.substr(m_globalInfo.projectDirectory.size() + 1);
		}
		m_globalInfo.rendererResourceManager.loadModel(fullModelPath, newRenderable.modelPath);
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Renderable", &newRenderable));
}
