#if defined(NTSHENGN_OS_WINDOWS) && defined(NTSHENGN_RELEASE)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "src/common/global_info.h"
#include "src/widgets/project_window.h"
#include "src/widgets/main_window.h"
#include <QApplication>
#include <memory>

int main() {
	int argc = 0;
	std::unique_ptr<QApplication> application = std::make_unique<QApplication>(argc, nullptr);
	application->setStyle("Fusion");

	GlobalInfo globalInfo;
	globalInfo.version = "0.2.2";
	globalInfo.devicePixelRatio = application->devicePixelRatio();
	globalInfo.actionUndoStack = std::make_unique<QUndoStack>();
	globalInfo.selectionUndoStack = std::make_unique<QUndoStack>();

	ProjectWindow projectWindow(globalInfo);
	projectWindow.show();

	application->exec();

	return 0;
}
