#include "renderable_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entity_component_command.h"
#include "../renderer/collider_mesh.h"
#include "../widgets/main_window.h"
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
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(modelPathWidget, &FileSelectorWidget::fileSelected, this, &RenderableComponentWidget::onStringChanged);
	connect(primitiveIndexWidget, &ComboBoxWidget::elementSelected, this, &RenderableComponentWidget::onElementChanged);
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
		modelPathWidget->filePathButton->path = modelPath;
		modelPathWidget->filePathButton->setText(QString::fromStdString(modelPath.substr(modelPath.rfind('/') + 1)));
		modelPathWidget->filePathButton->setToolTip(QString::fromStdString(modelPath));

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
		modelPathWidget->filePathButton->path = "";
		modelPathWidget->filePathButton->setText("No model selected");
		modelPathWidget->filePathButton->setToolTip("");

		{
			const QSignalBlocker signalBlocker(primitiveIndexWidget->comboBox);
			primitiveIndexWidget->comboBox->clear();
			primitiveIndexWidget->comboBox->addItem("No primitive index");
		}
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
		const Renderable& renderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();
		updateWidgets(renderable);
		show();
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void RenderableComponentWidget::onRemoveEntityRenderable(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void RenderableComponentWidget::onChangeEntityRenderable(EntityID entityID, const Renderable& renderable) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(renderable);
		}
	}

	ColliderMesh::update(m_globalInfo, entityID);

	SaveTitleChanger::change(reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow));
}

void RenderableComponentWidget::onStringChanged(const std::string& string) {
	Renderable newRenderable = m_globalInfo.entities[m_globalInfo.currentEntityID].renderable.value();

	QObject* senderWidget = sender();
	if (senderWidget == modelPathWidget) {
		std::string fullModelPath = string;
		newRenderable.modelPath = fullModelPath;
		if (!fullModelPath.empty()) {
			std::replace(newRenderable.modelPath.begin(), newRenderable.modelPath.end(), '\\', '/');
			if (newRenderable.modelPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
				newRenderable.modelPath = newRenderable.modelPath.substr(m_globalInfo.projectDirectory.size() + 1);
			}
			m_globalInfo.rendererResourceManager.loadModel(fullModelPath, newRenderable.modelPath);
			if (newRenderable.modelPath != m_globalInfo.entities[m_globalInfo.currentEntityID].renderable->modelPath) {
				newRenderable.primitiveIndex = 0;
			}
		}
	}
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Renderable", &newRenderable));
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
	m_globalInfo.undoStack->push(new ChangeEntityComponentCommand(m_globalInfo, m_globalInfo.currentEntityID, "Renderable", &newRenderable));
}
