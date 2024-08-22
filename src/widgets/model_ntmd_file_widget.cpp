#include "model_ntmd_file_widget.h"
#include "model_ntmd_primitive_widget.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <fstream>

ModelNtmdFileWidget::ModelNtmdFileWidget(GlobalInfo& globalInfo, const std::string& modelFilePath) : m_globalInfo(globalInfo), m_modelFilePath(modelFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Model File - " + QString::fromStdString(modelFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &ModelNtmdFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->addWidget(new QLabel("Primitives"));
	primitivesWidget = new QWidget();
	primitivesWidget->setLayout(new QVBoxLayout());
	primitivesWidget->layout()->setContentsMargins(0, 0, 0, 0);
	primitivesWidget->layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	QScrollArea* scrollPrimitivesWidget = new QScrollArea();
	scrollPrimitivesWidget->setWidgetResizable(true);
	scrollPrimitivesWidget->setWidget(primitivesWidget);
	layout()->addWidget(scrollPrimitivesWidget);
	addPrimitiveButton = new QPushButton("Add Primitive");
	layout()->addWidget(addPrimitiveButton);

	connect(addPrimitiveButton, &QPushButton::clicked, this, &ModelNtmdFileWidget::onAddPrimitiveButtonClicked);

	std::fstream optionsFile(modelFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + modelFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + modelFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(modelFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("primitives")) {
		for (const auto& primitive : j["primitives"]) {
			ModelNtmdPrimitiveWidget* newPrimitiveWidget = new ModelNtmdPrimitiveWidget(m_globalInfo);
			if (primitive.contains("meshPath")) {
				std::string meshPath = primitive["meshPath"];
				newPrimitiveWidget->primitiveMeshWidget->filePathButton->path = meshPath;
				newPrimitiveWidget->primitiveMeshWidget->filePathButton->setText(QString::fromStdString(meshPath.substr(meshPath.rfind('/') + 1)));
				newPrimitiveWidget->primitiveMeshWidget->filePathButton->setToolTip(QString::fromStdString(meshPath));
			}
			if (primitive.contains("materialPath")) {
				std::string materialPath = primitive["materialPath"];
				newPrimitiveWidget->primitiveMaterialWidget->filePathButton->path = materialPath;
				newPrimitiveWidget->primitiveMaterialWidget->filePathButton->setText(QString::fromStdString(materialPath.substr(materialPath.rfind('/') + 1)));
				newPrimitiveWidget->primitiveMaterialWidget->filePathButton->setToolTip(QString::fromStdString(materialPath));
			}
			primitivesWidget->layout()->addWidget(newPrimitiveWidget);

			connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::valueChanged, this, &ModelNtmdFileWidget::onValueChanged);
			connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::removePrimitive, this, &ModelNtmdFileWidget::onRemovePrimitiveButtonClicked);
		}
	}
}

void ModelNtmdFileWidget::onAddPrimitiveButtonClicked() {
	ModelNtmdPrimitiveWidget* newPrimitiveWidget = new ModelNtmdPrimitiveWidget(m_globalInfo);
	primitivesWidget->layout()->addWidget(newPrimitiveWidget);

	connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::valueChanged, this, &ModelNtmdFileWidget::onValueChanged);
	connect(newPrimitiveWidget, &ModelNtmdPrimitiveWidget::removePrimitive, this, &ModelNtmdFileWidget::onRemovePrimitiveButtonClicked);

	SaveTitleChanger::change(this);
}

void ModelNtmdFileWidget::onRemovePrimitiveButtonClicked() {
	QWidget* senderWidget = static_cast<QWidget*>(sender());

	primitivesWidget->layout()->takeAt(primitivesWidget->layout()->indexOf(senderWidget));
	delete senderWidget;

	SaveTitleChanger::change(this);
}

void ModelNtmdFileWidget::onValueChanged() {
	SaveTitleChanger::change(this);
}

void ModelNtmdFileWidget::save() {
	nlohmann::json j;
	for (int i = 0; i < primitivesWidget->layout()->count(); i++) {
		ModelNtmdPrimitiveWidget* primitiveWidget = static_cast<ModelNtmdPrimitiveWidget*>(primitivesWidget->layout()->itemAt(i)->widget());
		if ((primitiveWidget->primitiveMeshWidget->filePathButton->path != "") || (primitiveWidget->primitiveMaterialWidget->filePathButton->path != ""))
			j["primitives"].push_back(primitiveWidget->toJson());
	}

	std::fstream optionsFile(m_modelFilePath, std::ios::out | std::ios::trunc);
	if (j.empty()) {
		optionsFile << "{\n}";
	}
	else {
		optionsFile << j.dump(1, '\t');
	}

	SaveTitleChanger::reset(this);
}