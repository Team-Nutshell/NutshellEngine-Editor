#include "recent_project_widget.h"
#include "separator_line.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <fstream>

RecentProjectWidget::RecentProjectWidget(GlobalInfo& globalInfo, const std::string& projectDirectory) : m_globalInfo(globalInfo), m_projectDirectory(projectDirectory) {
	std::string projectName = projectDirectory.substr(projectDirectory.rfind('/') + 1);

	std::fstream projectFile(projectDirectory + "/project.ntpj", std::ios::in);
	if (projectFile.is_open()) {
		if (nlohmann::json::accept(projectFile)) {
			projectFile = std::fstream(projectDirectory + "/project.ntpj", std::ios::in);
			nlohmann::json j = nlohmann::json::parse(projectFile);

			if (j.contains("projectName")) {
				projectName = j["projectName"];
			}
		}
	}

	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	QWidget* hBoxLayoutWidget = new QWidget();
	hBoxLayoutWidget->setLayout(new QHBoxLayout());
	hBoxLayoutWidget->layout()->setContentsMargins(0, 0, 0, 0);
	projectNameLabel = new QLabel("<b>" + QString::fromStdString(projectName) + "</b>");
	hBoxLayoutWidget->layout()->addWidget(projectNameLabel);
	removeRecentProjectButton = new QPushButton("X");
	removeRecentProjectButton->setToolTip(QString::fromStdString(m_globalInfo.localization.getString("recent_projects_remove")));
	removeRecentProjectButton->setFixedWidth(20);
	hBoxLayoutWidget->layout()->addWidget(removeRecentProjectButton);
	hBoxLayoutWidget->layout()->setAlignment(removeRecentProjectButton, Qt::AlignmentFlag::AlignRight);
	layout()->addWidget(hBoxLayoutWidget);
	projectDirectoryLabel = new QLabel(QString::fromStdString(projectDirectory));
	projectDirectoryLabel->setStyleSheet("color: rgba(200, 200, 200, 255);");
	layout()->addWidget(projectDirectoryLabel);
	layout()->addWidget(new SeparatorLine());

	connect(removeRecentProjectButton, &QPushButton::clicked, this, &RecentProjectWidget::onRemoveRecentProjectButtonClicked);
}

const std::string& RecentProjectWidget::getProjectDirectory() {
	return m_projectDirectory;
}

void RecentProjectWidget::mousePressEvent(QMouseEvent* event) {
	(void)event;
	emit openProject(m_projectDirectory);
}

void RecentProjectWidget::enterEvent(QEnterEvent* event) {
	(void)event;
	QPalette palette = QPalette();
	palette.setColor(QPalette::ColorRole::Window, QColor::fromRgbF(0.2f, 0.2f, 0.2f));
	setAutoFillBackground(true);
	setPalette(palette);
}

void RecentProjectWidget::leaveEvent(QEvent* event) {
	(void)event;
	setPalette(QPalette());
}

void RecentProjectWidget::onRemoveRecentProjectButtonClicked() {
	emit removeRecentProject();
}