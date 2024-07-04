#include "log_bar.h"
#include "logs_widget.h"
#include "main_window.h"
#include <QPainter>
#include <QFontMetrics>

LogBar::LogBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setAlignment(Qt::AlignmentFlag::AlignVCenter);
}

void LogBar::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {
		LogsWidget* logsWidget = new LogsWidget(m_globalInfo);
		logsWidget->show();

		m_globalInfo.logger.logsWidget = logsWidget;
	}
	event->accept();
}

void LogBar::paintEvent(QPaintEvent* event) {
	(void)event;
		
	const Log& log = m_globalInfo.logger.getLogs().back();
	std::string time = std::string(std::asctime(std::localtime(&std::get<0>(log))));
	time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

	std::string logString = std::get<2>(log);
	logString.erase(std::remove(logString.begin(), logString.end(), '\n'), logString.end());

	std::string fullLog = time + " - " + logString;
	setText(QString::fromStdString(fullLog));

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

	QPainter painter(this);
	QFontMetrics fontMetrics(font());
	MainWindow* mainWindow = reinterpret_cast<MainWindow*>(m_globalInfo.mainWindow);
	QString elidedText = fontMetrics.elidedText(text(), Qt::ElideRight, mainWindow->width());

	painter.drawText(rect(), alignment(), elidedText);
}