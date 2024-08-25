#include "renderable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/asset_helper.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include "../renderer/collider_mesh.h"
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <vector>
#include <string>
#include <cstdlib>

RenderableComponentWidget::RenderableComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, "Renderable"));
	modelPathWidget = new FileSelectorWidget(m_globalInfo, "Model", "No model selected", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(modelPathWidget);
	std::vector<std::string> primitiveIndexElements = { "No primitive index" };
	primitiveIndexWidget = new ComboBoxWidget(m_globalInfo, "Primitive Index", primitiveIndexElements);
	layout()->addWidget(primitiveIndexWidget);
	materialPathWidget = new FileSelectorWidget(m_globalInfo, "Material", "No material selected", m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(materialPathWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(modelPathWidget, &FileSelectorWidget::fileSelected, this, &RenderableComponentWidget::onPathChanged);
	connect(primitiveIndexWidget, &ComboBoxWidget::elementSelected, this, &RenderableComponentWidget::onElementChanged);
	connect(materialPathWidget, &FileSelectorWidget::fileSelected, this, &RenderableComponentWidget::onPathChanged);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &RenderableComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntityRenderableSignal, this, &RenderableComponentWidget::onEntityRenderableAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntityRenderableSignal, this, &RenderableComponentWidget::onEntityRenderableRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntityRenderableSignal, this, &RenderableComponentWidget::onEntityRenderableChanged);
}

void RenderableComponentWidget::updateWidgets(const Renderable& renderable) {
	if (renderable.modelPath != "") {
		std::string modelPath = AssetHelper::absoluteToRelative(renderable.modelPath, m_globalInfo.projectDirectory);
		modelPathWidget->setPath(modelPath);

		{
			const QSignalBlocker signalBlocker(primitiveIndexWidget->comboBox);
			primitiveIndexWidget->comboBox->clear();
			QStringList primitiveIndexes;
			primitiveIndexes.append("No primitive index");
			uint32_t primitiveCount = 0;
			if (m_globalInfo.rendererResourceManager.models.find(renderable.modelPath) != m_globalInfo.rendererResourceManager.models.end()) {
				RendererResourceManager::Model& model = m_globalInfo.rendererResourceManager.models[renderable.modelPath];
				for (size_t i = 0; i < model.primitives.size(); i++) {
					std::string primitiveIndexName = std::to_string(i);
					if (model.primitives[i].name != "") {
						primitiveIndexName += " (" + model.primitives[i].name + ")";
					}
					primitiveIndexes.append(QString::fromStdString(primitiveIndexName));
				}
				primitiveCount = static_cast<uint32_t>(model.primitives.size());
			}
			primitiveIndexWidget->comboBox->addItems(primitiveIndexes);
			if ((renderable.primitiveIndex != NTSHENGN_NO_MODEL_PRIMITIVE) && (renderable.primitiveIndex < primitiveCount)) {
				primitiveIndexWidget->comboBox->setCurrentIndex(renderable.primitiveIndex + 1);
			}
			else {
				primitiveIndexWidget->comboBox->setCurrentIndex(primitiveIndexWidget->comboBox->findText("No primitive index"));
			}
		}
	}
	else {
		modelPathWidget->setPath("");

		{
			const QSignalBlocker signalBlocker(primitiveIndexWidget->comboBox);
			primitiveIndexWidget->comboBox->clear();
			primitiveIndexWidget->comboBox->addItem("No primitive index");
		}
	}

	if (renderable.materialPath != "") {
		std::string materialPath = AssetHelper::absoluteToRelative(renderable.materialPath, m_globalInfo.projectDirectory);
		materialPathWidget->setPath(materialPath);
	}
	else {
		materialPathWidget->setPath("");
	}
}

void RenderableComponentWidget::updateComponent(EntityID entityID, Component* component) {
	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, { entityID }, "Renderable", { component }));
}

void RenderableComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value());
	}
	else {
		hide();
	}
}

void RenderableComponentWidget::onEntityRenderableAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const Renderable& renderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();
		updateWidgets(renderable);
		show();
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RenderableComponentWidget::onEntityRenderableRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RenderableComponentWidget::onEntityRenderableChanged(EntityID entityID, const Renderable& renderable) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(renderable);
		}
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void RenderableComponentWidget::onPathChanged(const std::string& path) {
	Renderable newRenderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();

	QObject* senderWidget = sender();
	if (senderWidget == modelPathWidget) {
		std::string fullModelPath = path;
		newRenderable.modelPath = AssetHelper::absoluteToRelative(fullModelPath, m_globalInfo.projectDirectory);
		if (!fullModelPath.empty()) {
			m_globalInfo.rendererResourceManager.loadModel(fullModelPath, newRenderable.modelPath);
			if (newRenderable.modelPath != m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath) {
				newRenderable.primitiveIndex = 0;
			}
		}
	}
	else if (senderWidget == materialPathWidget) {
		std::string fullMaterialPath = path;
		newRenderable.materialPath = AssetHelper::absoluteToRelative(fullMaterialPath, m_globalInfo.projectDirectory);
		if (!fullMaterialPath.empty()) {
			m_globalInfo.rendererResourceManager.loadMaterial(fullMaterialPath, newRenderable.materialPath);
		}
	}
	updateComponent(m_globalInfo.currentEntityID, &newRenderable);
}

void RenderableComponentWidget::onElementChanged(const std::string& element) {
	uint primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;
	if (element != "No primitive index") {
		size_t spacePos = element.find(' ');
		if (spacePos != std::string::npos) {
			primitiveIndex = static_cast<uint32_t>(std::atoi(element.substr(0, spacePos).c_str()));
		}
		else {
			primitiveIndex = static_cast<uint32_t>(std::atoi(element.c_str()));
		}
	}

	Renderable newRenderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();
	QObject* senderWidget = sender();
	if (senderWidget == primitiveIndexWidget) {
		newRenderable.primitiveIndex = primitiveIndex;
	}
	updateComponent(m_globalInfo.currentEntityID, &newRenderable);
}
