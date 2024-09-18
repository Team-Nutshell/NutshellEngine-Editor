#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QCloseEvent>

class AboutWidget : public QWidget {
	Q_OBJECT
public:
	AboutWidget(GlobalInfo& globalInfo);

private slots:
	void closeEvent(QCloseEvent* event);
	
	void onLinkClicked(const QString& link);

signals:
	void closeWindow();

private:
	GlobalInfo& m_globalInfo;
};