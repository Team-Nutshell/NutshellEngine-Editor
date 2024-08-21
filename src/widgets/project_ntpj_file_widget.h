#pragma once
#include "../common/global_info.h"
#include "string_widget.h"

class ProjectNtpjFileWidget : public QWidget {
	Q_OBJECT
public:
	ProjectNtpjFileWidget(GlobalInfo& globalInfo);

private slots:
	void onValueChanged();
	void save();

private:
	GlobalInfo& m_globalInfo;

public:
	StringWidget* projectNameWidget;
};