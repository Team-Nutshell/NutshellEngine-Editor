#include "build_bar.h"
#include <QHBoxLayout>

BuildBar::BuildBar(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QHBoxLayout());
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);
	buildButton = new QPushButton("Build");
	layout()->addWidget(buildButton);

	if (globalInfo.compilerPath.empty()) {
		buildButton->setEnabled(false);
	}
	else {
		buildButton->setEnabled(true);
	}
}