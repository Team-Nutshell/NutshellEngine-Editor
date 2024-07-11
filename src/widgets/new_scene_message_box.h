#pragma once
#include "../common/global_info.h"
#include <QMessageBox>

class NewSceneMessageBox : public QMessageBox {
	Q_OBJECT
public:
	NewSceneMessageBox(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;
};