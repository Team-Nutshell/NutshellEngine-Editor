#pragma once
#include "../common/global_info.h"
#include <QMessageBox>
#include <QLineEdit>

class NewScriptMessageBox : public QMessageBox {
	Q_OBJECT
public:
	NewScriptMessageBox(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;

public:
	QLineEdit* scriptNameLineEdit;
};