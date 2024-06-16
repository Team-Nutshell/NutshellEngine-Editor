#include "logs_widget.h"

LogsWidget::LogsWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(640, 360);
	setWindowTitle("NutshellEngine - Logs");
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	updateLogs();
}

LogsWidget::~LogsWidget() {
	m_globalInfo.logger.logsWidget = nullptr;
}

void LogsWidget::updateLogs() {
	clear();
	const std::vector<Log> logs = m_globalInfo.logger.getLogs();
	for (const auto& log : logs) {
		std::string time = std::string(std::asctime(std::localtime(&std::get<0>(log))));
		time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

		LogLevel logLevel = std::get<1>(log);
		QColor color(225, 225, 225, 255);
		if (logLevel == LogLevel::Info) {
			color = QColor(225, 225, 225, 255);
		}
		else if (logLevel == LogLevel::Warning) {
			color = QColor(225, 115, 0, 255);
		}
		else if (logLevel == LogLevel::Error) {
			color = QColor(225, 25, 0, 255);
		}

		std::string fullLog = time + " - " + std::get<2>(log);

		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(fullLog));
		item->setForeground(color);
		addItem(item);
	}
}
