#include "log_bar_menu.h"

LogBarMenu::LogBarMenu(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	clearLogsAction = addAction("Clear Logs", this, &LogBarMenu::clearLogs);
}

void LogBarMenu::clearLogs() {
	emit m_globalInfo.logger.clearLogsSignal();
}