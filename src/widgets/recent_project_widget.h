#pragma once
#include "../common/global_info.h"
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QEnterEvent>
#include <string>

class RecentProjectWidget : public QWidget {
	Q_OBJECT
public:
	RecentProjectWidget(GlobalInfo& globalInfo, const std::string& projectDirectory);

	const std::string& getProjectDirectory();

signals:
	void openProject(const std::string&);
	void removeRecentProject();

private slots:
	void onRemoveRecentProjectButtonClicked();

	void mousePressEvent(QMouseEvent* event);
	void enterEvent(QEnterEvent* event);
	void leaveEvent(QEvent* event);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* projectNameLabel;
	QLabel* projectDirectoryLabel;
	QPushButton* removeRecentProjectButton;

	std::string m_projectDirectory;
};