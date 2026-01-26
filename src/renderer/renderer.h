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
#include <QKeyEvent>
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
	void destroyPickingImages();
	void createOutlineSoloImages();
	void destroyOutlineSoloImages();
	void createLightBuffer();

	bool anyEntityTransformMode();
	void updateCamera();
	void updateLights();

	void loadResourcesToGPU();

	void calculateTranslation(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition);
	void calculateRotation(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition);
	void calculateScale(const std::set<EntityID> entityIDs, const nml::vec2& mouseCursorCurrentPosition);
	void startTransform();
	void cancelTransform();

	nml::vec2 project(const nml::vec3& p, float width, float height, const nml::mat4& viewProjMatrix);
	nml::vec3 unproject(const nml::vec2& p, float width, float height, const nml::mat4& invViewMatrix, const nml::mat4& invProjMatrix);

	nml::mat4 perspectiveRHOpenGL(float fovY, float aspectRatio, float near, float far);
	nml::mat4 orthographicRHOpenGL(float left, float right, float bottom, float top, float near, float far);

private slots:
	void onEntityDestroyed(EntityID entityID);
	void onEntitySelected();
	void onCameraProjectionSwitched(bool cameraProjectionOrthographic);
	void onCameraReset();
	void onOrthographicCameraToAxisChanged(const nml::vec3& axis);
	void onCameraGoToEntity(EntityID entityID);

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

	float m_gizmoTranslationStepAccumulation = 0.0f;
	float m_gizmoRotationStepAccumulation = 0.0f;
	float m_gizmoScaleStepAccumulation = 0.0f;

	enum class GizmoMode {
		Translate,
		Rotate,
		Scale,
		None
	};
	GizmoMode m_gizmoMode = GizmoMode::None;

	enum class GizmoAxis {
		X,
		Y,
		Z,
		None
	};
	GizmoAxis m_gizmoAxis = GizmoAxis::None;

	bool m_multiSelectionKeyPressed = false;

	bool m_moveCameraButtonPressed = false;

	float m_mouseScrollY = 0.0f;

	nml::vec2 m_mouseCursorPreviousPosition = nml::vec2(0.0f, 0.0f);
	nml::vec2 m_mouseCursorDifference = nml::vec2(0.0f, 0.0f);

	nml::vec2 m_savedMousePosition = nml::vec2(0.0f, 0.0f);

	nml::vec3 m_selectionMeanPosition = nml::vec3(0.0f, 0.0f, 0.0f);

	bool m_mouseMoveFlag = false;

	QTimer m_waitTimer;

	bool m_doPicking = false;
	bool m_gotResized = false;

	enum class DragDropResourceType {
		Model,
		Material,
		None
	};
	DragDropResourceType m_dragDropResourceType = DragDropResourceType::None;
	std::string m_dragDropResourcePath;

	std::unordered_map<EntityID, Transform> m_entityMoveTransforms;

	GLuint m_entityProgram;
	GLuint m_cameraFrustumProgram;
	GLuint m_grid2DProgram;
	GLuint m_grid3DProgram;
	GLuint m_gizmoProgram;
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