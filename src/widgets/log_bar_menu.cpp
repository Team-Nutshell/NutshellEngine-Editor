#include "log_bar_menu.h"

LogBarMenu::LogBarMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	clearLogsAction = addAction(QString::fromStdString(m_globalInfo.localization.getString("logs_clear_logs")), this, &LogBarMenu::clearLogs);
}

void LogBarMenu::clearLogs() {
	emit m_globalInfo.logger.clearLogsSignal();
}