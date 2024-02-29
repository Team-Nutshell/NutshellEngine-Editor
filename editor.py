import copy
import ctypes
import OpenGL.GL as gl
from PyQt6.QtCore import Qt, QObject, pyqtSignal, QSignalBlocker, QLocale, QPoint, QTimer
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QLabel, QHBoxLayout, QVBoxLayout, QMenu, QFileDialog, QMessageBox, QListWidget, QListWidgetItem, QLineEdit, QCheckBox, QScrollArea, QFrame, QSplitter
from PyQt6.QtGui import QFocusEvent, QKeyEvent, QMouseEvent, QResizeEvent, QUndoStack, QUndoCommand, QCursor, QIcon, QDoubleValidator
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
import numpy as np

class Component():
	pass

class Transform(Component):
	def __init__(self):
		self.position = np.zeros(3, dtype=np.float32)
		self.rotation = np.zeros(3, dtype=np.float32)
		self.scale = np.ones(3, dtype=np.float32)

class Entity():
	def __init__(self, name, entityID=-1):
		if entityID == -1:
			self.entityID = globalInfo.entityID
			globalInfo.entityID += 1
		else:
			self.entityID = entityID
		self.name = name
		self.isPersistent = False
		self.components = {}

class GlobalInfo():
	def __init__(self):
		self.entities = []
		self.currentEntityID = -1
		self.devicePixelRatio = 1.0
		self.window = None
		self.undoStack = QUndoStack()
		self.signalEmitter = None
		self.entityID = 0

	def findEntityById(self, entityID):
		for i in range(len(self.entities)):
			if self.entities[i].entityID == entityID:
				return i
		return -1

globalInfo = GlobalInfo()

class SignalEmitter(QObject):
	createEntitySignal = pyqtSignal(int)
	destroyEntitySignal = pyqtSignal(int)
	selectEntitySignal = pyqtSignal(int)
	changeNameEntitySignal = pyqtSignal(int, str)
	changePersistenceEntitySignal = pyqtSignal(int, bool)
	changeEntityTransformSignal = pyqtSignal(int, Transform)

class NewMessageBox(QMessageBox):
	def __init__(self):
		super().__init__()
		self.setWindowTitle("New...")
		self.setText("Do you want to create a new scene?\nAll changes not saved will be lost.")
		self.setStandardButtons(QMessageBox.StandardButton.Ok | QMessageBox.StandardButton.Cancel)
		ret = self.exec()
		if ret == QMessageBox.StandardButton.Ok:
			self.okButton()

	def okButton(self):
		while len(globalInfo.entities) != 0:
			globalInfo.undoStack.push(DestroyEntityCommand(globalInfo.entities[-1].entityID))

class FileMenu(QMenu):
	def __init__(self):
		super().__init__("&File")
		self.newAction = self.addAction("New", self.new)
		self.newAction.setShortcut("Ctrl+N")
		self.openAction = self.addAction("Open...", self.open)
		self.openAction.setShortcut("Ctrl+O")

	def new(self):
		NewMessageBox()

	def open(self):
		fileDialog = QFileDialog()
		files = []
		if fileDialog.exec():
			files = fileDialog.selectedFiles()
		return files
	
class EditMenu(QMenu):
	def __init__(self):
		super().__init__("&Edit")
		self.undoAction = globalInfo.undoStack.createUndoAction(self, "&Undo")
		self.undoAction.setShortcut("Ctrl+Z")
		self.redoAction = globalInfo.undoStack.createRedoAction(self, "&Redo")
		self.redoAction.setShortcut("Ctrl+Y")
		self.addAction(self.undoAction)
		self.addAction(self.redoAction)

class MathHelper():
	@staticmethod
	def normalize(vector):
		return vector / np.linalg.norm(vector)

	@staticmethod
	def lookAtRH(fromVector, toVector, upVector):
		tMf = np.subtract(toVector, fromVector)
		forward = MathHelper.normalize(tMf)
		fXu = np.cross(forward, upVector)
		right = MathHelper.normalize(fXu)
		realUp = np.cross(right, forward)
		return np.array([right[0], realUp[0], -forward[0], 0.0,
					right[1], realUp[1], -forward[1], 0.0,
					right[2], realUp[2], -forward[2], 0.0,
					-np.dot(right, fromVector), -np.dot(realUp, fromVector), np.dot(forward, fromVector), 1.0], dtype=np.float32)
	
	@staticmethod
	def perspectiveRH(fovY, aspectRatio, near, far):
		tanHalfFovY = np.tan(fovY / 2.0)
		farMnear = far - near
		nearMfar = near - far
		return np.array([1.0 / (aspectRatio * tanHalfFovY), 0.0, 0.0, 0.0,
					0.0, 1.0 / tanHalfFovY, 0.0, 0.0,
					0.0, 0.0, far / nearMfar, -1.0,
					0.0, 0.0, -(far * near) / farMnear, 0.0], dtype=np.float32)
	
	@staticmethod
	def translate(translation):
		return np.array([1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					translation[0], translation[1], translation[2], 1.0], dtype=np.float32)
	
	@staticmethod
	def rotate(angle, axis):
		cosTheta = np.cos(angle)
		oMct = 1.0 - cosTheta
		sinTheta = np.sin(angle)
		return np.array([cosTheta + ((axis[0] * axis[0]) * oMct),
		((axis[1] * axis[0]) * oMct) + (axis[2] * sinTheta),
		((axis[2] * axis[0]) * oMct) - (axis[1] * sinTheta),
		0.0,
		((axis[0] * axis[1]) * oMct) - (axis[2] * sinTheta),
		cosTheta + ((axis[1] * axis[1]) * oMct),
		((axis[2] * axis[1]) * oMct) + (axis[0] * sinTheta),
		0.0,
		((axis[0] * axis[2]) * oMct) + (axis[1] * sinTheta),
		((axis[1] * axis[2]) * oMct) - (axis[0] * sinTheta),
		cosTheta + ((axis[2] * axis[2]) * oMct),
		0.0,
		0.0, 0.0, 0.0, 1.0], dtype=np.float32)
	
	@staticmethod
	def scale(scaling):
		return np.array([scaling[0], 0.0, 0.0, 0.0,
				0.0, scaling[1], 0.0, 0.0,
				0.0, 0.0, scaling[2], 0.0,
				0.0, 0.0, 0.0, 1.0], dtype=np.float32)
	
	@staticmethod
	def mat4x4Mult(m1, m2):
		return np.array([
				m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3],
				m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3],
				m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3],
				m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3],

				m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7],
				m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7],
				m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7],
				m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7],

				m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11],
				m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11],
				m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11],
				m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11],

				m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15],
				m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15],
				m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15],
				m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15]], dtype=np.float32)

class OpenGLHelper():
	@staticmethod
	def compileShader(shaderType, shaderCode):
		shaderCompilationState = True
		shader = gl.glCreateShader(shaderType)
		gl.glShaderSource(shader, shaderCode)
		gl.glCompileShader(shader)
		if not gl.glGetShaderiv(shader, gl.GL_COMPILE_STATUS):
			print("Error while compiling shader (", str(shaderType), "): ", gl.glGetShaderInfoLog(shader))
			shaderCompilationState = False
		return [shader, shaderCompilationState]
	
	@staticmethod
	def createProgram(vertexShader, fragmentShader):
		programLinkState = True
		program = gl.glCreateProgram()
		gl.glAttachShader(program, vertexShader)
		gl.glAttachShader(program, fragmentShader)
		gl.glLinkProgram(program)
		if not gl.glGetProgramiv(program, gl.GL_LINK_STATUS):
			print("Error while linking shaders to program: " + gl.glGetProgramInfoLog(program))
			programLinkState = False
		gl.glDetachShader(program, vertexShader)
		gl.glDetachShader(program, fragmentShader)

		return [program, programLinkState]

	@staticmethod
	def fullscreenVertexShaderCode():
		return '''
		#version 460

		out vec2 uv;

		void main() {
			uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
			gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
		}
		'''
	
class RendererCamera():
	def __init__(self):
		self.position = np.array([0.0, 1.0, -1.0], dtype=np.float32)
		self.direction = np.array([0.0, -1.0, 1.0], dtype=np.float32)
		self.direction = MathHelper.normalize(self.direction)
		self.nearPlane = 0.01
		self.farPlane = 100.0
		self.cameraSpeed = 1.0

class Renderer(QOpenGLWidget):
	def __init__(self):
		super().__init__()
		self.setFocusPolicy(Qt.FocusPolicy.ClickFocus)

		self.camera = RendererCamera()

		self.forwardKey = Qt.Key.Key_W
		self.forwardKeyPressed = False

		self.backwardKey = Qt.Key.Key_S
		self.backwardKeyPressed = False

		self.leftKey = Qt.Key.Key_A
		self.leftKeyPressed = False

		self.rightKey = Qt.Key.Key_D
		self.rightKeyPressed = False

		self.upKey = Qt.Key.Key_Space
		self.upKeyPressed = False

		self.downKey = Qt.Key.Key_Shift
		self.downKeyPressed = False

		self.mouseCursorPreviousPosition = np.array(2, dtype=np.float32)
		self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
		
		self.cameraYaw = np.rad2deg(np.arctan2(self.camera.direction[2], self.camera.direction[0]))
		self.cameraPitch = np.rad2deg(-np.arcsin(self.camera.direction[1]))
		
		self.waitTimer = QTimer()
		self.waitTimer.timeout.connect(self.update)

		self.doPicking = False

		self.gotResized = False

	def initializeGL(self):
		[fullscreenVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, OpenGLHelper.fullscreenVertexShaderCode())

		# Entity Program
		entityVertexShaderCode = '''
		#version 460

		in vec3 position;

		uniform mat4 viewProj;
		uniform mat4 model;

		void main() {
			gl_Position = viewProj * model * vec4(position, 1.0);
		}
		'''
		[entityVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, entityVertexShaderCode)

		entityFragmentShaderCode = '''
		#version 460

		out vec4 outColor;

		void main() {
			outColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
		'''
		[entityFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, entityFragmentShaderCode)

		[self.entityProgram, _] = OpenGLHelper.createProgram(entityVertexShader, entityFragmentShader)

		# Grid Program
		gridVertexShaderCode = '''
		#version 460

		uniform mat4 view;
		uniform mat4 projection;

		out vec3 nearPoint;
		out vec3 farPoint;

		vec2 plane[6] = vec2[](
			vec2(1.0, 1.0),
			vec2(-1.0, -1.0),
			vec2(-1.0, 1.0),
			vec2(-1.0, -1.0),
			vec2(1.0, 1.0),
			vec2(1.0, -1.0)
		);

		vec3 unprojectPoint(vec3 p) {
			vec4 unprojected = inverse(view) * inverse(projection) * vec4(p, 1.0);
			
			return unprojected.xyz / unprojected.w;
		}

		void main() {
			vec2 p = plane[gl_VertexID];
			
			nearPoint = unprojectPoint(vec3(p, 0.0));
			farPoint = unprojectPoint(vec3(p, 1.0));

			gl_Position = vec4(p, 0.0, 1.0);
		}
		'''
		[gridVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, gridVertexShaderCode)

		gridFragmentShaderCode = '''
		#version 460

		in vec3 nearPoint;
		in vec3 farPoint;

		uniform mat4 viewProj;
		uniform float near;
		uniform float far;

		out vec4 outColor;

		vec4 grid(vec3 p, float scale) {
			vec2 coord = p.xz * scale;
			vec2 derivative = fwidth(coord);
			vec2 g = abs(fract(coord - vec2(0.5)) - vec2(0.5)) / derivative;
			float line = min(g.x, g.y);
			float minX = min(derivative.x, 1.0);
			float minZ = min(derivative.y, 1.0);
			vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

			if ((p.z > -0.1 * minZ) && (p.z < 0.1 * minZ)) {
				color.x = 1.0;
			}

			if ((p.x > -0.1 * minX) && (p.x < 0.1 * minX)) {
				color.z = 1.0;
			}

			return color;
		}

		float depth(vec3 p) {
			vec4 clipSpace = viewProj * vec4(p, 1.0);

			return clipSpace.z / clipSpace.w;
		}

		float linearizeDepth(float depth) {
			float linearDepth = (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));

			return linearDepth / far;
		}

		void main() {
			float t = -nearPoint.y / (farPoint.y - nearPoint.y);
			vec3 fragPos = nearPoint + t * (farPoint - nearPoint);
			gl_FragDepth = (gl_DepthRange.diff * depth(fragPos) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
			float fading = max(0.5 - linearizeDepth(gl_FragDepth), 0.0);

			outColor = grid(fragPos, 10.0) * float(t > 0.0);
			outColor.a *= fading;
		}
		'''
		[gridFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, gridFragmentShaderCode)

		[self.gridProgram, _] = OpenGLHelper.createProgram(gridVertexShader, gridFragmentShader)

		# Cube
		self.cubeVertexBuffer = gl.glGenBuffers(1)
		cubeVertices = np.array([(-0.05, -0.05, -0.05), (0.05, -0.05, -0.05), (0.05, -0.05, 0.05), (-0.05, -0.05, 0.05), (-0.05, 0.05, -0.05), (0.05, 0.05, -0.05), (0.05, 0.05, 0.05), (-0.05, 0.05, 0.05)], dtype=np.float32)
		self.cubeVertexCount = len(cubeVertices)
		stride = cubeVertices.strides[0]
		offset = ctypes.c_void_p(0)
		vertexPositionLoc = gl.glGetAttribLocation(self.entityProgram, "position")
		gl.glEnableVertexAttribArray(vertexPositionLoc)
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
		gl.glVertexAttribPointer(vertexPositionLoc, 3, gl.GL_FLOAT, False, stride, offset)
		gl.glBufferData(gl.GL_ARRAY_BUFFER, cubeVertices.nbytes, cubeVertices, gl.GL_STATIC_DRAW)

		self.cubeIndexBuffer = gl.glGenBuffers(1)
		cubeIndices = np.array([0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 1, 2, 6, 1, 6, 5, 0, 3, 7, 0, 7, 4, 0, 1, 5, 0, 5, 4, 3, 2, 6, 3, 6, 7], dtype=np.uint32)
		self.cubeIndexCount = len(cubeIndices)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)
		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, cubeIndices.nbytes, cubeIndices, gl.GL_STATIC_DRAW)

		# Picking
		pickingFragmentShaderCode = '''
		#version 460

		uniform uint entityID;

		out uint outEntityID;

		void main() {
			outEntityID = entityID;
		}
		'''
		[pickingFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, pickingFragmentShaderCode)

		[self.pickingProgram, _] = OpenGLHelper.createProgram(entityVertexShader, pickingFragmentShader)

		self.pickingFramebuffer = gl.glGenFramebuffers(1)
		self.createPickingImages()

		# Outline
		outlineSoloFragmentShaderCode = '''
		#version 460

		out float outColor;

		void main() {
			outColor = 1.0;
		}
		'''
		[outlineSoloFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, outlineSoloFragmentShaderCode)

		[self.outlineSoloProgram, _] = OpenGLHelper.createProgram(entityVertexShader, outlineSoloFragmentShader)

		self.outlineSoloFramebuffer = gl.glGenFramebuffers(1)
		self.createOutlineSoloImages()

		outlineFragmentShaderCode = '''
		#version 460

		uniform sampler2D outlineSoloTexture;

		in vec2 uv;

		out vec4 outColor;

		void main() {
			float value = texture(outlineSoloTexture, uv).r;
			if (value == 1.0) {
				discard;
			}

			vec2 texelSize = 1.0 / vec2(textureSize(outlineSoloTexture, 0));
			bool foundValue = false;
			for (float range = 0.0; range < 2.0; range++) {
				float n = texture(outlineSoloTexture, uv + vec2(0.0, texelSize.y * (range + 1.0))).r;
				float s = texture(outlineSoloTexture, uv + vec2(0.0, -texelSize.y * (range + 1.0))).r;
				float e = texture(outlineSoloTexture, uv + vec2(-texelSize.x * (range + 1.0), 0.0)).r;
				float w = texture(outlineSoloTexture, uv + vec2(texelSize.x * (range + 1.0), 0.0)).r;
				if ((n == 1.0) || (s == 1.0) || (e == 1.0) || (w == 1.0)) {
					outColor = vec4(1.0, 1.0, 0.0, 1.0);
					foundValue = true;
					break;
				}
			}
			if (!foundValue) {
				discard;
			}
		}
		'''
		[outlineFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, outlineFragmentShaderCode)

		[self.outlineProgram, _] = OpenGLHelper.createProgram(fullscreenVertexShader, outlineFragmentShader)
		
		# Render
		self.waitTimer.start(16)

	def paintGL(self):
		if self.gotResized:
			self.createPickingImages()
			self.createOutlineSoloImages()

			self.gotResized = False

		self.updateCamera()
		viewMatrix = MathHelper.lookAtRH(self.camera.position, np.add(self.camera.position, self.camera.direction), [0.0, 1.0, 0.0])
		projectionMatrix = MathHelper.perspectiveRH(np.deg2rad(45.0), self.width() / self.height(), self.camera.nearPlane, self.camera.farPlane)
		viewProjMatrix = MathHelper.mat4x4Mult(projectionMatrix, viewMatrix)
		
		gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.defaultFramebufferObject())
		gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
		gl.glEnable(gl.GL_DEPTH_TEST)
		gl.glDepthFunc(gl.GL_LESS)
		gl.glDepthMask(gl.GL_TRUE)
		gl.glEnable(gl.GL_BLEND)
		gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

		# Entities
		for entity in globalInfo.entities:
			entityPosition = np.copy(entity.components["transform"].position)
			entityPosition[0] *= -1.0
			translationMatrix = MathHelper.translate(entityPosition)
			rotationMatrix = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[2]), [0.0, 0.0, 1.0])))
			scalingMatrix = MathHelper.scale(entity.components["transform"].scale)
			modelMatrix = MathHelper.mat4x4Mult(translationMatrix, MathHelper.mat4x4Mult(rotationMatrix, scalingMatrix))

			gl.glUseProgram(self.entityProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "viewProj"), 1, False, viewProjMatrix)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "model"), 1, False, modelMatrix)

			if "renderable" not in entity.components.keys():
				gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
				gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)
				
				gl.glDrawElements(gl.GL_TRIANGLES, self.cubeIndexCount, gl.GL_UNSIGNED_INT, None)

		# Grid
		gl.glUseProgram(self.gridProgram)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "view"), 1, False, viewMatrix)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "projection"), 1, False, projectionMatrix)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "viewProj"), 1, False, viewProjMatrix)
		gl.glUniform1f(gl.glGetUniformLocation(self.gridProgram, "near"), self.camera.nearPlane)
		gl.glUniform1f(gl.glGetUniformLocation(self.gridProgram, "far"), self.camera.farPlane)

		gl.glDrawArrays(gl.GL_TRIANGLES, 0, 6)

		# Picking
		if self.doPicking:
			gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.pickingFramebuffer)
			gl.glDrawBuffers(1, [gl.GL_COLOR_ATTACHMENT0])
			gl.glClearBufferuiv(gl.GL_COLOR, 0, np.iinfo(np.uint).max)
			gl.glClear(gl.GL_DEPTH_BUFFER_BIT)
			gl.glEnable(gl.GL_DEPTH_TEST)
			gl.glDepthFunc(gl.GL_LESS)
			gl.glDepthMask(gl.GL_TRUE)
			gl.glDisable(gl.GL_BLEND)

			for entity in globalInfo.entities:
				entityPosition = np.copy(entity.components["transform"].position)
				entityPosition[0] *= -1.0
				translationMatrix = MathHelper.translate(entityPosition)
				rotationMatrix = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[2]), [0.0, 0.0, 1.0])))
				scalingMatrix = MathHelper.scale(entity.components["transform"].scale)
				modelMatrix = MathHelper.mat4x4Mult(translationMatrix, MathHelper.mat4x4Mult(rotationMatrix, scalingMatrix))

				gl.glUseProgram(self.pickingProgram)
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "viewProj"), 1, False, viewProjMatrix)
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "model"), 1, False, modelMatrix)
				gl.glUniform1ui(gl.glGetUniformLocation(self.pickingProgram, "entityID"), entity.entityID)

				if "renderable" not in entity.components.keys():
					gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
					gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)
					
					gl.glDrawElements(gl.GL_TRIANGLES, self.cubeIndexCount, gl.GL_UNSIGNED_INT, None)
				
			cursorPosition = self.mapFromGlobal(QCursor.pos())
			pickedEntityID = gl.glReadPixels(cursorPosition.x() * globalInfo.devicePixelRatio, (self.height() - cursorPosition.y()) * globalInfo.devicePixelRatio, 1, 1, gl.GL_RED_INTEGER, gl.GL_UNSIGNED_INT)[0][0]
			if pickedEntityID != np.iinfo(np.uint32).max:
				globalInfo.currentEntityID = pickedEntityID
				globalInfo.signalEmitter.selectEntitySignal.emit(pickedEntityID)
			
			self.doPicking = False

		# Outline
		if globalInfo.currentEntityID != -1:
			# Outline Solo
			gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.outlineSoloFramebuffer)
			gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
			gl.glEnable(gl.GL_DEPTH_TEST)
			gl.glDepthFunc(gl.GL_LESS)
			gl.glDepthMask(gl.GL_TRUE)
			gl.glDisable(gl.GL_BLEND)

			entity = globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)]
			entityPosition = np.copy(entity.components["transform"].position)
			entityPosition[0] *= -1.0
			translationMatrix = MathHelper.translate(entityPosition)
			rotationMatrix = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[2]), [0.0, 0.0, 1.0])))
			scalingMatrix = MathHelper.scale(entity.components["transform"].scale)
			modelMatrix = MathHelper.mat4x4Mult(translationMatrix, MathHelper.mat4x4Mult(rotationMatrix, scalingMatrix))

			gl.glUseProgram(self.outlineSoloProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "viewProj"), 1, False, viewProjMatrix)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, modelMatrix)

			if "renderable" not in entity.components.keys():
				gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
				gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)
				
				gl.glDrawElements(gl.GL_TRIANGLES, self.cubeIndexCount, gl.GL_UNSIGNED_INT, None)

			# Outline
			gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.defaultFramebufferObject())
			gl.glDisable(gl.GL_DEPTH_TEST)
			gl.glEnable(gl.GL_DEPTH_TEST)
			gl.glDepthFunc(gl.GL_ALWAYS)

			gl.glUseProgram(self.outlineProgram)
			gl.glActiveTexture(gl.GL_TEXTURE0)
			gl.glBindTexture(gl.GL_TEXTURE_2D, self.outlineSoloImage)
			gl.glUniform1i(gl.glGetUniformLocation(self.outlineProgram, "outlineSoloTexture"), 0)

			gl.glBindBuffer(gl.GL_ARRAY_BUFFER, 0)

			gl.glDrawArrays(gl.GL_TRIANGLES, 0, 3)


	def updateCamera(self):
		deltaTime = self.waitTimer.interval() / 1000
		self.cameraYaw = (self.cameraYaw + self.mouseCursorDifference[0]) % 360.0
		self.cameraPitch = max(-89.0, min(89.0, self.cameraPitch + self.mouseCursorDifference[1]))

		cameraYawRad = np.deg2rad(self.cameraYaw)
		cameraPitchRad = np.deg2rad(self.cameraPitch)

		self.camera.direction = np.array([
			np.cos(cameraPitchRad) * np.cos(cameraYawRad),
			-np.sin(cameraPitchRad),
			np.cos(cameraPitchRad) * np.sin(cameraYawRad)
		])
		self.camera.direction = MathHelper.normalize(self.camera.direction)

		if self.forwardKeyPressed:
			self.camera.position = np.add(self.camera.position, self.camera.direction * self.camera.cameraSpeed * deltaTime)
		if self.backwardKeyPressed:
			self.camera.position = np.add(self.camera.position, self.camera.direction * -self.camera.cameraSpeed * deltaTime)
		if self.leftKeyPressed:
			t = MathHelper.normalize(np.array([-self.camera.direction[2], 0.0, self.camera.direction[0]]))
			self.camera.position = np.add(self.camera.position, t * -self.camera.cameraSpeed * deltaTime)
		if self.rightKeyPressed:
			t = MathHelper.normalize(np.array([-self.camera.direction[2], 0.0, self.camera.direction[0]]))
			self.camera.position = np.add(self.camera.position, t * self.camera.cameraSpeed * deltaTime)
		if self.upKeyPressed:
			self.camera.position[1] += self.camera.cameraSpeed * deltaTime
		if self.downKeyPressed:
			self.camera.position[1] -= self.camera.cameraSpeed * deltaTime

		self.mouseCursorDifference = np.zeros(2, dtype=np.float32)

	def createPickingImages(self):
		gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.pickingFramebuffer)

		self.pickingImage = gl.glGenTextures(1)
		gl.glBindTexture(gl.GL_TEXTURE_2D, self.pickingImage)
		gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_R32UI, int(self.width() * globalInfo.devicePixelRatio), int(self.height() * globalInfo.devicePixelRatio), 0, gl.GL_RED_INTEGER, gl.GL_UNSIGNED_INT, None)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_EDGE)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_EDGE)
		gl.glFramebufferTexture(gl.GL_FRAMEBUFFER, gl.GL_COLOR_ATTACHMENT0, self.pickingImage, 0)

		self.pickingDepthImage = gl.glGenRenderbuffers(1)
		gl.glBindRenderbuffer(gl.GL_RENDERBUFFER, self.pickingDepthImage)
		gl.glRenderbufferStorage(gl.GL_RENDERBUFFER, gl.GL_DEPTH_COMPONENT, int(self.width() * globalInfo.devicePixelRatio), int(self.height() * globalInfo.devicePixelRatio))
		gl.glFramebufferRenderbuffer(gl.GL_FRAMEBUFFER, gl.GL_DEPTH_ATTACHMENT, gl.GL_RENDERBUFFER, self.pickingDepthImage)

	def createOutlineSoloImages(self):
		gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.outlineSoloFramebuffer)

		self.outlineSoloImage = gl.glGenTextures(1)
		gl.glBindTexture(gl.GL_TEXTURE_2D, self.outlineSoloImage)
		gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RED, int(self.width() * globalInfo.devicePixelRatio), int(self.height() * globalInfo.devicePixelRatio), 0, gl.GL_RED, gl.GL_FLOAT, None)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_EDGE)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_EDGE)
		gl.glFramebufferTexture(gl.GL_FRAMEBUFFER, gl.GL_COLOR_ATTACHMENT0, self.outlineSoloImage, 0)

		self.outlineSoloDepthImage = gl.glGenRenderbuffers(1)
		gl.glBindRenderbuffer(gl.GL_RENDERBUFFER, self.outlineSoloDepthImage)
		gl.glRenderbufferStorage(gl.GL_RENDERBUFFER, gl.GL_DEPTH_COMPONENT, int(self.width() * globalInfo.devicePixelRatio), int(self.height() * globalInfo.devicePixelRatio))
		gl.glFramebufferRenderbuffer(gl.GL_FRAMEBUFFER, gl.GL_DEPTH_ATTACHMENT, gl.GL_RENDERBUFFER, self.outlineSoloDepthImage)

	def keyPressEvent(self, e):
		if e.key() == self.forwardKey:
			self.forwardKeyPressed = True
		if e.key() == self.backwardKey:
			self.backwardKeyPressed = True
		if e.key() == self.leftKey:
			self.leftKeyPressed = True
		if e.key() == self.rightKey:
			self.rightKeyPressed = True
		if e.key() == self.upKey:
			self.upKeyPressed = True
		if e.key() == self.downKey:
			self.downKeyPressed = True
		e.accept()

	def keyReleaseEvent(self, e):
		if e.key() == self.forwardKey:
			self.forwardKeyPressed = False
		if e.key() == self.backwardKey:
			self.backwardKeyPressed = False
		if e.key() == self.leftKey:
			self.leftKeyPressed = False
		if e.key() == self.rightKey:
			self.rightKeyPressed = False
		if e.key() == self.upKey:
			self.upKeyPressed = False
		if e.key() == self.downKey:
			self.downKeyPressed = False
		e.accept()

	def mouseMoveEvent(self, e):
		if e.buttons() & Qt.MouseButton.LeftButton:
			mouseCursorCurrentPosition = np.array([e.pos().x(), e.pos().y()])
			self.mouseCursorDifference = np.subtract(mouseCursorCurrentPosition, self.mouseCursorPreviousPosition)
			self.mouseCursorPreviousPosition = mouseCursorCurrentPosition
		e.accept()

	def mousePressEvent(self, e):
		if e.button() == Qt.MouseButton.LeftButton:
			self.setCursor(Qt.CursorShape.BlankCursor)
			widgetCenter = self.mapToGlobal(QPoint(int(self.width() / 2), int(self.height() / 2)))
			QCursor.setPos(widgetCenter)
			self.mouseCursorPreviousPosition = np.array([self.width() / 2, self.height() / 2])
		elif e.button() == Qt.MouseButton.RightButton:
			self.doPicking = True
		e.accept()

	def mouseReleaseEvent(self, e):
		if e.button() == Qt.MouseButton.LeftButton:
			self.setCursor(Qt.CursorShape.ArrowCursor)
			widgetCenter = self.mapToGlobal(QPoint(int(self.width() / 2), int(self.height() / 2)))
			QCursor.setPos(widgetCenter)
			self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
		e.accept()

	def focusOutEvent(self, e):
		self.forwardKeyPressed = False
		self.backwardKeyPressed = False
		self.leftKeyPressed = False
		self.rightKeyPressed = False
		self.upKeyPressed = False
		self.downKeyPressed = False
		e.accept()

	def resizeEvent(self, e):
		super().resizeEvent(e)
		self.gotResized = True

class CreateEntityCommand(QUndoCommand):
	def __init__(self, name):
		super().__init__()
		self.setText("Create Entity " + name)
		self.entityName = name
		self.entityID = -1

	def undo(self):
		del globalInfo.entities[self.index]
		globalInfo.signalEmitter.destroyEntitySignal.emit(self.entityID)

	def redo(self):
		newEntity = Entity(self.entityName, self.entityID)
		self.entityID = newEntity.entityID
		newEntity.components["transform"] = Transform()
		globalInfo.entities.append(newEntity)
		self.index = len(globalInfo.entities) - 1
		globalInfo.signalEmitter.createEntitySignal.emit(newEntity.entityID)

class DestroyEntityCommand(QUndoCommand):
	def __init__(self, entityID):
		super().__init__()
		self.setText("Destroy Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.destroyedEntity = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(entityID)])

	def undo(self):
		globalInfo.entities.insert(self.index, self.destroyedEntity)
		globalInfo.signalEmitter.createEntitySignal.emit(self.destroyedEntity.entityID)

	def redo(self):
		self.index = globalInfo.findEntityById(self.destroyedEntity.entityID)
		del globalInfo.entities[self.index]
		globalInfo.signalEmitter.destroyEntitySignal.emit(self.destroyedEntity.entityID)

class ChangeNameEntityCommand(QUndoCommand):
	def __init__(self, entityID, name):
		super().__init__()
		self.setText("Change Entity Name to " + name)
		self.entityID = entityID
		self.previousEntityName = globalInfo.entities[globalInfo.findEntityById(entityID)].name
		self.newEntityName = name

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].name = self.previousEntityName
		globalInfo.signalEmitter.changeNameEntitySignal.emit(self.entityID, self.previousEntityName)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].name = self.newEntityName
		globalInfo.signalEmitter.changeNameEntitySignal.emit(self.entityID, self.newEntityName)

class ChangePersistenceEntityCommand(QUndoCommand):
	def __init__(self, entityID, isPersistent):
		super().__init__()
		self.setText("Change Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name + " Persistence to " + str(isPersistent))
		self.entityID = entityID
		self.previousPersistence = globalInfo.entities[globalInfo.findEntityById(entityID)].isPersistent
		self.newPersistence = isPersistent

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].isPersistent = self.previousPersistence
		globalInfo.signalEmitter.changePersistenceEntitySignal.emit(self.entityID, self.previousPersistence)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].isPersistent = self.newPersistence
		globalInfo.signalEmitter.changePersistenceEntitySignal.emit(self.entityID, self.newPersistence)

class ChangeTransformEntityCommand(QUndoCommand):
	def __init__(self, entityID, transform):
		super().__init__()
		self.setText("Change Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name + " Transform component")
		self.entityID = entityID
		self.previousTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"])
		self.newTransform = copy.deepcopy(transform)

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["transform"] = copy.deepcopy(self.previousTransform)
		globalInfo.signalEmitter.changeEntityTransformSignal.emit(self.entityID, self.previousTransform)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["transform"] = copy.deepcopy(self.newTransform)
		globalInfo.signalEmitter.changeEntityTransformSignal.emit(self.entityID, self.newTransform)

class EntityListMenu(QMenu):
	def __init__(self):
		super().__init__()
		self.newEntityAction = self.addAction("New Entity", self.newEntity)
		self.deleteEntityAction = self.addAction("Delete Entity", self.deleteEntity)

	def newEntity(self):
		globalInfo.undoStack.push(CreateEntityCommand("Entity_" + str(globalInfo.entityID)))

	def deleteEntity(self):
		globalInfo.undoStack.push(DestroyEntityCommand(globalInfo.currentEntityID))

class EntityListItem(QListWidgetItem):
	def __init__(self, entityID):
		super().__init__()
		self.setText(globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.entityID = entityID

class EntityList(QListWidget):
	def __init__(self):
		super().__init__()
		self.menu = EntityListMenu()
		self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
		self.customContextMenuRequested.connect(self.showMenu)
		self.itemSelectionChanged.connect(self.onItemSelectionChanged)
		globalInfo.signalEmitter.createEntitySignal.connect(self.onCreateEntity)
		globalInfo.signalEmitter.destroyEntitySignal.connect(self.onDestroyEntity)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.changeNameEntitySignal.connect(self.onChangeNameEntity)

	def findItemWithEntityID(self, entityID):
		for i in range(self.count()):
			if self.item(i).entityID == entityID:
				return self.item(i)
		return None

	def onCreateEntity(self, entityID):
		self.addItem(EntityListItem(entityID))

	def onDestroyEntity(self, entityID):
		self.takeItem(self.row(self.findItemWithEntityID(entityID)))

	def onSelectEntity(self, entityID):
		if entityID != -1:
			with QSignalBlocker(self) as signalBlocker:
				self.clearSelection()
				self.findItemWithEntityID(entityID).setSelected(True)
		
	def onChangeNameEntity(self, entityID, name):
		self.findItemWithEntityID(entityID).setText(name)

	def onItemSelectionChanged(self):
		if len(self.selectedItems()) != 0:
			globalInfo.currentEntityID = self.selectedItems()[0].entityID
			globalInfo.signalEmitter.selectEntitySignal.emit(self.selectedItems()[0].entityID)
		else:
			globalInfo.currentEntityID = -1
			globalInfo.signalEmitter.selectEntitySignal.emit(-1)
	
	def showMenu(self, e):
		if not self.itemAt(e):
			self.menu.deleteEntityAction.setEnabled(False)
		else:
			globalInfo.currentEntityID = self.itemAt(e).entityID
			self.menu.deleteEntityAction.setEnabled(True)
		self.menu.popup(QCursor.pos())

	def keyPressEvent(self, e):
		if len(self.selectedItems()) != 0:
			currentSelectionIndex = self.row(self.selectedItems()[0])
			if e.key() == Qt.Key.Key_Delete:
				globalInfo.undoStack.push(DestroyEntityCommand(self.selectedItems()[0].entityID))
			elif e.key() == Qt.Key.Key_Up:
				self.clearSelection()
				if currentSelectionIndex == 0:
					self.setCurrentItem(self.item(self.count() - 1))
				else:
					self.setCurrentItem(self.item(currentSelectionIndex - 1))
			elif e.key() == Qt.Key.Key_Down:
				self.clearSelection()
				if currentSelectionIndex == (self.count() - 1):
					self.setCurrentItem(self.item(0))
				else:
					self.setCurrentItem(self.item(currentSelectionIndex + 1))
		e.accept()

class EntityPanel(QWidget):
	def __init__(self):
		super().__init__()
		self.resize(300, self.height())
		self.setMinimumWidth(50)
		self.setLayout(QVBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.entityList = EntityList()
		self.layout().addWidget(QLabel("Entity List"))
		self.layout().addWidget(self.entityList)

class Vector3Widget(QWidget):
	editingFinished = pyqtSignal(float, float, float)

	def __init__(self, name):
		super().__init__()
		self.name = name
		self.previousX = 0.0
		self.previousY = 0.0
		self.previousZ = 0.0
		useDot = QLocale(QLocale.Language.English, QLocale.Country.UnitedStates)
		doubleValidator = QDoubleValidator()
		doubleValidator.setLocale(useDot)
		doubleValidator.setNotation(QDoubleValidator.Notation.StandardNotation)
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.xLabel = QLabel("x:")
		self.layout().addWidget(self.xLabel)
		self.xLineEdit = QLineEdit("0.0")
		self.xLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.xLineEdit)
		self.yLabel = QLabel("y:")
		self.layout().addWidget(self.yLabel)
		self.yLineEdit = QLineEdit("0.0")
		self.yLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.yLineEdit)
		self.zLabel = QLabel("z:")
		self.layout().addWidget(self.zLabel)
		self.zLineEdit = QLineEdit("0.0")
		self.zLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.zLineEdit)
		self.xLineEdit.editingFinished.connect(self.onEditingFinished)
		self.yLineEdit.editingFinished.connect(self.onEditingFinished)
		self.zLineEdit.editingFinished.connect(self.onEditingFinished)

	def onEditingFinished(self):
		newX = float(self.xLineEdit.text())
		newY = float(self.yLineEdit.text())
		newZ = float(self.zLineEdit.text())
		if (self.previousX != newX) or (self.previousY != newY) or (self.previousZ != newZ):
			self.previousX = newX
			self.previousY = newY
			self.previousZ = newZ
			self.editingFinished.emit(newX, newY, newZ)

class ComponentSeparatorLine(QFrame):
	def __init__(self):
		super().__init__()
		self.setFrameShape(QFrame.Shape.HLine)
		self.setLineWidth(1)
		self.setStyleSheet("color: rgba(255, 255, 255, 120)")

class TransformComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(ComponentSeparatorLine())
		self.layout().addWidget(QLabel("<b>Transform</b>"))
		self.positionWidget = Vector3Widget("Position")
		self.layout().addWidget(self.positionWidget)
		self.rotationWidget = Vector3Widget("Rotation")
		self.layout().addWidget(self.rotationWidget)
		self.scaleWidget = Vector3Widget("Scale")
		self.layout().addWidget(self.scaleWidget)
		self.layout().addWidget(ComponentSeparatorLine())
		self.positionWidget.editingFinished.connect(self.onTransformUpdated)
		self.rotationWidget.editingFinished.connect(self.onTransformUpdated)
		self.scaleWidget.editingFinished.connect(self.onTransformUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.changeEntityTransformSignal.connect(self.onChangeEntityTransform)

	def onChangeEntityTransform(self, entityID, transform):
		if entityID == globalInfo.currentEntityID:
			self.positionWidget.xLineEdit.setText(str(transform.position[0]))
			self.positionWidget.yLineEdit.setText(str(transform.position[1]))
			self.positionWidget.zLineEdit.setText(str(transform.position[2]))
			self.rotationWidget.xLineEdit.setText(str(transform.rotation[0]))
			self.rotationWidget.yLineEdit.setText(str(transform.rotation[1]))
			self.rotationWidget.zLineEdit.setText(str(transform.rotation[2]))
			self.scaleWidget.xLineEdit.setText(str(transform.scale[0]))
			self.scaleWidget.yLineEdit.setText(str(transform.scale[1]))
			self.scaleWidget.zLineEdit.setText(str(transform.scale[2]))

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "transform" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				self.positionWidget.xLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].position[0]))
				self.positionWidget.yLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].position[1]))
				self.positionWidget.zLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].position[2]))
				self.rotationWidget.xLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].rotation[0]))
				self.rotationWidget.yLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].rotation[1]))
				self.rotationWidget.zLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].rotation[2]))
				self.scaleWidget.xLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].scale[0]))
				self.scaleWidget.yLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].scale[1]))
				self.scaleWidget.zLineEdit.setText(str(globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"].scale[2]))
			else:
				self.hide()

	def onTransformUpdated(self, x, y, z):
		newTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
		sender = self.sender()
		if sender == self.positionWidget:
			newTransform.position = [x, y, z]
		elif sender == self.rotationWidget:
			newTransform.rotation = [x, y, z]
		elif sender == self.scaleWidget:
			newTransform.scale = [x, y, z]
		globalInfo.undoStack.push(ChangeTransformEntityCommand(globalInfo.currentEntityID, newTransform))

class ComponentList(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.transformWidget = TransformComponentWidget()
		self.layout().addWidget(self.transformWidget)

class ComponentScrollArea(QScrollArea):
	def __init__(self):
		super().__init__()
		self.setWidgetResizable(True)
		self.componentList = ComponentList()
		self.setWidget(self.componentList)

class EntityInfoNameWidget(QLineEdit):
	def __init__(self):
		super().__init__()
		self.editingFinished.connect(self.onEditingFinished)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.changeNameEntitySignal.connect(self.onChangeNameEntity)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			self.setText(globalInfo.entities[globalInfo.findEntityById(entityID)].name)
			self.previousText = self.text()

	def onChangeNameEntity(self, entityID, name):
		if entityID == globalInfo.currentEntityID:
			self.previousText = self.text()
			self.setText(name)

	def onEditingFinished(self):
		if self.previousText != self.text():
			self.previousText = self.text()
			globalInfo.undoStack.push(ChangeNameEntityCommand(globalInfo.currentEntityID, self.text()))

class EntityInfoPersistenceWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignLeft)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.persistenceCheckBox = QCheckBox()
		self.layout().addWidget(self.persistenceCheckBox)
		self.layout().addWidget(QLabel("Is Persistent"))
		self.persistenceCheckBox.stateChanged.connect(self.onStateChange)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.changePersistenceEntitySignal.connect(self.onChangePersistenceEntity)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			with QSignalBlocker(self.persistenceCheckBox) as signalBlocker:
				self.persistenceCheckBox.setChecked(globalInfo.entities[globalInfo.findEntityById(entityID)].isPersistent)

	def onChangePersistenceEntity(self, entityID, isPersistent):
		if entityID == globalInfo.currentEntityID:
			with QSignalBlocker(self.persistenceCheckBox) as signalBlocker:
				self.persistenceCheckBox.setChecked(isPersistent)

	def onStateChange(self, state):
		globalInfo.undoStack.push(ChangePersistenceEntityCommand(globalInfo.currentEntityID, Qt.CheckState(state) == Qt.CheckState.Checked))

class EntityInfoPanel(QWidget):
	def __init__(self):
		super().__init__()
		self.resize(300, self.height())
		self.setMinimumWidth(50)
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(QLabel("Entity Info"))
		self.entityInfoName = EntityInfoNameWidget()
		self.entityInfoName.hide()
		self.layout().addWidget(self.entityInfoName)
		self.entityInfoPersistence = EntityInfoPersistenceWidget()
		self.entityInfoPersistence.hide()
		self.layout().addWidget(self.entityInfoPersistence)
		self.componentScrollArea = ComponentScrollArea()
		self.componentScrollArea.hide()
		self.layout().addWidget(self.componentScrollArea)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)

	def onSelectEntity(self, entityID):
		if entityID == -1:
			self.entityInfoName.hide()
			self.componentScrollArea.hide()
			self.entityInfoPersistence.hide()
		else:
			self.entityInfoName.show()
			self.componentScrollArea.show()
			self.entityInfoPersistence.show()

class MainWindow(QMainWindow):
	def __init__(self):
		super().__init__()
		self.resize(1280, 720)
		self.setWindowTitle("NutshellEngine Editor")
		self.main = QSplitter()
		self.setCentralWidget(self.main)
		self.createMenuBar()
		self.createEntityPanel()
		self.createRenderer()
		self.createEntityInfoPanel()
		self.show()
	
	def createMenuBar(self):
		self.fileMenu = FileMenu()
		self.editMenu = EditMenu()
		menuBar = self.menuBar()
		menuBar.addMenu(self.fileMenu)
		menuBar.addMenu(self.editMenu)

	def createEntityPanel(self):
		self.entityPanel = EntityPanel()
		self.main.addWidget(self.entityPanel)

	def createRenderer(self):
		self.renderer = Renderer()
		self.main.addWidget(self.renderer)

	def createEntityInfoPanel(self):
		self.entityInfoPanel = EntityInfoPanel()
		self.main.addWidget(self.entityInfoPanel)

if __name__ == "__main__":
	app = QApplication([])
	app.setStyle("Fusion")

	globalInfo.devicePixelRatio = app.devicePixelRatio()
	globalInfo.signalEmitter = SignalEmitter()

	window = MainWindow()
	window.setWindowIcon(QIcon("assets/icon.png"))
	globalInfo.window = window

	app.exec()