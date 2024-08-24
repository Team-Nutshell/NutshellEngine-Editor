#include "logs_widget.h"
#include <QVBoxLayout>
#include <QScrollBar>

LogsWidget::LogsWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	resize(1310, 720);
	setWindowTitle("NutshellEngine - Logs");
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	clearLogsButton = new QPushButton("Clear logs");
	layout()->addWidget(clearLogsButton);
	logsTable = new QTableWidget();
	logsTable->setColumnCount(3);
	logsTable->setColumnWidth(0, 150);
	logsTable->setColumnWidth(1, 75);
	logsTable->setColumnWidth(2, 1055);
	QStringList headerLabels = {"Time", "Level", "Log"};
	logsTable->setHorizontalHeaderLabels(headerLabels);
	layout()->addWidget(logsTable);

	connect(clearLogsButton, &QPushButton::clicked, this, &LogsWidget::onClearLogsButtonClicked);
	connect(&m_globalInfo.logger, &Logger::addLogSignal, this, &LogsWidget::onLogAdded);
	connect(&m_globalInfo.logger, &Logger::clearLogsSignal, this, &LogsWidget::onLogsCleared);

	updateLogs();
}

void LogsWidget::updateLogs() {
	const std::vector<Log>& logs = m_globalInfo.logger.getLogs();
	for (size_t i = m_currentLog; i < logs.size(); i++) {
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

		std::string logString = std::get<2>(logs[i]);

		logsTable->insertRow(logsTable->rowCount());
		QTableWidgetItem* timeItem = new QTableWidgetItem(QString::fromStdString(time));
		timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
		logsTable->setItem(logsTable->rowCount() - 1, 0, timeItem);
		QTableWidgetItem* levelItem = new QTableWidgetItem(QString::fromStdString(level));
		levelItem->setFlags(levelItem->flags() & ~Qt::ItemIsEditable);
		logsTable->setItem(logsTable->rowCount() - 1, 1, levelItem);
		QTableWidgetItem* logItem = new QTableWidgetItem(QString::fromStdString(logString));
		logItem->setFlags(logItem->flags() & ~Qt::ItemIsEditable);
		logsTable->setItem(logsTable->rowCount() - 1, 2, logItem);
		logsTable->resizeRowToContents(logsTable->rowCount() - 1);

		if (logLevel == LogLevel::Info) {
			timeItem->setForeground(QBrush(QColor(225, 225, 225)));
			levelItem->setForeground(QBrush(QColor(225, 225, 225)));
			logItem->setForeground(QBrush(QColor(225, 225, 225)));
		}
		else if (logLevel == LogLevel::Warning) {
			timeItem->setForeground(QBrush(QColor(225, 115, 0)));
			levelItem->setForeground(QBrush(QColor(225, 115, 0)));
			logItem->setForeground(QBrush(QColor(225, 115, 0)));
		}
		else if (logLevel == LogLevel::Error) {
			timeItem->setForeground(QBrush(QColor(225, 25, 0)));
			levelItem->setForeground(QBrush(QColor(225, 25, 0)));
			logItem->setForeground(QBrush(QColor(225, 25, 0)));
		}
	}
	m_currentLog += logs.size() - m_currentLog;
	logsTable->verticalScrollBar()->setSliderPosition(logsTable->verticalScrollBar()->maximum());
}

void LogsWidget::onClearLogsButtonClicked() {
	emit m_globalInfo.logger.clearLogsSignal();
}

void LogsWidget::onLogsCleared() {
	logsTable->clearContents();
	logsTable->setRowCount(0);

	m_currentLog = 0;
}

void LogsWidget::onLogAdded() {
	updateLogs();
}