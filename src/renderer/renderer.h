#pragma once
#include "../common/common.h"
#include "../../external/nml/include/nml.h"
#include "renderer_camera.h"
#include "renderer_model.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QResizeEvent>
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

	bool anyEntityTransformKeyPressed();
	void updateCamera();
	void updateLights();

	void loadResourcesToGPU();

	nml::vec3 unproject(const nml::vec2& p, float width, float height, const nml::mat4& invViewMatrix, const nml::mat4& invProjMatrix);

private slots:
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

private:
	GlobalInfo& m_globalInfo;

	RendererCamera m_camera;

	Qt::Key m_cameraForwardKey = Qt::Key::Key_W;
	Qt::Key m_cameraBackwardKey = Qt::Key::Key_S;
	Qt::Key m_cameraLeftKey = Qt::Key::Key_A;
	Qt::Key m_cameraRightKey = Qt::Key::Key_D;
	Qt::Key m_cameraUpKey = Qt::Key::Key_Space;
	Qt::Key m_cameraDownKey = Qt::Key::Key_Shift;

	Qt::Key m_translateEntityKey = Qt::Key::Key_T;
	Qt::Key m_rotateEntityKey = Qt::Key::Key_R;
	Qt::Key m_scaleEntityKey = Qt::Key::Key_E;

	bool m_cameraForwardKeyPressed = false;
	bool m_cameraBackwardKeyPressed = false;
	bool m_cameraLeftKeyPressed = false;
	bool m_cameraRightKeyPressed = false;
	bool m_cameraUpKeyPressed = false;
	bool m_cameraDownKeyPressed = false;

	bool m_translateEntityKeyPressed = false;
	bool m_rotateEntityKeyPressed = false;
	bool m_scaleEntityKeyPressed = false;

	bool m_moveCameraButtonPressed = false;

	float m_mouseScrollY = 0.0f;

	nml::vec2 m_mouseCursorPreviousPosition = nml::vec2(0.0f, 0.0f);
	nml::vec2 m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);

	nml::vec2 m_savedMousePosition = nml::vec2(0.0f, 0.0f);

	bool m_mouseMoveFlag = false;

	QTimer m_waitTimer;

	bool m_doPicking = false;
	bool m_gotResized = false;

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