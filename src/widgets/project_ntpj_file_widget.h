#pragma once
#include "../common/global_info.h"
#include "string_widget.h"
#include "integer_widget.h"

class ProjectNtpjFileWidget : public QWidget {
	Q_OBJECT
public:
	ProjectNtpjFileWidget(GlobalInfo& globalInfo);

private slots:
	void onStringChanged(const std::string& value);
	void onIntegerChanged(int value);
	void save();

private:
	GlobalInfo& m_globalInfo;

public:
	StringWidget* projectNameWidget;
	IntegerWidget* steamAppIDWidget;
};