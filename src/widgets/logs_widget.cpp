#include "logs_widget.h"

LogsWidget::LogsWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1280, 720);
	setWindowTitle("NutshellEngine - Logs");
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	setColumnCount(3);
	setColumnWidth(0, 150);
	setColumnWidth(1, 75);
	setColumnWidth(2, 1055);
	QStringList headerLabels = {"Time", "Level", "Log"};
	setHorizontalHeaderLabels(headerLabels);

	updateLogs();
}

LogsWidget::~LogsWidget() {
	m_globalInfo.logger.logsWidget = nullptr;
}

void LogsWidget::updateLogs() {
	const std::vector<Log>& logs = m_globalInfo.logger.getLogs();
	for (size_t i = currentLog; i < logs.size(); i++) {
		std::string time = std::string(std::asctime(std::localtime(&std::get<0>(logs[i]))));
		time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

		LogLevel logLevel = std::get<1>(logs[i]);
		std::string level = "Info";
		if (logLevel == LogLevel::Info) {
			level = "Info";
		}
		else if (logLevel == LogLevel::Warning) {
			level = "Warning";
		}
		else if (logLevel == LogLevel::Error) {
			level = "Error";
		}

		insertRow(rowCount());
		QTableWidgetItem* timeItem = new QTableWidgetItem(QString::fromStdString(time));
		timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
		setItem(rowCount() - 1, 0, timeItem);
		QTableWidgetItem* levelItem = new QTableWidgetItem(QString::fromStdString(level));
		levelItem->setFlags(levelItem->flags() & ~Qt::ItemIsEditable);
		setItem(rowCount() - 1, 1, levelItem);
		QTableWidgetItem* logItem = new QTableWidgetItem(QString::fromStdString(std::get<2>(logs[i])));
		logItem->setFlags(logItem->flags() & ~Qt::ItemIsEditable);
		setItem(rowCount() - 1, 2, logItem);
	}
	currentLog += logs.size() - currentLog;
}