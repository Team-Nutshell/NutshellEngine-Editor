#pragma once
#include "../common/common.h"
#include <QOpenGLWidget>

class Renderer : public QOpenGLWidget {
	Q_OBJECT
public:
	Renderer(GlobalInfo& globalInfo);

private:
	GlobalInfo& m_globalInfo;
};