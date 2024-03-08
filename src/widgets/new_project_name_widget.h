#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLineEdit>
#include <memory>

class NewProjectNameWidget : public QWidget {
	Q_OBJECT
public:
	NewProjectNameWidget(GlobalInfo& globalInfo);

signals:
	void textChanged(const std::string&);

private slots:
	void onTextChanged();

private:
	GlobalInfo& m_globalInfo;

	std::unique_ptr<QLineEdit> m_projectNameLineEdit;
};