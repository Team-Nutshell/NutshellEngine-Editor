#pragma once
#include "../common/global_info.h"
#include "../../external/nml/include/nml.h"
#include "renderer_camera.h"
#include "renderer_resources.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <optional>

class Renderer : public QOpenGLWidget {
	Q_OBJECT
public:
	Renderer(GlobalInfo& globalInfo);
	~Renderer();

	void initializeGL();
	void paintGL();

private:
	GLuint compileShader(GLenum shaderType, const std::string& shaderCode);
	GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);

	void createPickingImages();
	void createOutlineSoloImages();
	void createLightBuffer();

	bool anyEntityTransformMode();
	void updateCamera();
	void updateLights();

	void loadResourcesToGPU();

	void cancelTransform();

	nml::vec2 project(const nml::vec3& p, float width, float height, const nml::mat4& viewProjMatrix);
	nml::vec3 unproject(const nml::vec2& p, float width, float height, const nml::mat4& invViewMatrix, const nml::mat4& invProjMatrix);

private slots:
	void onEntitySelected();
	void onGridVisibilityToggled(bool showGrid);
	void onBackfaceCullingToggled(bool backfaceCulling);
	void onCamerasVisibilityToggled(bool showCameras);
	void onLightingToggled(bool lightingEnabled);
	void onCollidersVisibilityToggled(bool showColliders);
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);
	void onCameraReset();
	void onOrthographicCameraToAxisChanged(const nml::vec3& axis);

	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	void focusOutEvent(QFocusEvent* event);
	void resizeEvent(QResizeEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dropEvent(QDropEvent* event);
	void leaveEvent(QEvent* event);

private:
	GlobalInfo& m_globalInfo;

	RendererCamera m_camera;

	bool m_cameraForwardKeyPressed = false;
	bool m_cameraBackwardKeyPressed = false;
	bool m_cameraLeftKeyPressed = false;
	bool m_cameraRightKeyPressed = false;
	bool m_cameraUpKeyPressed = false;
	bool m_cameraDownKeyPressed = false;

	bool m_translateEntityMode = false;
	bool m_rotateEntityMode = false;
	bool m_scaleEntityMode = false;

	bool m_moveCameraButtonPressed = false;

	float m_mouseScrollY = 0.0f;

	nml::vec2 m_mouseCursorPreviousPosition = nml::vec2(0.0f, 0.0f);
	nml::vec2 m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);

	nml::vec2 m_savedMousePosition = nml::vec2(0.0f, 0.0f);

	bool m_mouseMoveFlag = false;

	QTimer m_waitTimer;

	bool m_doPicking = false;
	bool m_gotResized = false;

	bool m_showGrid = true;
	bool m_backfaceCullingEnabled = false;
	bool m_showCameras = false;
	bool m_lightingEnabled = false;
	bool m_showColliders = false;

	std::optional<Transform> m_entityMoveTransform;

	GLuint m_entityProgram;
	GLuint m_cameraFrustumProgram;
	GLuint m_gridProgram;
	GLuint m_pickingProgram;
	GLuint m_outlineSoloProgram;
	GLuint m_outlineProgram;
	GLuint m_colliderProgram;

	GLuint m_pickingFramebuffer;
	GLuint m_pickingImage;
	GLuint m_pickingDepthImage;

	GLuint m_outlineSoloFramebuffer;
	GLuint m_outlineSoloImage;
	GLuint m_outlineSoloDepthImage;

	GLuint m_lightBuffer;

	QOpenGLFunctions gl;
	QOpenGLExtraFunctions glex;
};