#include "log_bar.h"

LogBar::LogBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setAlignment(Qt::AlignmentFlag::AlignVCenter);
	QSizePolicy sizePolicy;
	sizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Ignored);
	sizePolicy.setVerticalPolicy(QSizePolicy::Policy::Fixed);
	setSizePolicy(sizePolicy);
	menu = new LogBarMenu(m_globalInfo);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	connect(this, &LogBar::customContextMenuRequested, this, &LogBar::showMenu);
	connect(&m_globalInfo.logger, &Logger::addLogSignal, this, &LogBar::onLogAdded);
	connect(&m_globalInfo.logger, &Logger::clearLogsSignal, this, &LogBar::onLogsCleared);
}

void LogBar::onLogAdded() {
	const Log log = m_globalInfo.logger.getLogs().back();
	std::string time = std::string(std::asctime(std::localtime(&std::get<0>(log))));
	time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

	std::string logString = std::get<2>(log);
	logString.erase(std::remove(logString.begin(), logString.end(), '\n'), logString.end());

	std::string fullLog = time + " - " + logString;
	setText(QString::fromStdString(fullLog));
}

void LogBar::onLogsCleared() {
	clear();
}

void LogBar::showMenu(const QPoint& pos) {
	(void)pos;
	menu->popup(QCursor::pos());
}

void LogBar::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {
		if (!m_logsWidget) {
			m_logsWidget = new LogsWidget(m_globalInfo);
			m_logsWidget->show();

			connect(m_logsWidget, &LogsWidget::closeWindow, this, &LogBar::onLogsWidgetClose);
		}
		else {
			m_logsWidget->activateWindow();
		}
	}
	event->accept();
}

void LogBar::paintEvent(QPaintEvent* event) {
	if (m_globalInfo.logger.getLogs().empty()) {
		return;
	}

	const Log& log = m_globalInfo.logger.getLogs().back();

	LogLevel logLevel = std::get<1>(log);
	if (logLevel == LogLevel::Info) {
		setStyleSheet("color: rgba(225, 225, 225, 255)");
	}
	else if (logLevel == LogLevel::Warning) {
		setStyleSheet("color: rgba(225, 115, 0, 255)");
	}
	else if (logLevel == LogLevel::Error) {
		setStyleSheet("color: rgba(225, 25, 0, 255)");
	}

	QLabel::paintEvent(event);
}

void LogBar::onLogsWidgetClose() {
	m_logsWidget = nullptr;
}
