#include "src/common/common.h"
#include "src/widgets/project_window.h"
#include "src/widgets/main_window.h"
#include <QApplication>
#include <memory>

int main() {
	int argc = 0;
	std::unique_ptr<QApplication> application = std::make_unique<QApplication>(argc, nullptr);
	application->setStyle("Fusion");

	GlobalInfo globalInfo;
	globalInfo.devicePixelRatio = application->devicePixelRatio();
	globalInfo.undoStack = std::make_unique<QUndoStack>();

	ProjectWindow projectWindow(globalInfo);
	projectWindow.show();

	application->exec();

	return 0;
}