#include "material_file_widget.h"
#include "separator_line.h"
#include "../common/save_title_changer.h"
#include "../../external/nlohmann/json.hpp"
#include <QVBoxLayout>
#include <fstream>

MaterialFileWidget::MaterialFileWidget(GlobalInfo& globalInfo, const std::string& materialFilePath) : m_globalInfo(globalInfo), m_materialFilePath(materialFilePath) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Material File - " + QString::fromStdString(materialFilePath));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_menuBar = new QMenuBar(this);
	m_fileMenu = m_menuBar->addMenu("File");
	m_fileSaveAction = m_fileMenu->addAction("Save", this, &MaterialFileWidget::save);
	m_fileSaveAction->setShortcut(QKeySequence::fromString("Ctrl+S"));

	setLayout(new QVBoxLayout());
	QMargins contentMargins = layout()->contentsMargins();
	contentMargins.setTop(contentMargins.top() + 10);
	layout()->setContentsMargins(contentMargins);
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	diffuseTextureImageWidget = new FileSelectorWidget(globalInfo, "Diffuse Image", "No image selected", "");
	layout()->addWidget(diffuseTextureImageWidget);
	diffuseTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Diffuse Image Sampler", "No image sampler selected", "");
	layout()->addWidget(diffuseTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	normalTextureImageWidget = new FileSelectorWidget(globalInfo, "Normal Image", "No image selected", "");
	layout()->addWidget(normalTextureImageWidget);
	normalTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Normal Image Sampler", "No image sampler selected", "");
	layout()->addWidget(normalTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	metalnessTextureImageWidget = new FileSelectorWidget(globalInfo, "Metalness Image", "No image selected", "");
	layout()->addWidget(metalnessTextureImageWidget);
	metalnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Metalness Image Sampler", "No image sampler selected", "");
	layout()->addWidget(metalnessTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	roughnessTextureImageWidget = new FileSelectorWidget(globalInfo, "Roughness Image", "No image selected", "");
	layout()->addWidget(roughnessTextureImageWidget);
	roughnessTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Roughness Image Sampler", "No image sampler selected", "");
	layout()->addWidget(roughnessTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	occlusionTextureImageWidget = new FileSelectorWidget(globalInfo, "Occlusion Image", "No image selected", "");
	layout()->addWidget(occlusionTextureImageWidget);
	occlusionTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Occlusion Image Sampler", "No image sampler selected", "");
	layout()->addWidget(occlusionTextureImageSamplerWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	emissiveTextureImageWidget = new FileSelectorWidget(globalInfo, "Emissive Image", "No image selected", "");
	layout()->addWidget(emissiveTextureImageWidget);
	emissiveTextureImageSamplerWidget = new FileSelectorWidget(globalInfo, "Emissive Image Sampler", "No image sampler selected", "");
	layout()->addWidget(emissiveTextureImageSamplerWidget);
	emissiveFactorWidget = new ScalarWidget(globalInfo, "Emissive Factor");
	layout()->addWidget(emissiveFactorWidget);
	layout()->addWidget(new SeparatorLine(globalInfo));
	alphaCutoffWidget = new ScalarWidget(globalInfo, "Alpha Cutoff");
	layout()->addWidget(alphaCutoffWidget);
	indexOfRefractionWidget = new ScalarWidget(globalInfo, "Index of Refraction");
	layout()->addWidget(indexOfRefractionWidget);

	std::fstream optionsFile(materialFilePath, std::ios::in);
	if (optionsFile.is_open()) {
		if (!nlohmann::json::accept(optionsFile)) {
			m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + materialFilePath + "\" is not a valid JSON file.");
			return;
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, "\"" + materialFilePath + "\" cannot be opened.");
		return;
	}

	optionsFile = std::fstream(materialFilePath, std::ios::in);
	nlohmann::json j = nlohmann::json::parse(optionsFile);

	if (j.contains("diffuseTexture")) {
		if (j["diffuseTexture"].contains("imagePath")) {
			std::string imagePath = j["diffuseTexture"]["imagePath"];
			diffuseTextureImageWidget->filePathButton->path = imagePath;
			diffuseTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			diffuseTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["diffuseTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["diffuseTexture"]["imageSamplerPath"];
			diffuseTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			diffuseTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			diffuseTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("normalTexture")) {
		if (j["normalTexture"].contains("imagePath")) {
			std::string imagePath = j["normalTexture"]["imagePath"];
			normalTextureImageWidget->filePathButton->path = imagePath;
			normalTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			normalTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["normalTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["normalTexture"]["imageSamplerPath"];
			normalTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			normalTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			normalTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("metalnessTexture")) {
		if (j["metalnessTexture"].contains("imagePath")) {
			std::string imagePath = j["metalnessTexture"]["imagePath"];
			metalnessTextureImageWidget->filePathButton->path = imagePath;
			metalnessTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			metalnessTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["metalnessTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["metalnessTexture"]["imageSamplerPath"];
			metalnessTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			metalnessTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			metalnessTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("roughnessTexture")) {
		if (j["roughnessTexture"].contains("imagePath")) {
			std::string imagePath = j["roughnessTexture"]["imagePath"];
			roughnessTextureImageWidget->filePathButton->path = imagePath;
			roughnessTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			roughnessTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["roughnessTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["roughnessTexture"]["imageSamplerPath"];
			roughnessTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			roughnessTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			roughnessTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("occlusionTexture")) {
		if (j["occlusionTexture"].contains("imagePath")) {
			std::string imagePath = j["occlusionTexture"]["imagePath"];
			occlusionTextureImageWidget->filePathButton->path = imagePath;
			occlusionTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			occlusionTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["occlusionTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["occlusionTexture"]["imageSamplerPath"];
			occlusionTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			occlusionTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			occlusionTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("emissiveTexture")) {
		if (j["emissiveTexture"].contains("imagePath")) {
			std::string imagePath = j["emissiveTexture"]["imagePath"];
			emissiveTextureImageWidget->filePathButton->path = imagePath;
			emissiveTextureImageWidget->filePathButton->setText(QString::fromStdString(imagePath.substr(imagePath.rfind('/') + 1)));
			emissiveTextureImageWidget->filePathButton->setToolTip(QString::fromStdString(imagePath));
		}
		if (j["emissiveTexture"].contains("imageSamplerPath")) {
			std::string imageSamplerPath = j["emissiveTexture"]["imageSamplerPath"];
			emissiveTextureImageSamplerWidget->filePathButton->path = imageSamplerPath;
			emissiveTextureImageSamplerWidget->filePathButton->setText(QString::fromStdString(imageSamplerPath.substr(imageSamplerPath.rfind('/') + 1)));
			emissiveTextureImageSamplerWidget->filePathButton->setToolTip(QString::fromStdString(imageSamplerPath));
		}
	}
	if (j.contains("emissiveFactor")) {
		float emissiveFactor = j["emissiveFactor"];
		emissiveFactorWidget->value = emissiveFactor;
		emissiveFactorWidget->valueLineEdit->setText(QString::number(emissiveFactor));
	}
	if (j.contains("alphaCutoff")) {
		float alphaCutoff = j["alphaCutoff"];
		alphaCutoffWidget->value = alphaCutoff;
		alphaCutoffWidget->valueLineEdit->setText(QString::number(alphaCutoff));
	}
	if (j.contains("indexOfRefraction")) {
		float indexOfRefraction = j["indexOfRefraction"];
		indexOfRefractionWidget->value = indexOfRefraction;
		indexOfRefractionWidget->valueLineEdit->setText(QString::number(indexOfRefraction));
	}

	connect(diffuseTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(diffuseTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(normalTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(normalTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(metalnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(metalnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(roughnessTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(roughnessTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(occlusionTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(occlusionTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(emissiveTextureImageWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(emissiveTextureImageSamplerWidget, &FileSelectorWidget::fileSelected, this, &MaterialFileWidget::onValueChanged);
	connect(emissiveFactorWidget, &ScalarWidget::valueChanged, this, &MaterialFileWidget::onValueChanged);
	connect(alphaCutoffWidget, &ScalarWidget::valueChanged, this, &MaterialFileWidget::onValueChanged);
	connect(indexOfRefractionWidget, &ScalarWidget::valueChanged, this, &MaterialFileWidget::onValueChanged);
}

void MaterialFileWidget::onValueChanged() {
	SaveTitleChanger::change(this);
}

void MaterialFileWidget::save() {
	nlohmann::json j;
	if (diffuseTextureImageWidget->filePathButton->path != "") {
		std::string diffuseTextureImagePath = diffuseTextureImageWidget->filePathButton->path;
		std::replace(diffuseTextureImagePath.begin(), diffuseTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(diffuseTextureImagePath).is_absolute()) {
				if (diffuseTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					diffuseTextureImagePath = diffuseTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["diffuseTexture"]["imagePath"] = diffuseTextureImagePath;
	}
	if (diffuseTextureImageSamplerWidget->filePathButton->path != "") {
		std::string diffuseTextureImageSamplerPath = diffuseTextureImageSamplerWidget->filePathButton->path;
		std::replace(diffuseTextureImageSamplerPath.begin(), diffuseTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(diffuseTextureImageSamplerPath).is_absolute()) {
				if (diffuseTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					diffuseTextureImageSamplerPath = diffuseTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["diffuseTexture"]["imageSamplerPath"] = diffuseTextureImageSamplerPath;
	}
	if (normalTextureImageWidget->filePathButton->path != "") {
		std::string normalTextureImagePath = normalTextureImageWidget->filePathButton->path;
		std::replace(normalTextureImagePath.begin(), normalTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(normalTextureImagePath).is_absolute()) {
				if (normalTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					normalTextureImagePath = normalTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["normalTexture"]["imagePath"] = normalTextureImagePath;
	}
	if (normalTextureImageSamplerWidget->filePathButton->path != "") {
		std::string normalTextureImageSamplerPath = normalTextureImageSamplerWidget->filePathButton->path;
		std::replace(normalTextureImageSamplerPath.begin(), normalTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(normalTextureImageSamplerPath).is_absolute()) {
				if (normalTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					normalTextureImageSamplerPath = normalTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["normalTexture"]["imageSamplerPath"] = normalTextureImageSamplerPath;
	}
	if (metalnessTextureImageWidget->filePathButton->path != "") {
		std::string metalnessTextureImagePath = metalnessTextureImageWidget->filePathButton->path;
		std::replace(metalnessTextureImagePath.begin(), metalnessTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(metalnessTextureImagePath).is_absolute()) {
				if (metalnessTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					metalnessTextureImagePath = metalnessTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["metalnessTexture"]["imagePath"] = metalnessTextureImagePath;
	}
	if (metalnessTextureImageSamplerWidget->filePathButton->path != "") {
		std::string metalnessTextureImageSamplerPath = metalnessTextureImageSamplerWidget->filePathButton->path;
		std::replace(metalnessTextureImageSamplerPath.begin(), metalnessTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(metalnessTextureImageSamplerPath).is_absolute()) {
				if (metalnessTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					metalnessTextureImageSamplerPath = metalnessTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["metalnessTexture"]["imageSamplerPath"] = metalnessTextureImageSamplerPath;
	}
	if (roughnessTextureImageWidget->filePathButton->path != "") {
		std::string roughnessTextureImagePath = roughnessTextureImageWidget->filePathButton->path;
		std::replace(roughnessTextureImagePath.begin(), roughnessTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(roughnessTextureImagePath).is_absolute()) {
				if (roughnessTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					roughnessTextureImagePath = roughnessTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["roughnessTexture"]["imagePath"] = roughnessTextureImagePath;
	}
	if (roughnessTextureImageSamplerWidget->filePathButton->path != "") {
		std::string roughnessTextureImageSamplerPath = roughnessTextureImageSamplerWidget->filePathButton->path;
		std::replace(roughnessTextureImageSamplerPath.begin(), roughnessTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(roughnessTextureImageSamplerPath).is_absolute()) {
				if (roughnessTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					roughnessTextureImageSamplerPath = roughnessTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["roughnessTexture"]["imageSamplerPath"] = roughnessTextureImageSamplerPath;
	}
	if (occlusionTextureImageWidget->filePathButton->path != "") {
		std::string occlusionTextureImagePath = occlusionTextureImageWidget->filePathButton->path;
		std::replace(occlusionTextureImagePath.begin(), occlusionTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(occlusionTextureImagePath).is_absolute()) {
				if (occlusionTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					occlusionTextureImagePath = occlusionTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["occlusionTexture"]["imagePath"] = occlusionTextureImagePath;
	}
	if (occlusionTextureImageSamplerWidget->filePathButton->path != "") {
		std::string occlusionTextureImageSamplerPath = occlusionTextureImageSamplerWidget->filePathButton->path;
		std::replace(occlusionTextureImageSamplerPath.begin(), occlusionTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(occlusionTextureImageSamplerPath).is_absolute()) {
				if (occlusionTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					occlusionTextureImageSamplerPath = occlusionTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["occlusionTexture"]["imageSamplerPath"] = occlusionTextureImageSamplerPath;
	}
	if (emissiveTextureImageWidget->filePathButton->path != "") {
		std::string emissiveTextureImagePath = emissiveTextureImageWidget->filePathButton->path;
		std::replace(emissiveTextureImagePath.begin(), emissiveTextureImagePath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(emissiveTextureImagePath).is_absolute()) {
				if (emissiveTextureImagePath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					emissiveTextureImagePath = emissiveTextureImagePath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["emissiveTexture"]["imagePath"] = emissiveTextureImagePath;
	}
	if (emissiveTextureImageSamplerWidget->filePathButton->path != "") {
		std::string emissiveTextureImageSamplerPath = emissiveTextureImageSamplerWidget->filePathButton->path;
		std::replace(emissiveTextureImageSamplerPath.begin(), emissiveTextureImageSamplerPath.end(), '\\', '/');
		if (m_globalInfo.projectDirectory != "") {
			if (std::filesystem::path(emissiveTextureImageSamplerPath).is_absolute()) {
				if (emissiveTextureImageSamplerPath.substr(0, m_globalInfo.projectDirectory.size()) == m_globalInfo.projectDirectory) {
					emissiveTextureImageSamplerPath = emissiveTextureImageSamplerPath.substr(m_globalInfo.projectDirectory.size() + 1);
				}
			}
		}
		j["emissiveTexture"]["imageSamplerPath"] = emissiveTextureImageSamplerPath;
	}
	j["emissiveFactor"] = emissiveFactorWidget->value;
	j["alphaCutoff"] = alphaCutoffWidget->value;
	j["indexOfRefraction"] = indexOfRefractionWidget->value;

	std::fstream optionsFile(m_materialFilePath, std::ios::out | std::ios::trunc);
	optionsFile << j.dump(1, '\t');

	SaveTitleChanger::reset(this);
}
