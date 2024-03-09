#include "renderer.h"

Renderer::Renderer(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMouseTracking(true);


}
