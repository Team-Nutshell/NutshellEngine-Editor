#include "about_widget.h"
#include "separator_line.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QDesktopServices>

AboutWidget::AboutWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFixedWidth(550);
	setFixedHeight(400);
	setWindowTitle("NutshellEngine - " + QString::fromStdString(m_globalInfo.localization.getString("header_about_nutshell_engine")));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	QLabel* logoLabel = new QLabel();
	QPixmap* logoPixmap = new QPixmap("assets/logo.png");
	logoLabel->setPixmap(logoPixmap->scaled(530, height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
	layout()->addWidget(logoLabel);
	QLabel* versionLabel = new QLabel("<b>NutshellEngine " + QString::fromStdString(m_globalInfo.version) + "</b>");
	layout()->addWidget(versionLabel);
	layout()->addWidget(new SeparatorLine(m_globalInfo));
	layout()->setAlignment(versionLabel, Qt::AlignmentFlag::AlignCenter);
	QLabel* aboutNutshellEngineLabel = new QLabel(QString::fromStdString(m_globalInfo.localization.getString("about_nutshell_engine", { "<b><a href=\"https://github.com/Team-Nutshell\">" + m_globalInfo.localization.getString("about_nutshell_engine_team_nutshell_github") + "</a></b>" })));
	aboutNutshellEngineLabel->setWordWrap(true);
	layout()->addWidget(aboutNutshellEngineLabel);

	connect(aboutNutshellEngineLabel, &QLabel::linkActivated, this, &AboutWidget::onLinkClicked);
}

void AboutWidget::onLinkClicked(const QString& link) {
	QDesktopServices::openUrl(QUrl(link));
}

void AboutWidget::closeEvent(QCloseEvent* event) {
	emit closeWindow();
	QWidget::closeEvent(event);
}