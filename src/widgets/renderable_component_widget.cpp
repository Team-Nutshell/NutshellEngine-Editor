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
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_renderable")));
	modelPathWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("component_renderable_model"), m_globalInfo.localization.getString("component_renderable_no_model_selected"), m_globalInfo.projectDirectory + "/assets");
	layout()->addWidget(modelPathWidget);
	std::vector<std::string> primitiveIndexElements = { m_globalInfo.localization.getString("component_renderable_no_primitive_index") };
	primitiveIndexWidget = new ComboBoxWidget(m_globalInfo, m_globalInfo.localization.getString("component_renderable_primitive_index"), primitiveIndexElements);
	layout()->addWidget(primitiveIndexWidget);
	materialPathWidget = new FileSelectorWidget(m_globalInfo, m_globalInfo.localization.getString("component_renderable_material"), m_globalInfo.localization.getString("component_renderable_no_material_selected"), m_globalInfo.projectDirectory + "/assets");
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
	if (!renderable.modelPath.empty()) {
		std::string modelPath = AssetHelper::absoluteToRelative(renderable.modelPath, m_globalInfo.projectDirectory);
		modelPathWidget->setPath(modelPath);

		{
			const QSignalBlocker signalBlocker(primitiveIndexWidget->comboBox);
			primitiveIndexWidget->comboBox->clear();
			QStringList primitiveIndexes;
			primitiveIndexes.append(QString::fromStdString(m_globalInfo.localization.getString("component_renderable_no_primitive_index")));
			uint32_t primitiveCount = 0;
			if (m_globalInfo.rendererResourceManager.models.find(renderable.modelPath) != m_globalInfo.rendererResourceManager.models.end()) {
				RendererResourceManager::Model& model = m_globalInfo.rendererResourceManager.models[renderable.modelPath];
				for (size_t i = 0; i < model.primitives.size(); i++) {
					std::string primitiveIndexName = std::to_string(i);
					if (!model.primitives[i].name.empty()) {
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
				primitiveIndexWidget->comboBox->setCurrentIndex(primitiveIndexWidget->comboBox->findText(QString::fromStdString(m_globalInfo.localization.getString("component_renderable_no_primitive_index"))));
			}
		}
	}
	else {
		modelPathWidget->setPath("");

		{
			const QSignalBlocker signalBlocker(primitiveIndexWidget->comboBox);
			primitiveIndexWidget->comboBox->clear();
			primitiveIndexWidget->comboBox->addItem(QString::fromStdString(m_globalInfo.localization.getString("component_renderable_no_primitive_index")));
		}
	}

	if (!renderable.materialPath.empty()) {
		std::string materialPath = AssetHelper::absoluteToRelative(renderable.materialPath, m_globalInfo.projectDirectory);
		materialPathWidget->setPath(materialPath);
	}
	else {
		materialPathWidget->setPath("");
	}
}

void RenderableComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Renderable>& renderables) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < renderables.size(); i++) {
		componentPointers.push_back(&renderables[i]);
	}

	m_globalInfo.actionUndoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "Renderable", componentPointers));
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
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Renderable> newRenderables;
	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].renderable) {
			Renderable newRenderable = m_globalInfo.entities[selectedEntityID].renderable.value();

			if (senderWidget == modelPathWidget) {
				std::string fullModelPath = path;
				newRenderable.modelPath = AssetHelper::absoluteToRelative(fullModelPath, m_globalInfo.projectDirectory);
				if (!fullModelPath.empty()) {
					m_globalInfo.rendererResourceManager.loadModel(fullModelPath, newRenderable.modelPath);
					if (newRenderable.modelPath != m_globalInfo.entities[selectedEntityID].renderable->modelPath) {
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

			entityIDs.push_back(selectedEntityID);
			newRenderables.push_back(newRenderable);
		}
	}

	updateComponents(entityIDs, newRenderables);
}

void RenderableComponentWidget::onElementChanged(const std::string& element) {
	uint primitiveIndex = NTSHENGN_NO_MODEL_PRIMITIVE;
	if (element != m_globalInfo.localization.getString("component_renderable_no_primitive_index")) {
		size_t spacePos = element.find(' ');
		if (spacePos != std::string::npos) {
			primitiveIndex = static_cast<uint32_t>(std::atoi(element.substr(0, spacePos).c_str()));
		}
		else {
			primitiveIndex = static_cast<uint32_t>(std::atoi(element.c_str()));
		}
	}

	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<Renderable> newRenderables;

	std::set<EntityID> selectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	selectedEntityIDs.insert(m_globalInfo.currentEntityID);
	for (EntityID selectedEntityID : selectedEntityIDs) {
		if (m_globalInfo.entities[selectedEntityID].renderable) {
			Renderable newRenderable = m_globalInfo.entities[selectedEntityID].renderable.value();

			if (senderWidget == primitiveIndexWidget) {
				newRenderable.primitiveIndex = primitiveIndex;
			}

			entityIDs.push_back(selectedEntityID);
			newRenderables.push_back(newRenderable);
		}
	}

	updateComponents(entityIDs, newRenderables);
}
