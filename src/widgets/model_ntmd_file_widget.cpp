#include "model_ntmd_file_widget.h"
#include "model_ntmd_primitive_widget.h"
#include "../../external/nlohmann/json.hpp"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <fstream>

ModelNtmdFileWidget::ModelNtmdFileWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(2, 0, 2, 0);
	layout()->addWidget(new QLabel(QString::fromStdString(m_globalInfo.localization.getString("assets_model_primitives"))));
	primitivesWidget = new QWidget();
	primitivesWidget->setLayout(new QVBoxLayout());
	primitivesWidget->layout()->setContentsMargins(0, 0, 0, 0);
	primitivesWidget->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	QScrollArea* scrollPrimitivesWidget = new QScrollArea();
	scrollPrimitivesWidget->setWidgetResizable(true);
	scrollPrimitivesWidget->setWidget(primitivesWidget);
	layout()->addWidget(scrollPrimitivesWidget);
	addPrimitiveButton = new QPushButton(QString::fromStdString(m_globalInfo.localization.getString("assets_model_add_primitive")));
	layout()->addWidget(addPrimitiveButton);

	connect(addPrimitiveButton, &QPushButton::clicked, this, &ModelNtmdFileWidget::onAddPrimitiveButtonClicked);
}

void ModelNtmdFileWidget::setPath(const std::string& path) {
	m_modelFilePath = path;
	std::fstream modelFile(m_modelFilePath, std::ios::in);
	if (modelFile.is_open()) {
		if (!nlohmann::json::accept(modelFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { m_modelFilePath }));
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_cannot_be_opened", { m_modelFilePath }));
		return;
	}

	while (QLayoutItem* item = primitivesWidget->layout()->takeAt(0)) {
		delete item->widget();
		delete item;
	}

	modelFile = std::fstream(m_modelFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(modelFile);

	if (j.contains("primitives")) {
		for (const auto& primitive : j["primitives"]) {
			ModelNtmdPrimitiveWidget* newPrimitiveWidget = new ModelNtmdPrimitiveWidget(m_globalInfo);
			if (primitive.contains("meshPath")) {
				newPrimitiveWidget->primitiveMeshWidget->setPath(primitive["meshPath"]);
			}
			if (primitive.contains("materialPath")) {
				newPrimitiveWidget->primitiveMaterialWidget->setPath(primitive["materialPath"]);
			}
			primitivesWidget->layout()->addWidget(newPrimitiveWidget);

			connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::valueChanged, this, &ModelNtmdFileWidget::onValueChanged);
			connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::removePrimitive, this, &ModelNtmdFileWidget::onRemovePrimitiveButtonClicked);
		}
	}
}

void ModelNtmdFileWidget::save() {
	nlohmann::json j;
	for (int i = 0; i < primitivesWidget->layout()->count(); i++) {
		ModelNtmdPrimitiveWidget* primitiveWidget = static_cast<ModelNtmdPrimitiveWidget*>(primitivesWidget->layout()->itemAt(i)->widget());
		if ((!primitiveWidget->primitiveMeshWidget->filePathButton->path.empty()) || (!primitiveWidget->primitiveMaterialWidget->filePathButton->path.empty()))
			j["primitives"].push_back(primitiveWidget->toJson());
	}

	std::fstream optionsFile(m_modelFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}
}

void ModelNtmdFileWidget::onAddPrimitiveButtonClicked() {
	ModelNtmdPrimitiveWidget* newPrimitiveWidget = new ModelNtmdPrimitiveWidget(m_globalInfo);
	primitivesWidget->layout()->addWidget(newPrimitiveWidget);

	connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::valueChanged, this, &ModelNtmdFileWidget::onValueChanged);
	connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::removePrimitive, this, &ModelNtmdFileWidget::onRemovePrimitiveButtonClicked);

	save();
}

void ModelNtmdFileWidget::onRemovePrimitiveButtonClicked() {
	QWidget* senderWidget = static_cast<QWidget*>(sender());

	primitivesWidget->layout()->takeAt(primitivesWidget->layout()->indexOf(senderWidget));
	delete senderWidget;

	save();
}

void ModelNtmdFileWidget::onValueChanged() {
	save();
}