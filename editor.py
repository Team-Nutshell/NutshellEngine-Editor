import copy
import ctypes
import OpenGL.GL as gl
from PyQt6.QtCore import Qt, QObject, pyqtSignal, QSignalBlocker, QLocale, QPoint, QTimer
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QLabel, QHBoxLayout, QVBoxLayout, QMenu, QFileDialog, QMessageBox, QListWidget, QListWidgetItem, QLineEdit, QCheckBox, QScrollArea, QFrame, QSplitter, QSizePolicy, QPushButton
from PyQt6.QtGui import QFocusEvent, QKeyEvent, QMouseEvent, QResizeEvent, QUndoStack, QUndoCommand, QCursor, QIcon, QDoubleValidator, QKeySequence
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
import numpy as np
import configparser
import json

class Transform():
	def __init__(self):
		self.position = np.zeros(3, dtype=np.float32)
		self.rotation = np.zeros(3, dtype=np.float32)
		self.scale = np.ones(3, dtype=np.float32)

	def toJson(self):
		dictionary = {}
		dictionary["position"] = [float(self.position[0]), float(self.position[1]), float(self.position[2])]
		dictionary["rotation"] = [float(self.rotation[0]), float(self.rotation[1]), float(self.rotation[2])]
		dictionary["scale"] = [float(self.scale[0]), float(self.scale[1]), float(self.scale[2])]
		return dictionary

	def fromJson(self, jsonData):
		if "position" in jsonData:
			self.position = np.array(jsonData["position"], dtype=np.float32)
		if "rotation" in jsonData:
			self.rotation = np.array(jsonData["rotation"], dtype=np.float32)
		if "scale" in jsonData:
			self.scale = np.array(jsonData["scale"], dtype=np.float32)

	def modelMatrix(self):
		position = np.copy(self.position)
		position[0] *= -1.0
		translationMatrix = MathHelper.translate(position)
		rotationMatrix = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(self.rotation[2]), [0.0, 0.0, 1.0])))
		scalingMatrix = MathHelper.scale(self.scale)
		return MathHelper.mat4x4Mult(translationMatrix, MathHelper.mat4x4Mult(rotationMatrix, scalingMatrix))

class Renderable():
	def __init__(self):
		self.modelPath = ""

	def toJson(self):
		dictionary = {}
		dictionary["modelPath"] = self.modelPath
		return dictionary

	def fromJson(self, jsonData):
		if "modelPath" in jsonData:
			self.modelPath = jsonData["modelPath"]

class Entity():
	def __init__(self, name, entityID=-1):
		if entityID == -1:
			self.entityID = globalInfo.entityID
			globalInfo.entityID += 1
		else:
			self.entityID = entityID
		self.name = name
		self.isPersistent = False
		self.components = {"transform": Transform()}

	def toJson(self):
		dictionary = {}
		dictionary["name"] = self.name
		dictionary["isPersistent"] = self.isPersistent
		for componentName in self.components.keys():
			dictionary[componentName] = self.components[componentName].toJson()
		return dictionary

	def fromJson(self, jsonData):
		if "name" in jsonData:
			self.name = jsonData["name"]
		if "isPersistent" in jsonData:
			self.isPersistent = jsonData["isPersistent"]
		if "transform" in jsonData:
			self.components["transform"].fromJson(jsonData["transform"])
		if "renderable" in jsonData:
			self.components["renderable"] = Renderable()
			self.components["renderable"].fromJson(jsonData["renderable"])

class GlobalInfo():
	def __init__(self):
		self.entities = []
		self.currentEntityID = -1
		self.devicePixelRatio = 1.0
		self.window = None
		self.undoStack = QUndoStack()
		self.signalEmitter = None
		self.entityID = 0
		self.currentScenePath = ""

	def findEntityById(self, entityID):
		for i in range(len(self.entities)):
			if self.entities[i].entityID == entityID:
				return i
		return -1

	def entitiesToJson(self):
		dictionary = {}
		entitiesArray = []
		for entity in self.entities:
			entitiesArray.append(entity.toJson())
		dictionary["entities"] = entitiesArray
		return dictionary

globalInfo = GlobalInfo()

class SceneManager():
	@staticmethod
	def newScene():
		globalInfo.currentScenePath = ""
		globalInfo.window.setWindowTitle("NutshellEngine Editor")
		globalInfo.undoStack.push(ClearSceneCommand())

	@staticmethod
	def openScene(filePath):
		with open(filePath, "r") as f:
			try:
				sceneData = json.load(f)
			except:
				return
		SceneManager.newScene()
		globalInfo.currentScenePath = filePath
		globalInfo.window.setWindowTitle("NutshellEngine Editor - " + filePath)
		if "entities" in sceneData:
			entities = []
			for entity in sceneData["entities"]:
				newEntity = Entity("")
				newEntity.fromJson(entity)
				entities.append(newEntity)
			if len(entities) != 0:
				globalInfo.undoStack.push(OpenSceneCommand(entities))

	@staticmethod
	def saveScene(filePath):
		globalInfo.currentScenePath = filePath
		globalInfo.window.setWindowTitle("NutshellEngine Editor - " + filePath)
		with open(filePath, "w+", encoding="utf-8") as f:
			json.dump(globalInfo.entitiesToJson(), f, ensure_ascii=False, indent=4)

class SignalEmitter(QObject):
	createEntitySignal = pyqtSignal(int)
	destroyEntitySignal = pyqtSignal(int)
	selectEntitySignal = pyqtSignal(int)
	changeNameEntitySignal = pyqtSignal(int, str)
	changePersistenceEntitySignal = pyqtSignal(int, bool)
	changeEntityTransformSignal = pyqtSignal(int, Transform)
	addEntityRenderableSignal = pyqtSignal(int)
	removeEntityRenderableSignal = pyqtSignal(int)
	changeEntityRenderableSignal = pyqtSignal(int, Renderable)

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
		SceneManager.newScene()

class FileMenu(QMenu):
	def __init__(self):
		super().__init__("&File")
		self.newAction = self.addAction("New", self.new)
		self.newAction.setShortcut("Ctrl+N")
		self.openAction = self.addAction("Open...", self.open)
		self.openAction.setShortcut("Ctrl+O")
		self.openAction = self.addAction("Save", self.save)
		self.openAction.setShortcut("Ctrl+S")
		self.openAction = self.addAction("Save as...", self.saveAs)
		self.openAction.setShortcut("Shift+Ctrl+S")

	def new(self):
		NewMessageBox()

	def open(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle("Open...")
		fileDialog.setNameFilter("NutshellEngine Scene (*.ntsn)")
		file = None
		if fileDialog.exec():
			file = fileDialog.selectedFiles()[0]
			SceneManager.openScene(file)
		return file

	def save(self):
		if globalInfo.currentScenePath == "":
			self.saveAs()
		else:
			SceneManager.saveScene(globalInfo.currentScenePath)

	def saveAs(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle("Save as...")
		fileDialog.setDefaultSuffix("ntsn")
		file = None
		if fileDialog.exec():
			file = fileDialog.selectedFiles()[0]
			SceneManager.saveScene(file)

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
	def lookAtRH(fromPosition, toVector, upVector):
		tMf = np.subtract(toVector, fromPosition)
		forward = MathHelper.normalize(tMf)
		fXu = np.cross(forward, upVector)
		right = MathHelper.normalize(fXu)
		realUp = np.cross(right, forward)
		return np.array([right[0], realUp[0], -forward[0], 0.0,
					right[1], realUp[1], -forward[1], 0.0,
					right[2], realUp[2], -forward[2], 0.0,
					-np.dot(right, fromPosition), -np.dot(realUp, fromPosition), np.dot(forward, fromPosition), 1.0], dtype=np.float32)

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

	@staticmethod
	def mat4x4Vec4Mult(m, v):
		return np.array([
			m[0] * v[0] + m[4] * v[1] + m[8] * v[2] +  m[12] * v[3],
			m[1] * v[0] + m[5] * v[1] + m[9] * v[2] +  m[13] * v[3],
			m[2] * v[0] + m[6] * v[1] + m[10] * v[2] +  m[14] * v[3],
			m[3] * v[0] + m[7] * v[1] + m[11] * v[2] +  m[15] * v[3]], dtype=np.float32)

	@staticmethod
	def unproject(p, width, height, invViewMatrix, invProjMatrix):
		screenSpace = np.array([p[0] / width, p[1] / height], dtype=np.float32)
		clipSpace = np.subtract(screenSpace * 2.0, np.array([1.0, 1.0], dtype=np.float32))
		viewSpace = MathHelper.mat4x4Vec4Mult(invProjMatrix.reshape((16)), np.array([clipSpace[0], clipSpace[1], 0.0, 1.0], dtype=np.float32))
		worldSpace = MathHelper.mat4x4Vec4Mult(invViewMatrix.reshape((16)), viewSpace)
		worldSpace[0] *= -1.0

		return worldSpace[:3] / worldSpace[3]

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

		self.viewMatrix = None
		self.projectionMatrix = None
		self.viewProjMatrix = None
		self.invViewMatrix = None
		self.invProjMatrix = None

class Renderer(QOpenGLWidget):
	def __init__(self):
		super().__init__()
		self.setFocusPolicy(Qt.FocusPolicy.ClickFocus)
		self.setMouseTracking(True)

		self.camera = RendererCamera()

		self.cameraForwardKey = Qt.Key.Key_W
		self.cameraBackwardKey = Qt.Key.Key_S
		self.cameraLeftKey = Qt.Key.Key_A
		self.cameraRightKey = Qt.Key.Key_D
		self.cameraUpKey = Qt.Key.Key_Space
		self.cameraDownKey = Qt.Key.Key_Shift

		self.translateEntityKey = Qt.Key.Key_T
		self.rotateEntityKey = Qt.Key.Key_R
		self.scaleEntityKey = Qt.Key.Key_E

		config = configparser.ConfigParser()
		if config.read("assets/options.ini") != []:
			if "Renderer" in config:
				if "cameraForwardKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraForwardKey"])
					if not input.isEmpty():
						self.cameraForwardKey = input[0].key()
				if "cameraBackwardKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraBackwardKey"])
					if not input.isEmpty():
						self.cameraBackwardKey = input[0].key()
				if "cameraLeftKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraLeftKey"])
					if not input.isEmpty():
						self.cameraLeftKey = input[0].key()
				if "cameraRightKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraRightKey"])
					if not input.isEmpty():
						self.cameraRightKey = input[0].key()
				if "cameraUpKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraUpKey"])
					if not input.isEmpty():
						self.cameraUpKey = input[0].key()
				if "cameraDownKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["cameraDownKey"])
					if not input.isEmpty():
						self.cameraDownKey = input[0].key()
				if "translateEntityKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["translateEntityKey"])
					if not input.isEmpty():
						self.translateEntityKey = input[0].key()
				if "rotateEntityKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["rotateEntityKey"])
					if not input.isEmpty():
						self.rotateEntityKey = input[0].key()
				if "scaleEntityKey" in config["Renderer"]:
					input = QKeySequence.fromString(config["Renderer"]["scaleEntityKey"])
					if not input.isEmpty():
						self.scaleEntityKey = input[0].key()

		self.cameraForwardKeyPressed = False
		self.cameraBackwardKeyPressed = False
		self.cameraLeftKeyPressed = False
		self.cameraRightKeyPressed = False
		self.cameraUpKeyPressed = False
		self.cameraDownKeyPressed = False

		self.translateEntityKeyPressed = False
		self.rotateEntityKeyPressed = False
		self.scaleEntityKeyPressed = False

		self.leftClickedPressed = False

		self.mouseCursorPreviousPosition = np.array(2, dtype=np.float32)
		self.mouseCursorDifference = np.zeros(2, dtype=np.float32)

		self.cameraYaw = np.rad2deg(np.arctan2(self.camera.direction[2], self.camera.direction[0]))
		self.cameraPitch = np.rad2deg(-np.arcsin(self.camera.direction[1]))

		self.waitTimer = QTimer()
		self.waitTimer.timeout.connect(self.update)

		self.doPicking = False

		self.entityMoveTransform = None

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

		gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.defaultFramebufferObject())
		gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
		gl.glEnable(gl.GL_DEPTH_TEST)
		gl.glDepthFunc(gl.GL_LESS)
		gl.glDepthMask(gl.GL_TRUE)
		gl.glEnable(gl.GL_BLEND)
		gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

		# Entities
		gl.glUseProgram(self.entityProgram)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)

		for entity in globalInfo.entities:
			if entity.entityID == globalInfo.currentEntityID and self.entityMoveTransform is not None:
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
			else:
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

			gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
			gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)

			gl.glDrawElements(gl.GL_TRIANGLES, self.cubeIndexCount, gl.GL_UNSIGNED_INT, None)

		# Grid
		gl.glUseProgram(self.gridProgram)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "view"), 1, False, self.camera.viewMatrix)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "projection"), 1, False, self.camera.projectionMatrix)
		gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)
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

			gl.glUseProgram(self.pickingProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)

			for entity in globalInfo.entities:
				if entity.entityID == globalInfo.currentEntityID and self.entityMoveTransform is not None:
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
				else:
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

				gl.glUniform1ui(gl.glGetUniformLocation(self.pickingProgram, "entityID"), entity.entityID)

				gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.cubeVertexBuffer)
				gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.cubeIndexBuffer)

				gl.glDrawElements(gl.GL_TRIANGLES, self.cubeIndexCount, gl.GL_UNSIGNED_INT, None)

			cursorPosition = self.mapFromGlobal(QCursor.pos())
			pickedEntityID = gl.glReadPixels(cursorPosition.x() * globalInfo.devicePixelRatio, (self.height() - cursorPosition.y()) * globalInfo.devicePixelRatio, 1, 1, gl.GL_RED_INTEGER, gl.GL_UNSIGNED_INT)[0][0]
			if pickedEntityID != np.iinfo(np.uint32).max:
				globalInfo.currentEntityID = pickedEntityID
			else:
				globalInfo.currentEntityID = -1
			globalInfo.signalEmitter.selectEntitySignal.emit(globalInfo.currentEntityID)

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

			gl.glUseProgram(self.outlineSoloProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)

			entity = globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)]
			if entity.entityID == globalInfo.currentEntityID and self.entityMoveTransform is not None:
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
			else:
				gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

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
		if self.anyEntityTransformKeyPressed():
			return

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

		if self.cameraForwardKeyPressed:
			self.camera.position = np.add(self.camera.position, self.camera.direction * self.camera.cameraSpeed * deltaTime)
		if self.cameraBackwardKeyPressed:
			self.camera.position = np.add(self.camera.position, self.camera.direction * -self.camera.cameraSpeed * deltaTime)
		if self.cameraLeftKeyPressed:
			t = MathHelper.normalize(np.array([-self.camera.direction[2], 0.0, self.camera.direction[0]]))
			self.camera.position = np.add(self.camera.position, t * -self.camera.cameraSpeed * deltaTime)
		if self.cameraRightKeyPressed:
			t = MathHelper.normalize(np.array([-self.camera.direction[2], 0.0, self.camera.direction[0]]))
			self.camera.position = np.add(self.camera.position, t * self.camera.cameraSpeed * deltaTime)
		if self.cameraUpKeyPressed:
			self.camera.position[1] += self.camera.cameraSpeed * deltaTime
		if self.cameraDownKeyPressed:
			self.camera.position[1] -= self.camera.cameraSpeed * deltaTime

		self.camera.viewMatrix = MathHelper.lookAtRH(self.camera.position, np.add(self.camera.position, self.camera.direction), [0.0, 1.0, 0.0])
		self.camera.projectionMatrix = MathHelper.perspectiveRH(np.deg2rad(45.0), self.width() / self.height(), self.camera.nearPlane, self.camera.farPlane)
		self.camera.viewProjMatrix = MathHelper.mat4x4Mult(self.camera.projectionMatrix, self.camera.viewMatrix)
		self.camera.invViewMatrix = np.linalg.inv(np.copy(self.camera.viewMatrix).reshape((4, 4)))
		self.camera.invProjMatrix = np.linalg.inv(np.copy(self.camera.projectionMatrix).reshape((4, 4)))

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

	def anyEntityTransformKeyPressed(self):
		return self.translateEntityKeyPressed or self.rotateEntityKeyPressed or self.scaleEntityKeyPressed

	def keyPressEvent(self, e):
		if e.isAutoRepeat():
			e.accept()
			return
		if e.key() == self.cameraForwardKey:
			self.cameraForwardKeyPressed = True
		if e.key() == self.cameraBackwardKey:
			self.cameraBackwardKeyPressed = True
		if e.key() == self.cameraLeftKey:
			self.cameraLeftKeyPressed = True
		if e.key() == self.cameraRightKey:
			self.cameraRightKeyPressed = True
		if e.key() == self.cameraUpKey:
			self.cameraUpKeyPressed = True
		if e.key() == self.cameraDownKey:
			self.cameraDownKeyPressed = True
		if e.key() == self.translateEntityKey:
			if globalInfo.currentEntityID != -1 and not self.leftClickedPressed and not self.anyEntityTransformKeyPressed():
				self.translateEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()])
		if e.key() == self.rotateEntityKey:
			if globalInfo.currentEntityID != -1 and not self.leftClickedPressed and not self.anyEntityTransformKeyPressed():
				self.rotateEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()])
		if e.key() == self.scaleEntityKey:
			if globalInfo.currentEntityID != -1 and not self.leftClickedPressed and not self.anyEntityTransformKeyPressed():
				self.scaleEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()])
		e.accept()

	def keyReleaseEvent(self, e):
		if e.isAutoRepeat():
			e.accept()
			return
		if e.key() == self.cameraForwardKey:
			self.cameraForwardKeyPressed = False
		if e.key() == self.cameraBackwardKey:
			self.cameraBackwardKeyPressed = False
		if e.key() == self.cameraLeftKey:
			self.cameraLeftKeyPressed = False
		if e.key() == self.cameraRightKey:
			self.cameraRightKeyPressed = False
		if e.key() == self.cameraUpKey:
			self.cameraUpKeyPressed = False
		if e.key() == self.cameraDownKey:
			self.cameraDownKeyPressed = False
		if e.key() == self.translateEntityKey:
			if self.translateEntityKeyPressed:
				self.translateEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeTransformEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		if e.key() == self.rotateEntityKey:
			if self.rotateEntityKeyPressed:
				self.rotateEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeTransformEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		if e.key() == self.scaleEntityKey:
			if self.scaleEntityKeyPressed:
				self.scaleEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeTransformEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		e.accept()

	def mouseMoveEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.buttons() & Qt.MouseButton.LeftButton:
				if self.leftClickedPressed:
					mouseCursorCurrentPosition = np.array([e.pos().x(), e.pos().y()])
					self.mouseCursorDifference = np.subtract(mouseCursorCurrentPosition, self.mouseCursorPreviousPosition)
					self.mouseCursorPreviousPosition = mouseCursorCurrentPosition
		else:
			if globalInfo.currentEntityID != -1:
				mouseCursorCurrentPosition = np.array([e.pos().x(), self.height() - e.pos().y()])
				if self.translateEntityKeyPressed:
					worldSpaceCursorCurrentPosition = MathHelper.unproject(mouseCursorCurrentPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorPreviousPosition = MathHelper.unproject(self.mouseCursorPreviousPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorDifference = np.subtract(worldSpaceCursorCurrentPosition, worldSpaceCursorPreviousPosition)
					cameraEntityDifference = np.subtract(self.entityMoveTransform.position, self.camera.position)
					if (np.dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0) and (np.dot(cameraEntityDifference, cameraEntityDifference) != 0.0):
						worldSpaceCursorDifferenceNormalized = MathHelper.normalize(worldSpaceCursorDifference)
						worldSpaceCursorDifferenceLength = np.linalg.norm(worldSpaceCursorDifference)
						cameraEntityDifferenceLength = np.linalg.norm(cameraEntityDifference)
						coefficient = (cameraEntityDifferenceLength * worldSpaceCursorDifferenceLength) / self.camera.nearPlane
						self.entityMoveTransform.position += worldSpaceCursorDifferenceNormalized * coefficient
				elif self.rotateEntityKeyPressed:
					rotationMatrix = MathHelper.rotate((mouseCursorCurrentPosition[0] - self.mouseCursorPreviousPosition[0]) / self.width(), self.camera.direction)
					rotationAngles = np.array([np.rad2deg(np.arctan2(rotationMatrix[9], rotationMatrix[10])), np.rad2deg(np.arctan2(-rotationMatrix[8], np.sqrt((rotationMatrix[9] * rotationMatrix[9]) + (rotationMatrix[10] * rotationMatrix[10])))), np.rad2deg(np.arctan2(rotationMatrix[4], rotationMatrix[0]))], dtype=np.float32)
					self.entityMoveTransform.rotation -= rotationAngles
					self.entityMoveTransform.rotation %= 360.0
				elif self.scaleEntityKeyPressed:
					mousePositionDifference = np.divide(np.subtract(mouseCursorCurrentPosition, self.mouseCursorPreviousPosition), np.array([self.width(), self.height()], dtype=np.float32))
					if np.dot(mousePositionDifference, mousePositionDifference) != 0.0:
						center = np.array([self.width() / 2, self.height() / 2], dtype=np.float32)
						self.entityMoveTransform.scale += np.linalg.norm(mousePositionDifference) * (1.0 if np.dot(mousePositionDifference, np.subtract(mouseCursorCurrentPosition, center)) > 0.0 else -1.0)
				self.mouseCursorPreviousPosition = mouseCursorCurrentPosition
		e.accept()

	def mousePressEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.button() == Qt.MouseButton.LeftButton:
				self.leftClickedPressed = True
				self.setCursor(Qt.CursorShape.BlankCursor)
				widgetCenter = self.mapToGlobal(QPoint(int(self.width() / 2), int(self.height() / 2)))
				QCursor.setPos(widgetCenter)
				self.mouseCursorPreviousPosition = np.array([self.width() / 2, self.height() / 2])
			elif e.button() == Qt.MouseButton.RightButton:
				self.doPicking = True
		e.accept()

	def mouseReleaseEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.button() == Qt.MouseButton.LeftButton:
				if self.leftClickedPressed:
					self.leftClickedPressed = False
					self.setCursor(Qt.CursorShape.ArrowCursor)
					widgetCenter = self.mapToGlobal(QPoint(int(self.width() / 2), int(self.height() / 2)))
					QCursor.setPos(widgetCenter)
					self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
		e.accept()

	def focusOutEvent(self, e):
		self.cameraForwardKeyPressed = False
		self.cameraBackwardKeyPressed = False
		self.cameraLeftKeyPressed = False
		self.cameraRightKeyPressed = False
		self.cameraUpKeyPressed = False
		self.cameraDownKeyPressed = False
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

class ClearSceneCommand(QUndoCommand):
	def __init__(self):
		super().__init__()
		self.setText("Clear Scene")
		self.previousEntities = copy.deepcopy(globalInfo.entities)

	def undo(self):
		globalInfo.entities = copy.deepcopy(self.previousEntities)
		for i in range(len(self.previousEntities)):
			globalInfo.signalEmitter.createEntitySignal.emit(self.previousEntities[i].entityID)

	def redo(self):
		while len(globalInfo.entities) != 0:
			destroyedEntityID = globalInfo.entities[-1].entityID
			del globalInfo.entities[-1]
			globalInfo.signalEmitter.destroyEntitySignal.emit(destroyedEntityID)

class OpenSceneCommand(QUndoCommand):
	def __init__(self, entities):
		super().__init__()
		self.setText("Open Scene")
		self.newEntities = copy.deepcopy(entities)

	def undo(self):
		while len(globalInfo.entities) != 0:
			destroyedEntityID = globalInfo.entities[-1].entityID
			del globalInfo.entities[-1]
			globalInfo.signalEmitter.destroyEntitySignal.emit(destroyedEntityID)

	def redo(self):
		globalInfo.entities = copy.deepcopy(self.newEntities)
		for i in range(len(self.newEntities)):
			globalInfo.signalEmitter.createEntitySignal.emit(self.newEntities[i].entityID)

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

class AddRenderableEntityCommand(QUndoCommand):
	def __init__(self, entityID):
		super().__init__()
		self.setText("Add Renderable Component to Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.entityID = entityID

	def undo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"]
		globalInfo.signalEmitter.removeEntityRenderableSignal.emit(self.entityID)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"] = Renderable()
		globalInfo.signalEmitter.addEntityRenderableSignal.emit(self.entityID)

class RemoveRenderableEntityCommand(QUndoCommand):
	def __init__(self, entityID):
		super().__init__()
		self.setText("Remove Renderable Component to Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.entityID = entityID
		self.renderable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"])

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"] = copy.deepcopy(self.renderable)
		globalInfo.signalEmitter.addEntityRenderableSignal.emit(self.entityID)

	def redo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"]
		globalInfo.signalEmitter.removeEntityRenderableSignal.emit(self.entityID)

class ChangeRenderableEntityCommand(QUndoCommand):
	def __init__(self, entityID, renderable):
		super().__init__()
		self.setText("Change Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name + " Renderable component")
		self.entityID = entityID
		self.previousRenderable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(entityID)].components["renderable"])
		self.newRenderable = copy.deepcopy(renderable)

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"] = copy.deepcopy(self.previousRenderable)
		globalInfo.signalEmitter.changeEntityRenderableSignal.emit(self.entityID, self.previousRenderable)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"] = copy.deepcopy(self.newRenderable)
		globalInfo.signalEmitter.changeEntityRenderableSignal.emit(self.entityID, self.newRenderable)

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
		sizePolicy = QSizePolicy()
		sizePolicy.setHorizontalPolicy(QSizePolicy.Policy.Ignored)
		sizePolicy.setVerticalPolicy(QSizePolicy.Policy.Expanding)
		self.setSizePolicy(sizePolicy)
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
		with QSignalBlocker(self) as signalBlocker:
			self.clearSelection()
		if entityID != -1:
			with QSignalBlocker(self) as signalBlocker:
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
		if e.isAutoRepeat():
			e.accept()
			return
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
		self.resize(100, self.height())
		self.setMinimumWidth(50)
		self.setLayout(QVBoxLayout())
		self.layout().setContentsMargins(2, 2, 0, 2)
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

class FileSelectorWidget(QWidget):
	fileSelected = pyqtSignal(str)

	def __init__(self, noFileText, buttonText):
		super().__init__()
		self.filePath = ""
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.filePathLabel = QLabel(noFileText)
		self.layout().addWidget(self.filePathLabel)
		self.filePathButton = QPushButton(buttonText)
		self.filePathButton.clicked.connect(self.onFilePathButtonClicked)
		self.layout().addWidget(self.filePathButton)

	def onFilePathButtonClicked(self):
		fileDialog = QFileDialog()
		if self.filePath != "":
			fileDialog.setDirectory(self.filePath.rsplit("/", 1)[0])
		fileDialog.setWindowTitle(self.filePathButton.text())
		if fileDialog.exec():
			self.filePath = fileDialog.selectedFiles()[0]
			self.filePathLabel.setText(self.filePath.rsplit("/")[-1])
			self.fileSelected.emit(self.filePath)

class ComponentSeparatorLine(QFrame):
	def __init__(self):
		super().__init__()
		self.setFrameShape(QFrame.Shape.HLine)
		self.setLineWidth(1)
		self.setStyleSheet("color: rgba(255, 255, 255, 120)")

class ComponentTitleWidget(QWidget):
	def __init__(self, name):
		super().__init__()
		self.name = name
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		if self.name != "Transform":
			self.removeWidgetButton = QPushButton("X")
			self.removeWidgetButton.setFixedWidth(20)
			self.removeWidgetButton.clicked.connect(self.onClick)
			self.layout().addWidget(self.removeWidgetButton)
		self.layout().addWidget(QLabel("<b>" + self.name + "</b>"))

	def onClick(self):
		if self.name == "Renderable":
			globalInfo.undoStack.push(RemoveRenderableEntityCommand(globalInfo.currentEntityID))

class TransformComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(ComponentSeparatorLine())
		self.componentTitle = ComponentTitleWidget("Transform")
		self.layout().addWidget(self.componentTitle)
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

	def updateWidgets(self, transform):
		self.positionWidget.xLineEdit.setText(format(transform.position[0], ".3f"))
		self.positionWidget.yLineEdit.setText(format(transform.position[1], ".3f"))
		self.positionWidget.zLineEdit.setText(format(transform.position[2], ".3f"))
		self.rotationWidget.xLineEdit.setText(format(transform.rotation[0], ".3f"))
		self.rotationWidget.yLineEdit.setText(format(transform.rotation[1], ".3f"))
		self.rotationWidget.zLineEdit.setText(format(transform.rotation[2], ".3f"))
		self.scaleWidget.xLineEdit.setText(format(transform.scale[0], ".3f"))
		self.scaleWidget.yLineEdit.setText(format(transform.scale[1], ".3f"))
		self.scaleWidget.zLineEdit.setText(format(transform.scale[2], ".3f"))

	def onChangeEntityTransform(self, entityID, transform):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(transform)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "transform" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				transform = globalInfo.entities[globalInfo.findEntityById(entityID)].components["transform"]
				self.updateWidgets(transform)
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

class RenderableComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Renderable")
		self.layout().addWidget(self.componentTitle)
		self.modelFileSelection = FileSelectorWidget("No model path", "Choose model")
		self.layout().addWidget(self.modelFileSelection)
		self.layout().addWidget(ComponentSeparatorLine())
		self.modelFileSelection.fileSelected.connect(self.onRenderableUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityRenderableSignal.connect(self.onAddEntityRenderable)
		globalInfo.signalEmitter.removeEntityRenderableSignal.connect(self.onRemoveEntityRenderable)
		globalInfo.signalEmitter.changeEntityRenderableSignal.connect(self.onChangeEntityRenderable)

	def updateWidgets(self, renderable):
		if renderable.modelPath != "":
			self.modelFileSelection.filePathLabel.setText(renderable.modelPath.rsplit("/")[-1])

	def onAddEntityRenderable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			renderable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["renderable"]
			self.updateWidgets(renderable)
			self.show()

	def onRemoveEntityRenderable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityRenderable(self, entityID, renderable):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(renderable)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "renderable" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				renderable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["renderable"]
				self.updateWidgets(renderable)
			else:
				self.hide()

	def onRenderableUpdated(self, filePath):
		newRenderable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["renderable"])
		newRenderable.modelPath = filePath
		globalInfo.undoStack.push(ChangeRenderableEntityCommand(globalInfo.currentEntityID, newRenderable))

class ComponentList(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.transformWidget = TransformComponentWidget()
		self.layout().addWidget(self.transformWidget)
		self.renderableWidget = RenderableComponentWidget()
		self.layout().addWidget(self.renderableWidget)

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
		self.resize(175, self.height())
		self.setMinimumWidth(50)
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 2, 2, 2)
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
		if entityID != -1:
			self.entityInfoName.show()
			self.componentScrollArea.show()
			self.entityInfoPersistence.show()
		else:
			self.entityInfoName.hide()
			self.componentScrollArea.hide()
			self.entityInfoPersistence.hide()

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