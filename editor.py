import copy
import ctypes
import OpenGL.GL as gl
from PyQt6.QtCore import Qt, QObject, pyqtSignal, QSignalBlocker, QLocale, QPoint, QTimer
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QLabel, QHBoxLayout, QVBoxLayout, QMenu, QFileDialog, QMessageBox, QListWidget, QListWidgetItem, QLineEdit, QCheckBox, QScrollArea, QFrame, QSplitter, QSizePolicy, QPushButton, QComboBox, QColorDialog
from PyQt6.QtGui import QFocusEvent, QKeyEvent, QMouseEvent, QResizeEvent, QUndoStack, QUndoCommand, QCursor, QIcon, QDoubleValidator, QKeySequence, QColor, QPalette
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
import numpy as np
from PIL import Image
from pygltflib import GLTF2
import struct
import configparser
import json
import re
import os

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
		translationMatrix = MathHelper.translate(self.position)
		rotationMatrix = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(self.rotation[2]), [0.0, 0.0, 1.0])))
		scalingMatrix = MathHelper.scale(self.scale)
		return MathHelper.mat4x4Mult(translationMatrix, MathHelper.mat4x4Mult(rotationMatrix, scalingMatrix))

class Camera():
	def __init__(self):
		self.forward = np.array([0.0, 0.0, -1.0], dtype=np.float32)
		self.up = np.array([0.0, 1.0, 0.0], dtype=np.float32)
		self.fov = 45.0
		self.nearPlane = 0.3
		self.farPlane = 200.0

	def toJson(self):
		dictionary = {}
		dictionary["forward"] = [float(self.forward[0]), float(self.forward[1]), float(self.forward[2])]
		dictionary["up"] = [float(self.up[0]), float(self.up[1]), float(self.up[2])]
		dictionary["fov"] = self.fov
		dictionary["nearPlane"] = self.nearPlane
		dictionary["farPlane"] = self.farPlane
		return dictionary

	def fromJson(self, jsonData):
		if "forward" in jsonData:
			self.forward = np.array(jsonData["forward"], dtype=np.float32)
		if "up" in jsonData:
			self.up = np.array(jsonData["up"], dtype=np.float32)
		if "fov" in jsonData:
			self.fov = jsonData["fov"]
		if "nearPlane" in jsonData:
			self.nearPlane = jsonData["nearPlane"]
		if "farPlane" in jsonData:
			self.farPlane = jsonData["farPlane"]

class Light():
	def __init__(self):
		self.type = "Directional"
		self.color = np.array([1.0, 1.0, 1.0], dtype=np.float32)
		self.direction = np.array([0.0, -1.0, 0.0], dtype=np.float32)
		self.cutoff = np.array([10.0, 20.0], dtype=np.float32)

	def toJson(self):
		dictionary = {}
		dictionary["type"] = self.type
		dictionary["color"] = [float(self.color[0]), float(self.color[1]), float(self.color[2])]
		dictionary["direction"] = [float(self.direction[0]), float(self.direction[1]), float(self.direction[2])]
		dictionary["cutoff"] = [float(self.cutoff[0]), float(self.cutoff[1])]
		return dictionary

	def fromJson(self, jsonData):
		if "type" in jsonData:
			self.type = jsonData["type"]
		if "color" in jsonData:
			self.color = np.array(jsonData["color"], dtype=np.float32)
		if "direction" in jsonData:
			self.direction = np.array(jsonData["direction"], dtype=np.float32)
		if "cutoff" in jsonData:
			self.cutoff = np.array(jsonData["cutoff"], dtype=np.float32)

class Renderable():
	def __init__(self):
		self.modelPath = ""

	def toJson(self):
		dictionary = {}
		dictionary["modelPath"] = self.modelPath
		if globalInfo.projectDirectory != "":
			if self.modelPath.startswith(globalInfo.projectDirectory):
				dictionary["modelPath"] = self.modelPath[len(globalInfo.projectDirectory) + 1:]
		return dictionary

	def fromJson(self, jsonData):
		if "modelPath" in jsonData:
			self.modelPath = jsonData["modelPath"]

class Rigidbody():
	def __init__(self):
		self.isStatic = False
		self.isAffectedByConstants = True
		self.lockRotation = False
		self.mass = 1.0
		self.inertia = 1.0
		self.restitution = 0.0
		self.staticFriction = 0.0
		self.dynamicFriction = 0.0

	def toJson(self):
		dictionary = {}
		dictionary["isStatic"] = self.isStatic
		dictionary["isAffectedByConstants"] = self.isAffectedByConstants
		dictionary["lockRotation"] = self.lockRotation
		dictionary["mass"] = self.mass
		dictionary["inertia"] = self.inertia
		dictionary["restitution"] = self.restitution
		dictionary["staticFriction"] = self.staticFriction
		dictionary["dynamicFriction"] = self.dynamicFriction
		return dictionary

	def fromJson(self, jsonData):
		if "isStatic" in jsonData:
			self.isStatic = jsonData["isStatic"]
		if "isAffectedByConstants" in jsonData:
			self.isAffectedByConstants = jsonData["isAffectedByConstants"]
		if "lockRotation" in jsonData:
			self.lockRotation = jsonData["lockRotation"]
		if "mass" in jsonData:
			self.mass = jsonData["mass"]
		if "inertia" in jsonData:
			self.inertia = jsonData["inertia"]
		if "restitution" in jsonData:
			self.restitution = jsonData["restitution"]
		if "staticFriction" in jsonData:
			self.staticFriction = jsonData["staticFriction"]
		if "dynamicFriction" in jsonData:
			self.dynamicFriction = jsonData["dynamicFriction"]

class Collidable():
	def __init__(self):
		self.type = "Box"
		self.center = np.array([0.0, 0.0, 0.0], dtype=np.float32)
		self.radius = 0.5
		self.halfExtent = np.array([0.5, 0.5, 0.5], dtype=np.float32)
		self.rotation = np.array([0.0, 0.0, 0.0], dtype=np.float32)
		self.base = np.array([0.0, 0.0, 0.0], dtype=np.float32)
		self.tip = np.array([0.0, 0.5, 0.0], dtype=np.float32)

		self.fromRenderable = False

	def toJson(self):
		dictionary = {}
		dictionary["type"] = self.type
		if not self.fromRenderable:
			dictionary["center"] = [float(self.center[0]), float(self.center[1]), float(self.center[2])]
			dictionary["radius"] = self.radius
			dictionary["halfExtent"] = [float(self.halfExtent[0]), float(self.halfExtent[1]), float(self.halfExtent[2])]
			dictionary["rotation"] = [float(self.rotation[0]), float(self.rotation[1]), float(self.rotation[2])]
			dictionary["base"] = [float(self.base[0]), float(self.base[1]), float(self.base[2])]
			dictionary["tip"] = [float(self.tip[0]), float(self.tip[1]), float(self.tip[2])]
		return dictionary

	def fromJson(self, jsonData):
		if "type" in jsonData:
			self.type = jsonData["type"]
		if "center" in jsonData:
			self.center = np.array(jsonData["center"], dtype=np.float32)
		if "radius" in jsonData:
			self.radius = jsonData["radius"]
		if "halfExtent" in jsonData:
			self.halfExtent = jsonData["halfExtent"]
		if "rotation" in jsonData:
			self.rotation = jsonData["rotation"]
		if "base" in jsonData:
			self.base = jsonData["base"]
		if "tip" in jsonData:
			self.tip = jsonData["tip"]

		if self.type == "Box":
			if ("center" not in jsonData) and ("halfExtent" not in jsonData) and ("rotation" not in jsonData):
				self.fromRenderable = True
		elif self.type == "Sphere":
			if ("center" not in jsonData) and ("radius" not in jsonData):
				self.fromRenderable = True
		elif self.type == "Capsule":
			if ("radius" not in jsonData) and ("base" not in jsonData) and ("tip" not in jsonData):
				self.fromRenderable = True

class Scriptable():
	def __init__(self):
		self.scriptPath = ""
		self.scriptName = ""

	def toJson(self):
		dictionary = {}
		dictionary["scriptName"] = self.scriptName
		return dictionary

	def fromJson(self, jsonData):
		if "scriptName" in jsonData:
			self.scriptName = jsonData["scriptName"]

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

		self.isVisible = True

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
		if "camera" in jsonData:
			self.components["camera"] = Camera()
			self.components["camera"].fromJson(jsonData["camera"])
		if "light" in jsonData:
			self.components["light"] = Light()
			self.components["light"].fromJson(jsonData["light"])
		if "renderable" in jsonData:
			self.components["renderable"] = Renderable()
			self.components["renderable"].fromJson(jsonData["renderable"])
			self.components["renderable"].modelPath = os.path.normpath(self.components["renderable"].modelPath).replace("\\", "/")
			if self.components["renderable"].modelPath != "":
				if not os.path.isabs(self.components["renderable"].modelPath):
					self.components["renderable"].modelPath = os.path.normpath(globalInfo.projectDirectory + "/" + self.components["renderable"].modelPath).replace("\\", "/")
				if os.path.exists(self.components["renderable"].modelPath):
					globalInfo.rendererResourceManager.loadModel(self.components["renderable"].modelPath)
				else:
					print("Model file \"" + self.components["renderable"].modelPath + "\" does not exist.")
		if "rigidbody" in jsonData:
			self.components["rigidbody"] = Rigidbody()
			self.components["rigidbody"].fromJson(jsonData["rigidbody"])
		if "collidable" in jsonData:
			self.components["collidable"] = Collidable()
			self.components["collidable"].fromJson(jsonData["collidable"])
		if "scriptable" in jsonData:
			self.components["scriptable"] = Scriptable()
			self.components["scriptable"].fromJson(jsonData["scriptable"])

class RendererResourceManager():
	def __init__(self):
		self.models = {}
		self.textures = {}

	def loadModel(self, modelPath):
		extension = modelPath.rsplit(".")[-1]
		rendererModel = None
		if (extension == "ntmd"):
			rendererModel = self.loadNtmd(modelPath)
		elif (extension == "gltf") or (extension == "glb"):
			rendererModel = self.loadGltf(modelPath)
		else:
			print("Model file extension \"." + extension + "\" is not supported.")
		if rendererModel is not None:
			globalInfo.rendererResourceManager.models[modelPath] = rendererModel

	def loadImage(self, imagePath):
		imageWidth = 0
		imageHeight = 0
		image = None
		extension = imagePath.rsplit(".")[-1]
		if (extension == "ntim"):
			imageWidth, imageHeight, image = self.loadNtim(imagePath)
		else:
			try:
				imageData = Image.open(imagePath, "r")
				imageData = imageData.convert("RGBA")
				imageWidth = imageData.width
				imageHeight = imageData.height
				image = np.array(imageData.getdata(), dtype=np.uint8).flatten()
			except:
				print("An error occured when loading image file \"" + imagePath + "\".")
		return imageWidth, imageHeight, image

	def loadNtmd(self, modelPath):
		rendererModel = RendererModel()
		modelData = None
		with open(modelPath, "r") as modelFile:
			try:
				modelData = json.load(modelFile)
			except:
				print("\"" + modelPath + "\" is not a valid JSON file.")
				return None
		if "primitives" in modelData:
			for primitive in modelData["primitives"]:
				rendererMesh = None
				if "meshPath" in primitive:
					rendererMesh = self.loadNtmh(primitive["meshPath"])

				if rendererMesh is not None:
					if "materialPath" in primitive:
						materialPath = primitive["materialPath"]
						materialData = None
						with open(globalInfo.projectDirectory + "/" + materialPath, "r") as materialFile:
							try:
								materialData = json.load(materialFile)
							except:
								print("\"" + materialPath + "\" is not a valid JSON file.")
								return None
						if "diffuseTexture" in materialData:
							diffuseTexture = materialData["diffuseTexture"]
							if "imagePath" in diffuseTexture:
								rendererMesh.texturePath = diffuseTexture["imagePath"]
								if not os.path.isabs(rendererMesh.texturePath):
									rendererMesh.texturePath = os.path.normpath(globalInfo.projectDirectory + "/" + rendererMesh.texturePath).replace("\\", "/")
								textureWidth, textureHeight, texture = self.loadImage(rendererMesh.texturePath)

								globalInfo.rendererResourceManager.textures[rendererMesh.texturePath] = gl.glGenTextures(1)
								gl.glBindTexture(gl.GL_TEXTURE_2D, globalInfo.rendererResourceManager.textures[rendererMesh.texturePath])
								gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA8, textureWidth, textureHeight, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, texture)

								gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR)
								gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR)
								gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_EDGE)
								gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_EDGE)
							if "imageSamplerPath" in diffuseTexture:
								samplerPath = diffuseTexture["imageSamplerPath"]
								samplerData = None
								with open(globalInfo.projectDirectory + "/" + samplerPath, "r") as samplerFile:
									try:
										samplerData = json.load(samplerFile)
									except:
										print("\"" + samplerPath + "\" is not a valid JSON file.")
								if samplerData is not None:
									if "minFilter" in samplerData:
										minFilter = samplerData["minFilter"]
										if minFilter == "Nearest":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
									if "magFilter" in samplerData:
										magFilter = samplerData["magFilter"]
										if magFilter == "Nearest":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
									if "addressModeU" in samplerData:
										addressModeU = samplerData["addressModeU"]
										if addressModeU == "Repeat":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_REPEAT)
										elif addressModeU == "MirrorRepeat":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_MIRRORED_REPEAT)
									if "addressModeV" in samplerData:
										addressModeV = samplerData["addressModeV"]
										if addressModeV == "Repeat":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_REPEAT)
										elif addressModeV == "MirrorRepeat":
											gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_MIRRORED_REPEAT)

				rendererModel.meshes.append(rendererMesh)
		return rendererModel

	def loadNtmh(self, meshPath):
		rendererMesh = RendererMesh()
		meshData = None
		with open(globalInfo.projectDirectory + "/" + meshPath, "r") as meshFile:
			try:
				meshData = json.load(meshFile)
			except:
				print("\"" + meshPath + "\" is not a valid JSON file.")
				return None

		vertices = None
		if "vertices" in meshData:
			vertices = np.zeros((len(meshData["vertices"]), 8), dtype=np.float32)
			for vertexIndex, vertex in enumerate(meshData["vertices"]):
				if "position" in vertex:
					position = vertex["position"]
					vertices[vertexIndex][0] = position[0]
					vertices[vertexIndex][1] = position[1]
					vertices[vertexIndex][2] = position[2]

				if "normal" in vertex:
					normal = vertex["normal"]
					vertices[vertexIndex][3] = normal[0]
					vertices[vertexIndex][4] = normal[1]
					vertices[vertexIndex][5] = normal[2]

				if "uv" in vertex:
					uv = vertex["uv"]
					vertices[vertexIndex][6] = uv[0]
					vertices[vertexIndex][7] = uv[1]

		indices = np.zeros((0), dtype=np.uint32)
		if "indices" in meshData:
			indices = np.array(meshData["indices"], dtype=np.uint32)
		else:
			indices = np.arange(vertices.size, dtype=np.uint32)

		rendererMesh.vertexBuffer = gl.glGenBuffers(1)
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, rendererMesh.vertexBuffer)
		gl.glBufferData(gl.GL_ARRAY_BUFFER, vertices.nbytes, vertices, gl.GL_STATIC_DRAW)

		rendererMesh.indexBuffer = gl.glGenBuffers(1)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, rendererMesh.indexBuffer)
		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, indices.nbytes, indices, gl.GL_STATIC_DRAW)

		rendererMesh.indexCount = indices.size
		return rendererMesh

	def loadNtim(self, imagePath):
		imageWidth = 0
		imageHeight = 0
		image = None
		imageData = None
		with open(imagePath, "r") as imageFile:
			try:
				imageData = json.load(imageFile)
			except:
				print("\"" + imagePath + "\" is not a valid JSON file.")
		if imageData is not None:
			if "width" in imageData:
				imageWidth = imageData["width"]
			if "height" in imageData:
				imageHeight = imageData["height"]
			if "data" in imageData:
				image = np.array(imageData["data"], dtype=np.uint8)
		return imageWidth, imageHeight, image

	def loadGltf(self, modelPath):
		rendererModel = RendererModel()
		gltfData = GLTF2().load(modelPath)
		scene = gltfData.scenes[gltfData.scene]
		for nodeIndex in scene.nodes:
			modelMatrix = np.array([1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0], dtype=np.float32)
			self.loadGltfNode(modelPath, rendererModel, np.copy(modelMatrix), gltfData, copy.deepcopy(gltfData.nodes[nodeIndex]))
		return rendererModel

	def loadGltfNode(self, modelPath, rendererModel, modelMatrix, gltfData, gltfNode):
		if gltfNode.matrix is not None:
			modelMatrix = MathHelper.mat4x4Mult(modelMatrix, np.array(gltfNode.matrix, dtype=np.float32))
		else:
			if gltfNode.translation is not None:
				modelMatrix = MathHelper.mat4x4Mult(modelMatrix, MathHelper.translate(np.array(gltfNode.translation, dtype=np.float32)))
			if gltfNode.rotation is not None:
				modelMatrix = MathHelper.mat4x4Mult(modelMatrix, MathHelper.quatToMat4x4(np.array([gltfNode.rotation[3], gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2]], dtype=np.float32)))
			if gltfNode.scale is not None:
				modelMatrix = MathHelper.mat4x4Mult(modelMatrix, MathHelper.scale(np.array(gltfNode.scale, dtype=np.float32)))

		if gltfNode.mesh is not None:
			rendererMesh = RendererMesh()
			mesh = gltfData.meshes[gltfNode.mesh]
			for primitive in mesh.primitives:
				vertexCount = 0

				positions = None
				if primitive.attributes.POSITION is not None:
					accessor = gltfData.accessors[primitive.attributes.POSITION]
					bufferView = gltfData.bufferViews[accessor.bufferView]
					buffer = gltfData.buffers[bufferView.buffer]
					attributeData = gltfData.get_data_from_buffer_uri(buffer.uri)

					vertexCount = accessor.count

					offset = accessor.byteOffset + bufferView.byteOffset
					positionData = attributeData[offset:offset+(12 * accessor.count)]
					positions = struct.unpack("<" + ("f" * (3 * accessor.count)), positionData)

				normals = None
				if primitive.attributes.NORMAL is not None:
					accessor = gltfData.accessors[primitive.attributes.NORMAL]
					bufferView = gltfData.bufferViews[accessor.bufferView]
					buffer = gltfData.buffers[bufferView.buffer]
					attributeData = gltfData.get_data_from_buffer_uri(buffer.uri)

					offset = accessor.byteOffset + bufferView.byteOffset
					normalData = attributeData[offset:offset+(12 * accessor.count)]
					normals = struct.unpack("<" + ("f" * (3 * accessor.count)), normalData)

				uvs = None
				if primitive.attributes.TEXCOORD_0 is not None:
					accessor = gltfData.accessors[primitive.attributes.TEXCOORD_0]
					bufferView = gltfData.bufferViews[accessor.bufferView]
					buffer = gltfData.buffers[bufferView.buffer]
					attributeData = gltfData.get_data_from_buffer_uri(buffer.uri)

					offset = accessor.byteOffset + bufferView.byteOffset
					uvData = attributeData[offset:offset+(8 * accessor.count)]
					uvs = struct.unpack("<" + ("f" * (2 * accessor.count)), uvData)

				vertices = np.zeros((vertexCount, 8), dtype=np.float32)
				for vertexIndex in range(vertexCount):
					position = MathHelper.mat4x4Vec4Mult(modelMatrix, [positions[0 + (vertexIndex * 3)], positions[1 + (vertexIndex * 3)], positions[2 + (vertexIndex * 3)], 1.0])
					vertices[vertexIndex][0] = position[0]
					vertices[vertexIndex][1] = position[1]
					vertices[vertexIndex][2] = position[2]

					if normals is not None:
						normal = MathHelper.mat4x4Vec4Mult(MathHelper.transpose(np.linalg.inv(modelMatrix.reshape((4, 4))).reshape((16))), [normals[0 + (vertexIndex * 3)], normals[1 + (vertexIndex * 3)], normals[2 + (vertexIndex * 3)], 0.0])
						normal = MathHelper.normalize([normal[0], normal[1], normal[2]])
						vertices[vertexIndex][3] = normal[0]
						vertices[vertexIndex][4] = normal[1]
						vertices[vertexIndex][5] = normal[2]

					if uvs is not None:
						vertices[vertexIndex][6] = uvs[0 + (vertexIndex * 2)]
						vertices[vertexIndex][7] = uvs[1 + (vertexIndex * 2)]

				indices = np.zeros((0), dtype=np.uint32)
				if primitive.indices is not None:
					accessor = gltfData.accessors[primitive.indices]
					bufferView = gltfData.bufferViews[accessor.bufferView]
					buffer = gltfData.buffers[bufferView.buffer]
					attributeData = gltfData.get_data_from_buffer_uri(buffer.uri)

					for indexIndex in range(accessor.count):
						if accessor.componentType == 5120: # Byte
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 1)
							indexData = attributeData[offset:offset+1]
							indices = np.append(indices, np.array(struct.unpack("<b", indexData), np.uint32))
						elif accessor.componentType == 5121: # Unsigned Byte
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 1)
							indexData = attributeData[offset:offset+1]
							indices = np.append(indices, np.array(struct.unpack("<B", indexData), np.uint32))
						elif accessor.componentType == 5122: # Short
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 2)
							indexData = attributeData[offset:offset+2]
							indices = np.append(indices, np.array(struct.unpack("<h", indexData), np.uint32))
						elif accessor.componentType == 5123: # Unsigned Short
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 2)
							indexData = attributeData[offset:offset+2]
							indices = np.append(indices, np.array(struct.unpack("<H", indexData), np.uint32))
						elif accessor.componentType == 5125: # Unsigned Int
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 4)
							indexData = attributeData[offset:offset+4]
							indices = np.append(indices, np.array(struct.unpack("<I", indexData), np.uint32))
						elif accessor.componentType == 5126: # Float
							offset = accessor.byteOffset + bufferView.byteOffset + (indexIndex * 4)
							indexData = attributeData[offset:offset+4]
							indices = np.append(indices, np.array(struct.unpack("<f", indexData), np.uint32))
				else:
					indices = np.arange(vertexCount, dtype=np.uint32)

				textureWidth = 0
				textureHeight = 0
				texture = None
				minFilter = gl.GL_LINEAR
				magFilter = gl.GL_LINEAR
				wrapS = gl.GL_CLAMP_TO_EDGE
				wrapT = gl.GL_CLAMP_TO_EDGE
				if primitive.material is not None:
					material = gltfData.materials[primitive.material]

					if material.pbrMetallicRoughness is not None:
						if material.pbrMetallicRoughness.baseColorTexture is not None:
							baseColorTexture = gltfData.textures[material.pbrMetallicRoughness.baseColorTexture.index]
							texturePath = gltfData.images[baseColorTexture.source].uri
							texturePath = os.path.normpath(modelPath.rsplit("/", 1)[0] + "/" + texturePath).replace("\\", "/")
							textureWidth, textureHeight, texture = self.loadImage(texturePath)
							rendererMesh.texturePath = texturePath

							if baseColorTexture.sampler is not None:
								sampler = gltfData.samplers[baseColorTexture.sampler]
								if sampler.minFilter is not None:
									if (sampler.minFilter == 9728) or (sampler.minFilter == 9984) or (sampler.minFilter == 9986): # Nearest
										minFilter = gl.GL_NEAREST
								if sampler.magFilter is not None:
									if (sampler.magFilter == 9728) or (sampler.magFilter == 9984) or (sampler.magFilter == 9986): # Nearest
										magFilter = gl.GL_NEAREST
								if sampler.wrapS is not None:
									if sampler.wrapS == 10497: # Repeat
										wrapS = gl.GL_REPEAT
									elif sampler.wrapS == 10497: # MirroredRepeat
										wrapS = gl.GL_MIRRORED_REPEAT
								if sampler.wrapT is not None:
									if sampler.wrapT == 10497: # Repeat
										wrapT = gl.GL_REPEAT
									elif sampler.wrapT == 10497: # MirroredRepeat
										wrapT = gl.GL_MIRRORED_REPEAT
						elif material.pbrMetallicRoughness.baseColorFactor is not None:
							texture = np.array([material.pbrMetallicRoughness.baseColorFactor[0] * 255, material.pbrMetallicRoughness.baseColorFactor[1] * 255, material.pbrMetallicRoughness.baseColorFactor[2] * 255, material.pbrMetallicRoughness.baseColorFactor[3] * 255], dtype=np.uint8)
							textureWidth = 1
							textureHeight = 1
							rendererMesh.texturePath = np.array2string(texture)

				rendererMesh.vertexBuffer = gl.glGenBuffers(1)
				gl.glBindBuffer(gl.GL_ARRAY_BUFFER, rendererMesh.vertexBuffer)
				gl.glBufferData(gl.GL_ARRAY_BUFFER, vertices.nbytes, vertices, gl.GL_STATIC_DRAW)

				rendererMesh.indexBuffer = gl.glGenBuffers(1)
				gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, rendererMesh.indexBuffer)
				gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, indices.nbytes, indices, gl.GL_STATIC_DRAW)

				rendererMesh.indexCount = indices.size

				globalInfo.rendererResourceManager.textures[rendererMesh.texturePath] = gl.glGenTextures(1)
				gl.glBindTexture(gl.GL_TEXTURE_2D, globalInfo.rendererResourceManager.textures[rendererMesh.texturePath])
				gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA8, textureWidth, textureHeight, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, texture)
				gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, minFilter)
				gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, magFilter)
				gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, wrapS)
				gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, wrapT)

				rendererModel.meshes.append(copy.deepcopy(rendererMesh))

		for childIndex in gltfNode.children:
			self.loadGltfNode(modelPath, rendererModel, np.copy(modelMatrix), gltfData, copy.deepcopy(gltfData.nodes[childIndex]))

class GlobalInfo():
	def __init__(self):
		self.entities = []
		self.currentEntityID = -1
		self.devicePixelRatio = 1.0
		self.mainWindow = None
		self.undoStack = QUndoStack()
		self.signalEmitter = None
		self.entityID = 0
		self.currentScenePath = ""
		self.projectDirectory = "."
		self.copiedEntity = None
		self.rendererResourceManager = RendererResourceManager()

	def findEntityById(self, entityID):
		for i in range(len(self.entities)):
			if self.entities[i].entityID == entityID:
				return i
		return -1

	def findEntityByName(self, entityName):
		for i in range(len(self.entities)):
			if self.entities[i].name == entityName:
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
		globalInfo.mainWindow.setWindowTitle("NutshellEngine")
		globalInfo.undoStack.push(ClearSceneCommand())
		globalInfo.signalEmitter.resetCameraSignal.emit()

	@staticmethod
	def openScene(filePath):
		with open(filePath, "r") as f:
			try:
				sceneData = json.load(f)
			except:
				print("\"" + filePath + "\" is not a valid JSON file.")
				return
		SceneManager.newScene()
		globalInfo.currentScenePath = filePath
		globalInfo.mainWindow.setWindowTitle("NutshellEngine - " + filePath)
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
		globalInfo.mainWindow.setWindowTitle("NutshellEngine - " + filePath)
		with open(filePath, "w+", encoding="utf-8") as f:
			json.dump(globalInfo.entitiesToJson(), f, ensure_ascii=False, indent=4)

class SignalEmitter(QObject):
	createEntitySignal = pyqtSignal(int)
	destroyEntitySignal = pyqtSignal(int)
	selectEntitySignal = pyqtSignal(int)
	changeNameEntitySignal = pyqtSignal(int, str)
	changePersistenceEntitySignal = pyqtSignal(int, bool)
	changeEntityTransformSignal = pyqtSignal(int, Transform)
	addEntityCameraSignal = pyqtSignal(int)
	removeEntityCameraSignal = pyqtSignal(int)
	changeEntityCameraSignal = pyqtSignal(int, Camera)
	addEntityLightSignal = pyqtSignal(int)
	removeEntityLightSignal = pyqtSignal(int)
	changeEntityLightSignal = pyqtSignal(int, Light)
	addEntityRenderableSignal = pyqtSignal(int)
	removeEntityRenderableSignal = pyqtSignal(int)
	changeEntityRenderableSignal = pyqtSignal(int, Renderable)
	addEntityRigidbodySignal = pyqtSignal(int)
	removeEntityRigidbodySignal = pyqtSignal(int)
	changeEntityRigidbodySignal = pyqtSignal(int, Rigidbody)
	addEntityCollidableSignal = pyqtSignal(int)
	removeEntityCollidableSignal = pyqtSignal(int)
	changeEntityCollidableSignal = pyqtSignal(int, Collidable)
	addEntityScriptableSignal = pyqtSignal(int)
	removeEntityScriptableSignal = pyqtSignal(int)
	changeEntityScriptableSignal = pyqtSignal(int, Scriptable)

	toggleBackfaceCullingSignal = pyqtSignal(bool)
	toggleCurrentEntityVisibilitySignal = pyqtSignal(bool)
	toggleCamerasVisibilitySignal = pyqtSignal(bool)
	switchCameraProjectionSignal = pyqtSignal(bool)
	resetCameraSignal = pyqtSignal()
	orthographicCameraToAxisSignal = pyqtSignal(list)

class NewSceneMessageBox(QMessageBox):
	def __init__(self):
		super().__init__()
		self.setWindowTitle("New Scene...")
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
		self.newSceneAction = self.addAction("New Scene", self.newScene)
		self.newSceneAction.setShortcut("Ctrl+N")
		self.openSceneAction = self.addAction("Open Scene...", self.openScene)
		self.openSceneAction.setShortcut("Ctrl+O")
		self.saveSceneAction = self.addAction("Save Scene", self.saveScene)
		self.saveSceneAction.setShortcut("Ctrl+S")
		self.saveSceneAsAction = self.addAction("Save Scene as...", self.saveSceneAs)
		self.saveSceneAsAction.setShortcut("Shift+Ctrl+S")

	def newScene(self):
		NewSceneMessageBox()

	def openScene(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle("Open...")
		fileDialog.setNameFilter("NutshellEngine Scene (*.ntsn)")
		if globalInfo.projectDirectory != "":
			fileDialog.setDirectory(globalInfo.projectDirectory)
		file = None
		if fileDialog.exec():
			file = fileDialog.selectedFiles()[0]
			SceneManager.openScene(file)
		return file

	def saveScene(self):
		if globalInfo.currentScenePath == "":
			self.saveSceneAs()
		else:
			SceneManager.saveScene(globalInfo.currentScenePath)

	def saveSceneAs(self):
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
		self.addAction(self.undoAction)
		self.redoAction = globalInfo.undoStack.createRedoAction(self, "&Redo")
		self.redoAction.setShortcut("Ctrl+Y")
		self.addAction(self.redoAction)
		self.copyEntityAction = self.addAction("Copy Entity", self.copyEntity)
		self.copyEntityAction.setShortcut("Ctrl+C")
		self.copyEntityAction.setEnabled(False)
		self.pasteEntityAction = self.addAction("Paste Entity", self.pasteEntity)
		self.pasteEntityAction.setShortcut("Ctrl+V")
		self.pasteEntityAction.setEnabled(False)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)

	def copyEntity(self):
		if globalInfo.currentEntityID != -1:
			globalInfo.copiedEntity = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)])
			self.pasteEntityAction.setEnabled(True)

	def pasteEntity(self):
		if globalInfo.copiedEntity != None:
			globalInfo.undoStack.push(CopyEntityCommand(globalInfo.copiedEntity))

	def onSelectEntity(self, entityID):
		if entityID != -1:
			self.copyEntityAction.setEnabled(True)
		else:
			self.copyEntityAction.setEnabled(False)

class ViewMenu(QMenu):
	def __init__(self):
		self.backfaceCullingEnabled = False
		self.showCameras = False
		self.cameraProjectionOrthographic = False
		super().__init__("&View")
		self.toggleCurrentEntityVisibilityAction = self.addAction("Toggle Current Entity Visibility", self.toggleCurrentEntityVisibility)
		self.toggleCurrentEntityVisibilityAction.setShortcut("V")
		self.toggleCurrentEntityVisibilityAction.setEnabled(False)
		self.toggleBackfaceCullingAction = self.addAction("Enable Backface Culling", self.toggleBackfaceCulling)
		self.toggleBackfaceCullingAction.setShortcut("F")
		self.toggleCamerasVisibilityAction = self.addAction("Show Cameras", self.toggleCameraVisibility)
		self.toggleCamerasVisibilityAction.setShortcut("C")
		self.addSeparator()
		self.switchCameraProjectionAction = self.addAction("Switch Camera Projection to Orthographic", self.switchCameraProjection)
		self.switchCameraProjectionAction.setShortcut("P")
		self.resetCameraAction = self.addAction("Reset Camera", self.resetCamera)
		self.resetCameraAction.setShortcut("0")
		self.orthographicCameraToXMAction = self.addAction("Orthographic Camera X-", self.orthographicCameraToXM)
		self.orthographicCameraToXMAction.setShortcut("4")
		self.orthographicCameraToXPAction = self.addAction("Orthographic Camera X+", self.orthographicCameraToXP)
		self.orthographicCameraToXPAction.setShortcut("6")
		self.orthographicCameraToYMAction = self.addAction("Orthographic Camera Y-", self.orthographicCameraToYM)
		self.orthographicCameraToYMAction.setShortcut("1")
		self.orthographicCameraToYPAction = self.addAction("Orthographic Camera Y+", self.orthographicCameraToYP)
		self.orthographicCameraToYPAction.setShortcut("7")
		self.orthographicCameraToZMAction = self.addAction("Orthographic Camera Z-", self.orthographicCameraToZM)
		self.orthographicCameraToZMAction.setShortcut("8")
		self.orthographicCameraToZPAction = self.addAction("Orthographic Camera Z+", self.orthographicCameraToZP)
		self.orthographicCameraToZPAction.setShortcut("2")
		config = configparser.ConfigParser()
		if config.read("assets/options.ini") != []:
			if "Renderer" in config:
				if "toggleCurrentEntityVisibility" in config["Renderer"]:
					self.toggleCurrentEntityVisibilityAction.setShortcut(config["Renderer"]["toggleCurrentEntityVisibility"])
				if "toggleBackfaceCulling" in config["Renderer"]:
					self.toggleBackfaceCullingAction.setShortcut(config["Renderer"]["toggleBackfaceCulling"])
				if "showHideCamerasKey" in config["Renderer"]:
					self.toggleCamerasVisibilityAction.setShortcut(config["Renderer"]["toggleCamerasVisibility"])
				if "switchCameraProjection" in config["Renderer"]:
					self.switchCameraProjectionAction.setShortcut(config["Renderer"]["switchCameraProjection"])
				if "resetCamera" in config["Renderer"]:
					self.resetCameraAction.setShortcut(config["Renderer"]["resetCamera"])
				if "orthographicCameraToXM" in config["Renderer"]:
					self.orthographicCameraToXMAction.setShortcut(config["Renderer"]["orthographicCameraToXM"])
				if "orthographicCameraToXP" in config["Renderer"]:
					self.orthographicCameraToXPAction.setShortcut(config["Renderer"]["orthographicCameraToXP"])
				if "orthographicCameraToYM" in config["Renderer"]:
					self.orthographicCameraToYMAction.setShortcut(config["Renderer"]["orthographicCameraToYM"])
				if "orthographicCameraToYP" in config["Renderer"]:
					self.orthographicCameraToYPAction.setShortcut(config["Renderer"]["orthographicCameraToYP"])
				if "orthographicCameraToZM" in config["Renderer"]:
					self.orthographicCameraToZMAction.setShortcut(config["Renderer"]["orthographicCameraToZM"])
				if "orthographicCameraToZP" in config["Renderer"]:
					self.orthographicCameraToZPAction.setShortcut(config["Renderer"]["orthographicCameraToZP"])
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.toggleBackfaceCullingSignal.connect(self.onBackfaceCullingToggled)
		globalInfo.signalEmitter.toggleCamerasVisibilitySignal.connect(self.onCamerasVisibilityToggled)
		globalInfo.signalEmitter.switchCameraProjectionSignal.connect(self.onCameraProjectionSwitched)

	def toggleCurrentEntityVisibility(self):
		globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].isVisible = not globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].isVisible
		globalInfo.signalEmitter.toggleCurrentEntityVisibilitySignal.emit(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].isVisible)

	def toggleBackfaceCulling(self):
		globalInfo.signalEmitter.toggleBackfaceCullingSignal.emit(not self.backfaceCullingEnabled)

	def toggleCameraVisibility(self):
		globalInfo.signalEmitter.toggleCamerasVisibilitySignal.emit(not self.showCameras)

	def switchCameraProjection(self):
		globalInfo.signalEmitter.switchCameraProjectionSignal.emit(not self.cameraProjectionOrthographic)

	def resetCamera(self):
		globalInfo.signalEmitter.resetCameraSignal.emit()

	def orthographicCameraToXM(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([-1.0, 0.0, 0.0])

	def orthographicCameraToXP(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([1.0, 0.0, 0.0])

	def orthographicCameraToYM(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([0.0, -1.0, 0.0])

	def orthographicCameraToYP(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([0.0, 1.0, 0.0])

	def orthographicCameraToZM(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([0.0, 0.0, -1.0])

	def orthographicCameraToZP(self):
		if not self.cameraProjectionOrthographic:
			globalInfo.signalEmitter.switchCameraProjectionSignal.emit(True)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.emit([0.0, 0.0, 1.0])

	def onSelectEntity(self, entityID):
		if entityID != -1:
			self.toggleCurrentEntityVisibilityAction.setEnabled(True)
		else:
			self.toggleCurrentEntityVisibilityAction.setEnabled(False)

	def onBackfaceCullingToggled(self, backfaceCullingEnabled):
		self.backfaceCullingEnabled = backfaceCullingEnabled
		self.toggleBackfaceCullingAction.setText("Disable Backface Culling" if self.backfaceCullingEnabled else "Enable Backface Culling")

	def onCamerasVisibilityToggled(self, showCameras):
		self.showCameras = showCameras
		self.toggleCamerasVisibilityAction.setText("Hide Cameras" if self.showCameras else "Show Cameras")

	def onCameraProjectionSwitched(self, cameraProjectionOrthographic):
		self.cameraProjectionOrthographic = cameraProjectionOrthographic
		self.switchCameraProjectionAction.setText("Switch Camera Projection to Perspective" if self.cameraProjectionOrthographic else "Switch Camera Projection to Orthographic")

class MathHelper():
	@staticmethod
	def normalize(vector):
		return vector / np.linalg.norm(vector)

	@staticmethod
	def transpose(matrix):
		return np.array([
			matrix[0], matrix[4], matrix[8], matrix[12],
			matrix[1], matrix[5], matrix[9], matrix[13],
			matrix[2], matrix[6], matrix[10], matrix[14],
			matrix[3], matrix[7], matrix[11], matrix[15]], dtype=np.float32)

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
	def orthoRH(left, right, top, bottom, near, far):
		rightPleft = right + left
		rightMleft = right - left
		topPbottom = top + bottom
		topMbottom = top - bottom
		farMNear = far - near
		return np.array([2.0 / rightMleft, 0.0, 0.0, 0.0,
			0.0, 2.0 / topMbottom, 0.0, 0.0,
			0.0, 0.0, -1.0 / farMNear, 0.0,
			-(rightPleft / rightMleft), -(topPbottom / topMbottom), -near / farMNear, 1.0], dtype=np.float32)

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

		return worldSpace[:3] / worldSpace[3]

	@staticmethod
	def quatToMat4x4(quat):
		return np.array([
			1.0 - 2.0 * ((quat[2] * quat[2]) + (quat[3] * quat[3])),
			2.0 * ((quat[1] * quat[2]) + (quat[0] * quat[3])),
			2.0 * ((quat[1] * quat[3]) - (quat[0] * quat[2])),
			0.0,

			2.0 * ((quat[1] * quat[2]) - (quat[0] * quat[3])),
			1.0 - 2.0 * ((quat[1] * quat[1]) + (quat[3] * quat[3])),
			2.0 * ((quat[2] * quat[3]) + (quat[0] * quat[1])),
			0.0,

			2.0 * ((quat[1] * quat[3]) + (quat[0] * quat[2])),
			2.0 * ((quat[2] * quat[3]) - (quat[0] * quat[1])),
			1.0 - 2.0 * ((quat[1] * quat[1]) + (quat[2] * quat[2])),
			0.0,

			0.0, 0.0, 0.0, 1.0], dtype=np.float32)

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

		out vec2 fragUV;

		void main() {
			fragUV = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
			gl_Position = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
		}
		'''

class RendererCamera():
	def __init__(self):
		self.perspectivePosition = np.array([0.0, 1.0, 1.0], dtype=np.float32)
		self.perspectiveDirection = np.array([0.0, -1.0, -1.0], dtype=np.float32)
		self.perspectiveDirection = MathHelper.normalize(self.perspectiveDirection)
		self.perspectiveUp = np.array([0.0, 1.0, 0.0], dtype=np.float32)

		self.perspectiveYaw = np.rad2deg(np.arctan2(self.perspectiveDirection[2], self.perspectiveDirection[0]))
		self.perspectivePitch = np.rad2deg(-np.arcsin(self.perspectiveDirection[1]))

		self.orthographicPosition = np.array([0.0, 0.0, 0.0], dtype=np.float32)
		self.orthographicDirection = np.array([0.0, 0.0, -1.0], dtype=np.float32)
		self.orthographicUp = np.array([0.0, 1.0, 0.0], dtype=np.float32)
		self.orthographicHalfExtent = 10.0

		self.nearPlane = 0.01
		self.farPlane = 500.0

		self.cameraSpeed = 2.0

		self.viewMatrix = None
		self.projectionMatrix = None
		self.viewProjMatrix = None
		self.invViewMatrix = None
		self.invProjMatrix = None

		self.basePerspectivePosition = np.copy(self.perspectivePosition)
		self.basePerspectiveDirection = np.copy(self.perspectiveDirection)

		self.baseOrthographicPosition = np.copy(self.orthographicPosition)
		self.baseOrthographicDirection = np.copy(self.orthographicDirection)
		self.baseOrthographicUp = np.copy(self.orthographicUp)
		self.baseOrthographicHalfExtent = self.orthographicHalfExtent

class RendererModel():
	def __init__(self):
		self.meshes = []

class RendererMesh():
	def __init__(self):
		self.vertexBuffer = 0
		self.indexBuffer = 0
		self.indexCount = 0
		self.texturePath = "defaultTexture"

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

		self.moveCameraButtonPressed = False

		self.mouseScrollY = 0.0

		self.mouseCursorPreviousPosition = np.array(2, dtype=np.float32)
		self.mouseCursorDifference = np.zeros(2, dtype=np.float32)

		self.waitTimer = QTimer()
		self.waitTimer.timeout.connect(self.update)

		self.doPicking = False

		self.entityMoveTransform = None

		self.gotResized = False

		self.backfaceCullingEnabled = False
		self.showCameras = False
		self.cameraProjectionOrthographic = False

		globalInfo.signalEmitter.toggleBackfaceCullingSignal.connect(self.onBackfaceCullingToggled)
		globalInfo.signalEmitter.toggleCamerasVisibilitySignal.connect(self.onCamerasVisibilityToggled)
		globalInfo.signalEmitter.switchCameraProjectionSignal.connect(self.onCameraProjectionSwitched)
		globalInfo.signalEmitter.resetCameraSignal.connect(self.onCameraReset)
		globalInfo.signalEmitter.orthographicCameraToAxisSignal.connect(self.onOrthographicCameraToAxisChanged)

	def initializeGL(self):
		[fullscreenVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, OpenGLHelper.fullscreenVertexShaderCode())

		# Entity Program
		entityVertexShaderCode = '''
		#version 460

		in vec3 position;
		in vec3 normal;
		in vec2 uv;

		uniform mat4 viewProj;
		uniform mat4 model;

		out vec3 fragNormal;
		out vec2 fragUV;

		void main() {
			fragNormal = (model * vec4(normal, 0.0)).xyz;
			fragUV = uv;
			gl_Position = viewProj * model * vec4(position, 1.0);
		}
		'''
		[entityVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, entityVertexShaderCode)

		entityFragmentShaderCode = '''
		#version 460

		in vec3 fragNormal;
		in vec2 fragUV;

		uniform sampler2D textureSampler;
		uniform bool doShading;

		out vec4 outColor;

		void main() {
			if (doShading) {
				outColor = vec4(texture(textureSampler, fragUV).rgb * dot(vec3(0.0, 1.0, 0.0), fragNormal), 1.0);
			}
			else {
				outColor = vec4(texture(textureSampler, fragUV).rgb, 1.0);
			}
		}
		'''
		[entityFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, entityFragmentShaderCode)

		[self.entityProgram, _] = OpenGLHelper.createProgram(entityVertexShader, entityFragmentShader)

		# Camera Frustum Program
		cameraFrustumVertexShaderCode = '''
		#version 460

		in vec3 position;

		uniform mat4 viewProj;
		uniform mat4 model;

		void main() {
			gl_Position = viewProj * model * vec4(position, 1.0);
		}
		'''
		[cameraFrustumVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, cameraFrustumVertexShaderCode)

		cameraFrustumFragmentShaderCode = '''
		#version 460

		out vec4 outColor;

		void main() {
			outColor = vec4(0.0, 1.0, 0.0, 1.0);
		}
		'''
		[cameraFrustumFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, cameraFrustumFragmentShaderCode)

		[self.cameraFrustumProgram, _] = OpenGLHelper.createProgram(cameraFrustumVertexShader, cameraFrustumFragmentShader)

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
				color.r = 1.0;
			}

			if ((p.x > -0.1 * minX) && (p.x < 0.1 * minX)) {
				color.b = 1.0;
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

		# Picking
		pickingVertexShaderCode = '''
		#version 460

		in vec3 position;

		uniform mat4 viewProj;
		uniform mat4 model;

		void main() {
			gl_Position = viewProj * model * vec4(position, 1.0);
		}
		'''
		[pickingVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, pickingVertexShaderCode)

		pickingFragmentShaderCode = '''
		#version 460

		uniform uint entityID;

		out uint outEntityID;

		void main() {
			outEntityID = entityID;
		}
		'''
		[pickingFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, pickingFragmentShaderCode)

		[self.pickingProgram, _] = OpenGLHelper.createProgram(pickingVertexShader, pickingFragmentShader)

		self.pickingFramebuffer = gl.glGenFramebuffers(1)
		self.createPickingImages()

		# Outline
		outlineSoloVertexShaderCode = '''
		#version 460

		in vec3 position;

		uniform mat4 viewProj;
		uniform mat4 model;

		void main() {
			gl_Position = viewProj * model * vec4(position, 1.0);
		}
		'''
		[outlineSoloVertexShader, _] = OpenGLHelper.compileShader(gl.GL_VERTEX_SHADER, outlineSoloVertexShaderCode)

		outlineSoloFragmentShaderCode = '''
		#version 460

		out float outColor;

		void main() {
			outColor = 1.0;
		}
		'''
		[outlineSoloFragmentShader, _] = OpenGLHelper.compileShader(gl.GL_FRAGMENT_SHADER, outlineSoloFragmentShaderCode)

		[self.outlineSoloProgram, _] = OpenGLHelper.createProgram(outlineSoloVertexShader, outlineSoloFragmentShader)

		self.outlineSoloFramebuffer = gl.glGenFramebuffers(1)
		self.createOutlineSoloImages()

		outlineFragmentShaderCode = '''
		#version 460

		uniform sampler2D outlineSoloTexture;

		in vec2 fragUV;

		out vec4 outColor;

		void main() {
			float value = texture(outlineSoloTexture, fragUV).r;
			if (value == 1.0) {
				discard;
			}

			vec2 texelSize = 1.0 / vec2(textureSize(outlineSoloTexture, 0));
			bool foundValue = false;
			for (float range = 0.0; range < 2.0; range++) {
				float n = texture(outlineSoloTexture, fragUV + vec2(0.0, texelSize.y * (range + 1.0))).r;
				float s = texture(outlineSoloTexture, fragUV + vec2(0.0, -texelSize.y * (range + 1.0))).r;
				float e = texture(outlineSoloTexture, fragUV + vec2(-texelSize.x * (range + 1.0), 0.0)).r;
				float w = texture(outlineSoloTexture, fragUV + vec2(texelSize.x * (range + 1.0), 0.0)).r;
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

		# Cube indices
		cubeTriangleIndexBuffer = gl.glGenBuffers(1)
		cubeTriangleIndices = np.array([0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23], dtype=np.uint32)
		cubeTriangleIndexCount = len(cubeTriangleIndices)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, cubeTriangleIndexBuffer)
		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, cubeTriangleIndices.nbytes, cubeTriangleIndices, gl.GL_STATIC_DRAW)

		cubeLineIndexBuffer = gl.glGenBuffers(1)
		cubeLineIndices = np.array([0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7], dtype=np.uint32)
		cubeLineIndexCount = len(cubeLineIndices)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, cubeLineIndexBuffer)
		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER, cubeLineIndices.nbytes, cubeLineIndices, gl.GL_STATIC_DRAW)

		# Default Cube
		defaultCubeVertexBuffer = gl.glGenBuffers(1)
		defaultCubeVertices = np.array([(0.05, 0.05, -0.05, 0.0, 1.0, 0.0, 1.0, 0.0), (-0.05, 0.05, -0.05, 0.0, 1.0, 0.0, 1.0, 1.0), (-0.05, 0.05, 0.05, 0.0, 1.0, 0.0, 0.0, 1.0), (0.05, 0.05, 0.05, 0.0, 1.0, 0.0, 0.0, 0.0), (0.05, -0.05, 0.05, 0.0, 0.0, 1.0, 1.0, 0.0), (0.05, 0.05, 0.05, 0.0, 0.0, 1.0, 1.0, 1.0), (-0.05, 0.05, 0.05, 0.0, 0.0, 1.0, 0.0, 1.0), (-0.05, -0.05, 0.05, 0.0, 0.0, 1.0, 0.0, 0.0), (-0.05, -0.05, 0.05, -1.0, 0.0, 0.0, 1.0, 0.0), (-0.05, 0.05, 0.05, -1.0, 0.0, 0.0, 1.0, 1.0), (-0.05, 0.05, -0.05, -1.0, 0.0, 0.0, 0.0, 1.0), (-0.05, -0.05, -0.05, -1.0, 0.0, 0.0, 0.0, 0.0), (-0.05, -0.05, -0.05, 0.0, -1.0, 0.0, 1.0, 0.0), (0.05, -0.05, -0.05, 0.0, -1.0, 0.0, 1.0, 1.0), (0.05, -0.05, 0.05, 0.0, -1.0, 0.0, 0.0, 1.0), (-0.05, -0.05, 0.05, 0.0, -1.0, 0.0, 0.0, 0.0), (0.05, -0.05, -0.05, 1.0, 0.0, 0.0, 1.0, 0.0), (0.05, 0.05, -0.05, 1.0, 0.0, 0.0, 1.0, 1.0), (0.05, 0.05, 0.05, 1.0, 0.0, 0.0, 0.0, 1.0), (0.05, -0.05, 0.05, 1.0, 0.0, 0.0, 0.0, 0.0), (-0.05, -0.05, -0.05, 0.0, 0.0, -1.0, 1.0, 0.0), (-0.05, 0.05, -0.05, 0.0, 0.0, -1.0, 1.0, 1.0), (0.05, 0.05, -0.05, 0.0, 0.0, -1.0, 0.0, 1.0), (0.05, -0.05, -0.05, 0.0, 0.0, -1.0, 0.0, 0.0)], dtype=np.float32)
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, defaultCubeVertexBuffer)
		gl.glBufferData(gl.GL_ARRAY_BUFFER, defaultCubeVertices.nbytes, defaultCubeVertices, gl.GL_STATIC_DRAW)

		defaultCubeMesh = RendererMesh()
		defaultCubeMesh.vertexBuffer = defaultCubeVertexBuffer
		defaultCubeMesh.indexBuffer = cubeTriangleIndexBuffer
		defaultCubeMesh.indexCount = cubeTriangleIndexCount
		defaultCubeMesh.texturePath = "defaultTexture"

		defaultCubeModel = RendererModel()
		defaultCubeModel.meshes.append(defaultCubeMesh)

		globalInfo.rendererResourceManager.models["defaultCube"] = defaultCubeModel

		# Default Texture
		defaultTexture = gl.glGenTextures(1)
		defaultTextureData = np.array([(145, 99, 65, 255), (208, 194, 175, 255), (208, 194, 175, 255), (145, 99, 65, 255)], dtype=np.uint8)
		gl.glBindTexture(gl.GL_TEXTURE_2D, defaultTexture)
		gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA8, 2, 2, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, defaultTextureData)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP_TO_EDGE)
		gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP_TO_EDGE)

		globalInfo.rendererResourceManager.textures["defaultTexture"] = defaultTexture

		# Frustum Cube
		cameraFrustumCubeVertexBuffer = gl.glGenBuffers(1)
		cameraFrustumCubeVertices = np.array([(-1.0, -1.0, -1.0), (1.0, -1.0, -1.0), (1.0, -1.0, 1.0), (-1.0, -1.0, 1.0), (-1.0, 1.0, -1.0), (1.0, 1.0, -1.0), (1.0, 1.0, 1.0), (-1.0, 1.0, 1.0)], dtype=np.float32)
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, cameraFrustumCubeVertexBuffer)
		gl.glBufferData(gl.GL_ARRAY_BUFFER, cameraFrustumCubeVertices.nbytes, cameraFrustumCubeVertices, gl.GL_STATIC_DRAW)

		cameraFrustumCubeMesh = RendererMesh()
		cameraFrustumCubeMesh.vertexBuffer = cameraFrustumCubeVertexBuffer
		cameraFrustumCubeMesh.indexBuffer = cubeLineIndexBuffer
		cameraFrustumCubeMesh.indexCount = cubeLineIndexCount

		cameraFrustumCubeModel = RendererModel()
		cameraFrustumCubeModel.meshes.append(cameraFrustumCubeMesh)

		globalInfo.rendererResourceManager.models["cameraFrustumCube"] = cameraFrustumCubeModel

		# Render
		self.waitTimer.start(16)

	def paintGL(self):
		if self.gotResized:
			self.createPickingImages()
			self.createOutlineSoloImages()

			self.gotResized = False

		self.updateCamera()

		if self.backfaceCullingEnabled:
			gl.glEnable(gl.GL_CULL_FACE)
		else:
			gl.glDisable(gl.GL_CULL_FACE)

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
			if entity.isVisible:
				if (entity.entityID == globalInfo.currentEntityID) and (self.entityMoveTransform is not None):
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
				else:
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.entityProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

				if ("renderable" in entity.components) and (entity.components["renderable"].modelPath in globalInfo.rendererResourceManager.models):
					entityModel = globalInfo.rendererResourceManager.models[entity.components["renderable"].modelPath]
					for entityMesh in entityModel.meshes:
						gl.glBindBuffer(gl.GL_ARRAY_BUFFER, entityMesh.vertexBuffer)
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "position"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "normal"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "normal"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(12))
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "uv"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "uv"), 2, gl.GL_FLOAT, False, 32, ctypes.c_void_p(24))
						gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer)

						gl.glActiveTexture(gl.GL_TEXTURE0)
						gl.glBindTexture(gl.GL_TEXTURE_2D, globalInfo.rendererResourceManager.textures[entityMesh.texturePath])
						gl.glUniform1i(gl.glGetUniformLocation(self.entityProgram, "textureSampler"), 0)

						gl.glUniform1i(gl.glGetUniformLocation(self.entityProgram, "doShading"), 0)

						gl.glDrawElements(gl.GL_TRIANGLES, entityMesh.indexCount, gl.GL_UNSIGNED_INT, None)
				else:
					gl.glBindBuffer(gl.GL_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].vertexBuffer)
					gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "position"))
					gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
					gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "normal"))
					gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "normal"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(12))
					gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.entityProgram, "uv"))
					gl.glVertexAttribPointer(gl.glGetAttribLocation(self.entityProgram, "uv"), 2, gl.GL_FLOAT, False, 32, ctypes.c_void_p(24))
					gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexBuffer)

					gl.glActiveTexture(gl.GL_TEXTURE0)
					gl.glBindTexture(gl.GL_TEXTURE_2D, globalInfo.rendererResourceManager.textures["defaultTexture"])
					gl.glUniform1i(gl.glGetUniformLocation(self.entityProgram, "textureSampler"), 0)

					gl.glUniform1i(gl.glGetUniformLocation(self.entityProgram, "doShading"), 0)

					gl.glDrawElements(gl.GL_TRIANGLES, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexCount, gl.GL_UNSIGNED_INT, None)

		gl.glDisable(gl.GL_CULL_FACE)

		# Entities Cameras
		if self.showCameras:
			gl.glUseProgram(self.cameraFrustumProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.cameraFrustumProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)

			gl.glBindBuffer(gl.GL_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].vertexBuffer)
			gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.cameraFrustumProgram, "position"))
			gl.glVertexAttribPointer(gl.glGetAttribLocation(self.cameraFrustumProgram, "position"), 3, gl.GL_FLOAT, False, 12, ctypes.c_void_p(0))
			gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexBuffer)

			for entity in globalInfo.entities:
				if entity.isVisible:
					if "camera" in entity.components:
						if (entity.entityID == globalInfo.currentEntityID) and (self.entityMoveTransform is not None):
							entityCameraViewMatrix = MathHelper.lookAtRH(self.entityMoveTransform.position, np.add(self.entityMoveTransform.position, entity.components["camera"].forward), entity.components["camera"].up)
							entityCameraRotation = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[2]), [0.0, 0.0, 1.0])))
						else:
							entityCameraViewMatrix = MathHelper.lookAtRH(entity.components["transform"].position, np.add(entity.components["transform"].position, entity.components["camera"].forward), entity.components["camera"].up)
							entityCameraRotation = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[2]), [0.0, 0.0, 1.0])))
						entityCameraProjectionMatrix = MathHelper.perspectiveRH(np.deg2rad(entity.components["camera"].fov), 16.0 / 9.0, max(entity.components["camera"].nearPlane, 0.001), max(entity.components["camera"].farPlane, 0.001))
						invEntityCameraModel = np.linalg.inv(MathHelper.mat4x4Mult(entityCameraProjectionMatrix, MathHelper.mat4x4Mult(entityCameraRotation, entityCameraViewMatrix)).reshape((4, 4)))
						gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.cameraFrustumProgram, "model"), 1, False, invEntityCameraModel)

						gl.glDrawElements(gl.GL_LINES, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexCount, gl.GL_UNSIGNED_INT, None)

		# Grid
		if not self.cameraProjectionOrthographic:
			gl.glUseProgram(self.gridProgram)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "view"), 1, False, self.camera.viewMatrix)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "projection"), 1, False, self.camera.projectionMatrix)
			gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.gridProgram, "viewProj"), 1, False, self.camera.viewProjMatrix)
			gl.glUniform1f(gl.glGetUniformLocation(self.gridProgram, "near"), self.camera.nearPlane)
			gl.glUniform1f(gl.glGetUniformLocation(self.gridProgram, "far"), self.camera.farPlane)

			gl.glDrawArrays(gl.GL_TRIANGLES, 0, 6)

		if self.backfaceCullingEnabled:
			gl.glEnable(gl.GL_CULL_FACE)
		else:
			gl.glDisable(gl.GL_CULL_FACE)

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
				if entity.isVisible:
					if (entity.entityID == globalInfo.currentEntityID) and (self.entityMoveTransform is not None):
						gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
					else:
						gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.pickingProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

					gl.glUniform1ui(gl.glGetUniformLocation(self.pickingProgram, "entityID"), entity.entityID)

					if ("renderable" in entity.components) and (entity.components["renderable"].modelPath in globalInfo.rendererResourceManager.models):
						entityModel = globalInfo.rendererResourceManager.models[entity.components["renderable"].modelPath]
						for entityMesh in entityModel.meshes:
							gl.glBindBuffer(gl.GL_ARRAY_BUFFER, entityMesh.vertexBuffer)
							gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.pickingProgram, "position"))
							gl.glVertexAttribPointer(gl.glGetAttribLocation(self.pickingProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
							gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer)

							gl.glDrawElements(gl.GL_TRIANGLES, entityMesh.indexCount, gl.GL_UNSIGNED_INT, None)
					else:
						gl.glBindBuffer(gl.GL_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].vertexBuffer)
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.pickingProgram, "position"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.pickingProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
						gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexBuffer)

						gl.glDrawElements(gl.GL_TRIANGLES, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexCount, gl.GL_UNSIGNED_INT, None)

			cursorPosition = self.mapFromGlobal(QCursor.pos())
			pickedEntityID = gl.glReadPixels(cursorPosition.x() * globalInfo.devicePixelRatio, (self.height() - cursorPosition.y()) * globalInfo.devicePixelRatio, 1, 1, gl.GL_RED_INTEGER, gl.GL_UNSIGNED_INT)[0][0]
			if pickedEntityID != np.iinfo(np.uint32).max:
				globalInfo.currentEntityID = pickedEntityID
			else:
				globalInfo.currentEntityID = -1
			globalInfo.signalEmitter.selectEntitySignal.emit(globalInfo.currentEntityID)

			self.doPicking = False

		gl.glDisable(gl.GL_CULL_FACE)

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

			# Entity
			entity = globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)]
			if entity.isVisible:
				if (entity.entityID == globalInfo.currentEntityID) and (self.entityMoveTransform is not None):
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, self.entityMoveTransform.modelMatrix())
				else:
					gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, entity.components["transform"].modelMatrix())

				if ("renderable" in entity.components) and (entity.components["renderable"].modelPath in globalInfo.rendererResourceManager.models):
					entityModel = globalInfo.rendererResourceManager.models[entity.components["renderable"].modelPath]
					for entityMesh in entityModel.meshes:
						gl.glBindBuffer(gl.GL_ARRAY_BUFFER, entityMesh.vertexBuffer)
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.outlineSoloProgram, "position"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.outlineSoloProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
						gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, entityMesh.indexBuffer)

						gl.glDrawElements(gl.GL_TRIANGLES, entityMesh.indexCount, gl.GL_UNSIGNED_INT, None)
				else:
					gl.glBindBuffer(gl.GL_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].vertexBuffer)
					gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.outlineSoloProgram, "position"))
					gl.glVertexAttribPointer(gl.glGetAttribLocation(self.outlineSoloProgram, "position"), 3, gl.GL_FLOAT, False, 32, ctypes.c_void_p(0))
					gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexBuffer)

					gl.glDrawElements(gl.GL_TRIANGLES, globalInfo.rendererResourceManager.models["defaultCube"].meshes[0].indexCount, gl.GL_UNSIGNED_INT, None)

				# Entity Camera
				if self.showCameras:
					if "camera" in entity.components:
						if (entity.entityID == globalInfo.currentEntityID) and (self.entityMoveTransform is not None):
							entityCameraViewMatrix = MathHelper.lookAtRH(self.entityMoveTransform.position, np.add(self.entityMoveTransform.position, entity.components["camera"].forward), entity.components["camera"].up)
							entityCameraRotation = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(self.entityMoveTransform.rotation[2]), [0.0, 0.0, 1.0])))
						else:
							entityCameraViewMatrix = MathHelper.lookAtRH(entity.components["transform"].position, np.add(entity.components["transform"].position, entity.components["camera"].forward), entity.components["camera"].up)
							entityCameraRotation = MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[0]), [1.0, 0.0, 0.0]), MathHelper.mat4x4Mult(MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[1]), [0.0, 1.0, 0.0]), MathHelper.rotate(np.deg2rad(entity.components["transform"].rotation[2]), [0.0, 0.0, 1.0])))
						entityCameraProjectionMatrix = MathHelper.perspectiveRH(np.deg2rad(entity.components["camera"].fov), 16.0 / 9.0, max(entity.components["camera"].nearPlane, 0.001), max(entity.components["camera"].farPlane, 0.001))
						invEntityCameraModel = np.linalg.inv(MathHelper.mat4x4Mult(entityCameraProjectionMatrix, MathHelper.mat4x4Mult(entityCameraRotation, entityCameraViewMatrix)).reshape((4, 4)))
						gl.glUniformMatrix4fv(gl.glGetUniformLocation(self.outlineSoloProgram, "model"), 1, False, invEntityCameraModel)

						gl.glBindBuffer(gl.GL_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].vertexBuffer)
						gl.glEnableVertexAttribArray(gl.glGetAttribLocation(self.outlineSoloProgram, "position"))
						gl.glVertexAttribPointer(gl.glGetAttribLocation(self.outlineSoloProgram, "position"), 3, gl.GL_FLOAT, False, 12, ctypes.c_void_p(0))
						gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexBuffer)

						gl.glDrawElements(gl.GL_LINES, globalInfo.rendererResourceManager.models["cameraFrustumCube"].meshes[0].indexCount, gl.GL_UNSIGNED_INT, None)

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

		if not self.cameraProjectionOrthographic:
			self.camera.perspectiveYaw = (self.camera.perspectiveYaw + self.mouseCursorDifference[0]) % 360.0
			self.camera.perspectivePitch = max(-89.0, min(89.0, self.camera.perspectivePitch + self.mouseCursorDifference[1]))

			cameraYawRad = np.deg2rad(self.camera.perspectiveYaw)
			cameraPitchRad = np.deg2rad(self.camera.perspectivePitch)

			self.camera.perspectiveDirection = np.array([
				np.cos(cameraPitchRad) * np.cos(cameraYawRad),
				-np.sin(cameraPitchRad),
				np.cos(cameraPitchRad) * np.sin(cameraYawRad)
			])
			self.camera.perspectiveDirection = MathHelper.normalize(self.camera.perspectiveDirection)

			t = MathHelper.normalize(np.array([-self.camera.perspectiveDirection[2], 0.0, self.camera.perspectiveDirection[0]]))

			cameraSpeed = self.camera.cameraSpeed * deltaTime

			if self.cameraForwardKeyPressed:
				self.camera.perspectivePosition = np.add(self.camera.perspectivePosition, self.camera.perspectiveDirection * cameraSpeed)

			if self.cameraBackwardKeyPressed:
				self.camera.perspectivePosition = np.add(self.camera.perspectivePosition, self.camera.perspectiveDirection * -cameraSpeed)

			if self.cameraLeftKeyPressed:
				self.camera.perspectivePosition = np.add(self.camera.perspectivePosition, t * -cameraSpeed)

			if self.cameraRightKeyPressed:
				self.camera.perspectivePosition = np.add(self.camera.perspectivePosition, t * cameraSpeed)

			if self.cameraUpKeyPressed:
				self.camera.perspectivePosition[1] += cameraSpeed

			if self.cameraDownKeyPressed:
				self.camera.perspectivePosition[1] -= cameraSpeed

			self.camera.viewMatrix = MathHelper.lookAtRH(self.camera.perspectivePosition, np.add(self.camera.perspectivePosition, self.camera.perspectiveDirection), self.camera.perspectiveUp)
			self.camera.projectionMatrix = MathHelper.perspectiveRH(np.deg2rad(45.0), self.width() / self.height(), self.camera.nearPlane, self.camera.farPlane)
		else:
			if (self.camera.orthographicDirection[1] == 1.0) or (self.camera.orthographicDirection[1] == -1.0):
				t = MathHelper.normalize(np.array([-self.camera.orthographicDirection[1], 0.0, self.camera.orthographicDirection[0]]))
			else:
				t = MathHelper.normalize(np.array([-self.camera.orthographicDirection[2], 0.0, self.camera.orthographicDirection[0]]))

			horizontalSpeed = (1.0 if (self.mouseCursorDifference[0] == 0.0) else abs(self.mouseCursorDifference[0])) * self.camera.orthographicHalfExtent * self.camera.cameraSpeed * deltaTime
			verticalSpeed = (1.0 if (self.mouseCursorDifference[1] == 0.0) else abs(self.mouseCursorDifference[1])) * self.camera.orthographicHalfExtent * self.camera.cameraSpeed * deltaTime
			halfExtentSpeed = self.camera.cameraSpeed * 5.0 * (1.0 if (self.mouseScrollY == 0.0) else 2.0) * deltaTime

			if self.cameraForwardKeyPressed or (self.mouseCursorDifference[1] < 0.0):
				self.camera.orthographicPosition = np.add(self.camera.orthographicPosition, self.camera.orthographicUp * verticalSpeed)

			if self.cameraBackwardKeyPressed or (self.mouseCursorDifference[1] > 0.0):
				self.camera.orthographicPosition = np.add(self.camera.orthographicPosition, self.camera.orthographicUp * -verticalSpeed)

			if self.cameraLeftKeyPressed or (self.mouseCursorDifference[0] < 0.0):
				self.camera.orthographicPosition = np.add(self.camera.orthographicPosition, t * -horizontalSpeed)

			if self.cameraRightKeyPressed or (self.mouseCursorDifference[0] > 0.0):
				self.camera.orthographicPosition = np.add(self.camera.orthographicPosition, t * horizontalSpeed)

			if self.cameraUpKeyPressed or (self.mouseScrollY < 0.0):
				self.camera.orthographicHalfExtent += halfExtentSpeed

			if self.cameraDownKeyPressed or (self.mouseScrollY > 0.0):
				self.camera.orthographicHalfExtent -= halfExtentSpeed
				self.camera.orthographicHalfExtent = max(self.camera.orthographicHalfExtent, 0.01)

			self.camera.viewMatrix = MathHelper.lookAtRH(self.camera.orthographicPosition, np.add(self.camera.orthographicPosition, self.camera.orthographicDirection), self.camera.orthographicUp)
			orthographicHalfExtentWidth = self.camera.orthographicHalfExtent * (self.width() / self.height())
			self.camera.projectionMatrix = MathHelper.orthoRH(-orthographicHalfExtentWidth, orthographicHalfExtentWidth, self.camera.orthographicHalfExtent, -self.camera.orthographicHalfExtent, self.camera.nearPlane, self.camera.farPlane)

		self.camera.viewProjMatrix = MathHelper.mat4x4Mult(self.camera.projectionMatrix, self.camera.viewMatrix)
		self.camera.invViewMatrix = np.linalg.inv(np.copy(self.camera.viewMatrix).reshape((4, 4)))
		self.camera.invProjMatrix = np.linalg.inv(np.copy(self.camera.projectionMatrix).reshape((4, 4)))

		self.mouseCursorDifference = np.zeros(2, dtype=np.float32)

		self.mouseScrollY = 0.0

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
		elif e.key() == self.cameraBackwardKey:
			self.cameraBackwardKeyPressed = True
		elif e.key() == self.cameraLeftKey:
			self.cameraLeftKeyPressed = True
		elif e.key() == self.cameraRightKey:
			self.cameraRightKeyPressed = True
		elif e.key() == self.cameraUpKey:
			self.cameraUpKeyPressed = True
		elif e.key() == self.cameraDownKey:
			self.cameraDownKeyPressed = True
		elif e.key() == self.translateEntityKey:
			if (globalInfo.currentEntityID != -1) and (not self.moveCameraButtonPressed) and (not self.anyEntityTransformKeyPressed()):
				self.translateEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()], dtype=np.float32)
		elif e.key() == self.rotateEntityKey:
			if (globalInfo.currentEntityID != -1) and (not self.moveCameraButtonPressed) and (not self.anyEntityTransformKeyPressed()):
				self.rotateEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()], dtype=np.float32)
		elif e.key() == self.scaleEntityKey:
			if (globalInfo.currentEntityID != -1) and (not self.moveCameraButtonPressed) and (not self.anyEntityTransformKeyPressed()):
				self.scaleEntityKeyPressed = True
				self.entityMoveTransform = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["transform"])
				cursorPos = self.mapFromGlobal(QCursor.pos())
				self.mouseCursorPreviousPosition = np.array([cursorPos.x(), self.height() - cursorPos.y()], dtype=np.float32)
		elif e.key() == Qt.Key.Key_Delete:
			if globalInfo.currentEntityID != -1:
				globalInfo.undoStack.push(DestroyEntityCommand(globalInfo.currentEntityID))
		e.accept()

	def keyReleaseEvent(self, e):
		if e.isAutoRepeat():
			e.accept()
			return
		if e.key() == self.cameraForwardKey:
			self.cameraForwardKeyPressed = False
		elif e.key() == self.cameraBackwardKey:
			self.cameraBackwardKeyPressed = False
		elif e.key() == self.cameraLeftKey:
			self.cameraLeftKeyPressed = False
		elif e.key() == self.cameraRightKey:
			self.cameraRightKeyPressed = False
		elif e.key() == self.cameraUpKey:
			self.cameraUpKeyPressed = False
		elif e.key() == self.cameraDownKey:
			self.cameraDownKeyPressed = False
		elif e.key() == self.translateEntityKey:
			if self.translateEntityKeyPressed:
				self.translateEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		elif e.key() == self.rotateEntityKey:
			if self.rotateEntityKeyPressed:
				self.rotateEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		elif e.key() == self.scaleEntityKey:
			if self.scaleEntityKeyPressed:
				self.scaleEntityKeyPressed = False
				self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
				if globalInfo.currentEntityID != -1:
					globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, self.entityMoveTransform))
					self.entityMoveTransform = None
		e.accept()

	def mousePressEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.button() == Qt.MouseButton.RightButton:
				self.moveCameraButtonPressed = True
				self.savedMousePosition = QCursor.pos()
				self.setCursor(Qt.CursorShape.BlankCursor)
				widgetCenter = QPoint(int(self.width() / 2), int(self.height() / 2))
				self.mouseCursorPreviousPosition = np.array([widgetCenter.x(), widgetCenter.y()])
				QCursor.setPos(self.mapToGlobal(widgetCenter))
			elif e.button() == Qt.MouseButton.LeftButton:
				self.doPicking = True
		e.accept()

	def mouseReleaseEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.button() == Qt.MouseButton.RightButton:
				if self.moveCameraButtonPressed:
					self.moveCameraButtonPressed = False
					self.setCursor(Qt.CursorShape.ArrowCursor)
					QCursor.setPos(self.savedMousePosition)
					self.mouseCursorDifference = np.zeros(2, dtype=np.float32)
		e.accept()

	def mouseMoveEvent(self, e):
		if not self.anyEntityTransformKeyPressed():
			if e.buttons() & Qt.MouseButton.RightButton:
				if self.moveCameraButtonPressed:
					mouseCursorCurrentPosition = np.array([e.pos().x(), e.pos().y()], dtype=np.float32)
					widgetCenter = QPoint(int(self.width() / 2), int(self.height() / 2))
					self.mouseCursorPreviousPosition = np.array([widgetCenter.x(), widgetCenter.y()], dtype=np.float32)
					self.mouseCursorDifference = np.subtract(mouseCursorCurrentPosition, self.mouseCursorPreviousPosition)
					QCursor.setPos(self.mapToGlobal(widgetCenter))
		else:
			if globalInfo.currentEntityID != -1:
				mouseCursorCurrentPosition = np.array([e.pos().x(), self.height() - e.pos().y()], dtype=np.float32)
				if self.translateEntityKeyPressed:
					worldSpaceCursorCurrentPosition = MathHelper.unproject(mouseCursorCurrentPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorPreviousPosition = MathHelper.unproject(self.mouseCursorPreviousPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorDifference = np.subtract(worldSpaceCursorCurrentPosition, worldSpaceCursorPreviousPosition)
					if np.dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0:
						if not self.cameraProjectionOrthographic:
							cameraEntityDifference = np.subtract(self.entityMoveTransform.position, self.camera.perspectivePosition)
							if np.dot(cameraEntityDifference, cameraEntityDifference) != 0.0:
								worldSpaceCursorDifferenceNormalized = MathHelper.normalize(worldSpaceCursorDifference)
								worldSpaceCursorDifferenceLength = np.linalg.norm(worldSpaceCursorDifference)
								cameraEntityDifferenceLength = np.linalg.norm(cameraEntityDifference)
								coefficient = (cameraEntityDifferenceLength * worldSpaceCursorDifferenceLength) / self.camera.nearPlane
								self.entityMoveTransform.position += worldSpaceCursorDifferenceNormalized * coefficient
						else:
							self.entityMoveTransform.position += worldSpaceCursorDifference
				elif self.rotateEntityKeyPressed:
					if not self.cameraProjectionOrthographic:
						rotationMatrix = MathHelper.rotate((mouseCursorCurrentPosition[0] - self.mouseCursorPreviousPosition[0]) / self.width(), self.camera.perspectiveDirection)
					else:
						rotationMatrix = MathHelper.rotate((mouseCursorCurrentPosition[0] - self.mouseCursorPreviousPosition[0]) / self.width(), self.camera.orthographicDirection)
					rotationAngles = np.array([np.rad2deg(np.arctan2(rotationMatrix[9], rotationMatrix[10])), np.rad2deg(np.arctan2(-rotationMatrix[8], np.sqrt((rotationMatrix[9] * rotationMatrix[9]) + (rotationMatrix[10] * rotationMatrix[10])))), np.rad2deg(np.arctan2(rotationMatrix[4], rotationMatrix[0]))], dtype=np.float32)
					self.entityMoveTransform.rotation -= rotationAngles
					self.entityMoveTransform.rotation %= 360.0
				elif self.scaleEntityKeyPressed:
					worldSpaceCursorCurrentPosition = MathHelper.unproject(mouseCursorCurrentPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorPreviousPosition = MathHelper.unproject(self.mouseCursorPreviousPosition, self.width(), self.height(), self.camera.invViewMatrix, self.camera.invProjMatrix)
					worldSpaceCursorDifference = np.subtract(worldSpaceCursorCurrentPosition, worldSpaceCursorPreviousPosition)
					if np.dot(worldSpaceCursorDifference, worldSpaceCursorDifference) != 0.0:
						worldSpaceCursorDifferenceLength = np.linalg.norm(worldSpaceCursorDifference)
						worldSpaceCursorPreviousEntityDifference = np.subtract(worldSpaceCursorPreviousPosition, self.entityMoveTransform.position)
						if np.dot(worldSpaceCursorPreviousEntityDifference, worldSpaceCursorPreviousEntityDifference) != 0.0:
							scaleFactor = 1.0
							if not self.cameraProjectionOrthographic:
								scaleFactor = 1000.0
							self.entityMoveTransform.scale += ((worldSpaceCursorDifferenceLength * scaleFactor) / np.linalg.norm(worldSpaceCursorPreviousEntityDifference)) * (1.0 if np.dot(worldSpaceCursorDifference, worldSpaceCursorPreviousEntityDifference) > 0.0 else -1.0)
				self.mouseCursorPreviousPosition = mouseCursorCurrentPosition
		e.accept()

	def wheelEvent(self, e):
		self.mouseScrollY = e.angleDelta().y() / 120.0
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

	def onBackfaceCullingToggled(self, backfaceCullingEnabled):
		self.backfaceCullingEnabled = backfaceCullingEnabled

	def onCamerasVisibilityToggled(self, showCameras):
		self.showCameras = showCameras

	def onCameraProjectionSwitched(self, cameraProjectionOrthographic):
		self.cameraProjectionOrthographic = cameraProjectionOrthographic

	def onCameraReset(self):
		if not self.cameraProjectionOrthographic:
			self.camera.perspectivePosition = np.copy(self.camera.basePerspectivePosition)
			self.camera.perspectiveDirection = np.copy(self.camera.basePerspectiveDirection)
			self.camera.perspectiveYaw = np.rad2deg(np.arctan2(self.camera.perspectiveDirection[2], self.camera.perspectiveDirection[0]))
			self.camera.perspectivePitch = np.rad2deg(-np.arcsin(self.camera.perspectiveDirection[1]))
		else:
			self.camera.orthographicPosition = np.copy(self.camera.baseOrthographicPosition)
			self.camera.orthographicDirection = np.copy(self.camera.baseOrthographicDirection)
			self.camera.orthographicUp = np.copy(self.camera.baseOrthographicUp)
			self.camera.orthographicHalfExtent = self.camera.baseOrthographicHalfExtent

	def onOrthographicCameraToAxisChanged(self, axis):
		self.cameraProjectionOrthographic = True
		self.camera.orthographicPosition = np.array([0.0, 0.0, 0.0], dtype=np.float32)
		self.camera.orthographicDirection = np.array(axis, dtype=np.float32)
		if (axis[1] ==  -1.0) or (axis[1] == 1.0):
			self.camera.orthographicUp = np.array([0.0, 0.0, -1.0], dtype=np.float32)
		else:
			self.camera.orthographicUp = np.array([0.0, 1.0, 0.0], dtype=np.float32)

class CreateEntityCommand(QUndoCommand):
	def __init__(self, name):
		super().__init__()
		self.setText("Create Entity " + name)
		self.entityName = name
		self.entityID = -1

	def undo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.entityID)]
		globalInfo.signalEmitter.destroyEntitySignal.emit(self.entityID)

	def redo(self):
		newEntity = Entity(self.entityName, self.entityID)
		self.entityID = newEntity.entityID
		globalInfo.entities.append(newEntity)
		globalInfo.signalEmitter.createEntitySignal.emit(self.entityID)

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

class CopyEntityCommand(QUndoCommand):
	def __init__(self, entity):
		super().__init__()
		self.copiedEntity = copy.deepcopy(entity)
		entityNameIndex = 0
		while globalInfo.findEntityByName(self.copiedEntity.name + "_" + str(entityNameIndex)) != -1:
			entityNameIndex += 1
		self.pastedEntityName = self.copiedEntity.name + "_" + str(entityNameIndex)
		self.setText("Copy Entity " + entity.name + " to Entity " + self.pastedEntityName)

	def undo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.pastedEntityID)]
		globalInfo.signalEmitter.destroyEntitySignal.emit(self.pastedEntityID)

	def redo(self):
		pastedEntity = copy.deepcopy(self.copiedEntity)
		pastedEntity.entityID = globalInfo.entityID
		globalInfo.entityID += 1
		pastedEntity.name = self.pastedEntityName
		globalInfo.entities.append(pastedEntity)
		self.pastedEntityID = pastedEntity.entityID
		globalInfo.signalEmitter.createEntitySignal.emit(self.pastedEntityID)
		globalInfo.currentEntityID = self.pastedEntityID
		globalInfo.signalEmitter.selectEntitySignal.emit(self.pastedEntityID)

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

class AddComponentEntityCommand(QUndoCommand):
	def __init__(self, entityID, componentName):
		super().__init__()
		self.setText("Add " + componentName + " Component to Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.componentName = componentName
		self.entityID = entityID

	def undo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()]
		if self.componentName == "Camera":
			globalInfo.signalEmitter.removeEntityCameraSignal.emit(self.entityID)
		elif self.componentName == "Light":
			globalInfo.signalEmitter.removeEntityLightSignal.emit(self.entityID)
		elif self.componentName == "Renderable":
			globalInfo.signalEmitter.removeEntityRenderableSignal.emit(self.entityID)
		elif self.componentName == "Rigidbody":
			globalInfo.signalEmitter.removeEntityRigidbodySignal.emit(self.entityID)
		elif self.componentName == "Collidable":
			globalInfo.signalEmitter.removeEntityCollidableSignal.emit(self.entityID)
		elif self.componentName == "Scriptable":
			globalInfo.signalEmitter.removeEntityScriptableSignal.emit(self.entityID)

	def redo(self):
		if self.componentName == "Camera":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["camera"] = Camera()
			globalInfo.signalEmitter.addEntityCameraSignal.emit(self.entityID)
		elif self.componentName == "Light":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["light"] = Light()
			globalInfo.signalEmitter.addEntityLightSignal.emit(self.entityID)
		elif self.componentName == "Renderable":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["renderable"] = Renderable()
			globalInfo.signalEmitter.addEntityRenderableSignal.emit(self.entityID)
		elif self.componentName == "Rigidbody":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["rigidbody"] = Rigidbody()
			globalInfo.signalEmitter.addEntityRigidbodySignal.emit(self.entityID)
		elif self.componentName == "Collidable":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["collidable"] = Collidable()
			globalInfo.signalEmitter.addEntityCollidableSignal.emit(self.entityID)
		elif self.componentName == "Scriptable":
			globalInfo.entities[globalInfo.findEntityById(self.entityID)].components["scriptable"] = Scriptable()
			globalInfo.signalEmitter.addEntityScriptableSignal.emit(self.entityID)

class RemoveComponentEntityCommand(QUndoCommand):
	def __init__(self, entityID, componentName):
		super().__init__()
		self.setText("Remove " + componentName + " Component to Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name)
		self.componentName = componentName
		self.entityID = entityID
		self.component = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()])

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()] = copy.deepcopy(self.component)
		if self.componentName == "Camera":
			globalInfo.signalEmitter.addEntityCameraSignal.emit(self.entityID)
		elif self.componentName == "Light":
			globalInfo.signalEmitter.addEntityLightSignal.emit(self.entityID)
		elif self.componentName == "Renderable":
			globalInfo.signalEmitter.addEntityRenderableSignal.emit(self.entityID)
		elif self.componentName == "Rigidbody":
			globalInfo.signalEmitter.addEntityRigidbodySignal.emit(self.entityID)
		elif self.componentName == "Collidable":
			globalInfo.signalEmitter.addEntityCollidableSignal.emit(self.entityID)
		elif self.componentName == "Scriptable":
			globalInfo.signalEmitter.addEntityScriptableSignal.emit(self.entityID)

	def redo(self):
		del globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()]
		if self.componentName == "Camera":
			globalInfo.signalEmitter.removeEntityCameraSignal.emit(self.entityID)
		elif self.componentName == "Light":
			globalInfo.signalEmitter.removeEntityLightSignal.emit(self.entityID)
		elif self.componentName == "Renderable":
			globalInfo.signalEmitter.removeEntityRenderableSignal.emit(self.entityID)
		elif self.componentName == "Rigidbody":
			globalInfo.signalEmitter.removeEntityRigidbodySignal.emit(self.entityID)
		elif self.componentName == "Collidable":
			globalInfo.signalEmitter.removeEntityCollidableSignal.emit(self.entityID)
		elif self.componentName == "Scriptable":
			globalInfo.signalEmitter.removeEntityScriptableSignal.emit(self.entityID)

class ChangeComponentEntityCommand(QUndoCommand):
	def __init__(self, entityID, component):
		super().__init__()
		self.componentName = type(component).__name__
		self.setText("Change Entity " + globalInfo.entities[globalInfo.findEntityById(entityID)].name + " " + self.componentName + " Component")
		self.entityID = entityID
		self.previousComponent = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(entityID)].components[self.componentName.lower()])
		self.newComponent = copy.deepcopy(component)

	def undo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()] = copy.deepcopy(self.previousComponent)
		if self.componentName == "Transform":
			globalInfo.signalEmitter.changeEntityTransformSignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Camera":
			globalInfo.signalEmitter.changeEntityCameraSignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Light":
			globalInfo.signalEmitter.changeEntityLightSignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Renderable":
			globalInfo.signalEmitter.changeEntityRenderableSignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Rigidbody":
			globalInfo.signalEmitter.changeEntityRigidbodySignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Collidable":
			globalInfo.signalEmitter.changeEntityCollidableSignal.emit(self.entityID, self.previousComponent)
		elif self.componentName == "Scriptable":
			globalInfo.signalEmitter.changeEntityScriptableSignal.emit(self.entityID, self.previousComponent)

	def redo(self):
		globalInfo.entities[globalInfo.findEntityById(self.entityID)].components[self.componentName.lower()] = copy.deepcopy(self.newComponent)
		if self.componentName == "Transform":
			globalInfo.signalEmitter.changeEntityTransformSignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Camera":
			globalInfo.signalEmitter.changeEntityCameraSignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Light":
			globalInfo.signalEmitter.changeEntityLightSignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Renderable":
			globalInfo.signalEmitter.changeEntityRenderableSignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Rigidbody":
			globalInfo.signalEmitter.changeEntityRigidbodySignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Collidable":
			globalInfo.signalEmitter.changeEntityCollidableSignal.emit(self.entityID, self.newComponent)
		elif self.componentName == "Scriptable":
			globalInfo.signalEmitter.changeEntityScriptableSignal.emit(self.entityID, self.newComponent)

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
		globalInfo.signalEmitter.toggleCurrentEntityVisibilitySignal.connect(self.onToggleCurrentEntityVisibility)

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

	def onToggleCurrentEntityVisibility(self, isVisible):
		font = self.findItemWithEntityID(globalInfo.currentEntityID).font()
		if isVisible:
			font.setItalic(False)
		else:
			font.setItalic(True)
		self.findItemWithEntityID(globalInfo.currentEntityID).setFont(font)

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
		self.setLayout(QVBoxLayout())
		self.layout().setContentsMargins(2, 2, 0, 2)
		self.layout().addWidget(QLabel("Entity List"))
		self.entityList = EntityList()
		self.layout().addWidget(self.entityList)

class BooleanWidget(QWidget):
	stateChanged = pyqtSignal(bool)

	def __init__(self, name):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.checkBox = QCheckBox()
		self.layout().addWidget(self.checkBox, 0, Qt.AlignmentFlag.AlignRight)
		self.checkBox.stateChanged.connect(self.onStateChanged)

	def onStateChanged(self):
		self.stateChanged.emit(self.checkBox.isChecked())

class ScalarWidget(QWidget):
	editingFinished = pyqtSignal(float)

	def __init__(self, name):
		super().__init__()
		self.previousValue = 0.0
		useDot = QLocale(QLocale.Language.English, QLocale.Country.UnitedStates)
		doubleValidator = QDoubleValidator()
		doubleValidator.setLocale(useDot)
		doubleValidator.setNotation(QDoubleValidator.Notation.StandardNotation)
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.valueLineEdit = QLineEdit("0.0")
		self.valueLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.valueLineEdit, 0, Qt.AlignmentFlag.AlignRight)
		self.valueLineEdit.editingFinished.connect(self.onEditingFinished)

	def onEditingFinished(self):
		newValue = float(self.valueLineEdit.text())
		if self.previousValue != newValue:
			self.previousValue = newValue
			self.editingFinished.emit(newValue)

class Vector2Widget(QWidget):
	editingFinished = pyqtSignal(float, float)

	def __init__(self, name):
		super().__init__()
		self.previousX = 0.0
		self.previousY = 0.0
		useDot = QLocale(QLocale.Language.English, QLocale.Country.UnitedStates)
		doubleValidator = QDoubleValidator()
		doubleValidator.setLocale(useDot)
		doubleValidator.setNotation(QDoubleValidator.Notation.StandardNotation)
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.xLabel = QLabel("x:")
		self.layout().addWidget(self.xLabel, 0, Qt.AlignmentFlag.AlignRight)
		self.xLineEdit = QLineEdit("0.0")
		self.xLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.xLineEdit, 0, Qt.AlignmentFlag.AlignRight)
		self.yLabel = QLabel("y:")
		self.layout().addWidget(self.yLabel, 0, Qt.AlignmentFlag.AlignRight)
		self.yLineEdit = QLineEdit("0.0")
		self.yLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.yLineEdit, 0, Qt.AlignmentFlag.AlignRight)
		self.xLineEdit.editingFinished.connect(self.onEditingFinished)
		self.yLineEdit.editingFinished.connect(self.onEditingFinished)

	def onEditingFinished(self):
		newX = float(self.xLineEdit.text())
		newY = float(self.yLineEdit.text())
		if (self.previousX != newX) or (self.previousY != newY):
			self.previousX = newX
			self.previousY = newY
			self.editingFinished.emit(newX, newY)

class Vector3Widget(QWidget):
	editingFinished = pyqtSignal(float, float, float)

	def __init__(self, name):
		super().__init__()
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
		self.layout().addWidget(self.xLabel, 0, Qt.AlignmentFlag.AlignRight)
		self.xLineEdit = QLineEdit("0.0")
		self.xLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.xLineEdit, 0, Qt.AlignmentFlag.AlignRight)
		self.yLabel = QLabel("y:")
		self.layout().addWidget(self.yLabel, 0, Qt.AlignmentFlag.AlignRight)
		self.yLineEdit = QLineEdit("0.0")
		self.yLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.yLineEdit, 0, Qt.AlignmentFlag.AlignRight)
		self.zLabel = QLabel("z:")
		self.layout().addWidget(self.zLabel, 0, Qt.AlignmentFlag.AlignRight)
		self.zLineEdit = QLineEdit("0.0")
		self.zLineEdit.setValidator(doubleValidator)
		self.layout().addWidget(self.zLineEdit, 0, Qt.AlignmentFlag.AlignRight)
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

class ComboBoxWidget(QWidget):
	elementSelected = pyqtSignal(str)

	def __init__(self, name, elements):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.comboBox = QComboBox()
		self.comboBox.addItems(elements)
		self.layout().addWidget(self.comboBox, 0, Qt.AlignmentFlag.AlignRight)
		self.comboBox.currentTextChanged.connect(self.onElementSelected)

	def onElementSelected(self, element):
		self.elementSelected.emit(element)

class ColorPickerWidget(QWidget):
	colorChanged = pyqtSignal(float, float, float, float)

	def __init__(self, name, defaultColor):
		super().__init__()
		self.previousR = defaultColor[0]
		self.previousG = defaultColor[1]
		self.previousB = defaultColor[2]
		self.previousA = defaultColor[3]
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.nameLabel = QLabel(name)
		self.layout().addWidget(self.nameLabel)
		self.colorButton = QPushButton()
		self.colorButton.setText("(" + format(defaultColor[0], ".2f") + ", " + format(defaultColor[1], ".2f") + ", " + format(defaultColor[2], ".2f") + ", " + format(defaultColor[3], ".2f") + ")")
		buttonPalette = self.colorButton.palette()
		buttonPalette.setColor(QPalette.ColorRole.Button, QColor.fromRgbF(defaultColor[0], defaultColor[1], defaultColor[2]))
		buttonPalette.setColor(QPalette.ColorRole.ButtonText, QColor.fromRgbF(1.0 - np.clip(defaultColor[0], 0.0, 1.0), 1.0 - np.clip(defaultColor[1], 0.0, 1.0), 1.0 - np.clip(defaultColor[2], 0.0, 1.0)))
		self.colorButton.setAutoFillBackground(True)
		self.colorButton.setPalette(buttonPalette)
		self.colorButton.update()
		self.layout().addWidget(self.colorButton, 0, Qt.AlignmentFlag.AlignRight)
		self.colorButton.clicked.connect(self.onColorButtonClicked)

	def onColorButtonClicked(self):
		newColor = QColorDialog.getColor(initial=QColor.fromRgbF(self.previousR, self.previousG, self.previousB, self.previousA), title="Select a color")
		if (newColor.redF() != self.previousR) or (newColor.greenF() != self.previousG) or (newColor.blueF() != self.previousB) or (newColor.alphaF() != self.previousA):
			self.previousR = newColor.redF()
			self.previousG = newColor.greenF()
			self.previousB = newColor.blueF()
			self.previousA = newColor.alphaF()
			self.colorChanged.emit(newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF())

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
		self.layout().addWidget(self.filePathButton, 0, Qt.AlignmentFlag.AlignRight)
		self.filePathButton.clicked.connect(self.onFilePathButtonClicked)

	def onFilePathButtonClicked(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle(self.filePathButton.text())
		if self.filePath != "":
			fileDialog.setDirectory(self.filePath.rsplit("/", 1)[0])
		elif globalInfo.projectDirectory != ".":
			fileDialog.setDirectory(globalInfo.projectDirectory)
		if fileDialog.exec():
			self.filePath = fileDialog.selectedFiles()[0]
			self.filePathLabel.setText(self.filePath.rsplit("/")[-1])
			self.filePathLabel.setToolTip(self.filePath)
			self.fileSelected.emit(self.filePath)

class SeparatorLine(QFrame):
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
		globalInfo.undoStack.push(RemoveComponentEntityCommand(globalInfo.currentEntityID, self.name))

class TransformComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(SeparatorLine())
		self.componentTitle = ComponentTitleWidget("Transform")
		self.layout().addWidget(self.componentTitle)
		self.positionWidget = Vector3Widget("Position")
		self.layout().addWidget(self.positionWidget)
		self.rotationWidget = Vector3Widget("Rotation")
		self.layout().addWidget(self.rotationWidget)
		self.scaleWidget = Vector3Widget("Scale")
		self.layout().addWidget(self.scaleWidget)
		self.layout().addWidget(SeparatorLine())
		self.positionWidget.editingFinished.connect(self.onTransformUpdated)
		self.rotationWidget.editingFinished.connect(self.onTransformUpdated)
		self.scaleWidget.editingFinished.connect(self.onTransformUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.changeEntityTransformSignal.connect(self.onChangeEntityTransform)

	def updateWidgets(self, transform):
		self.positionWidget.xLineEdit.setText(format(transform.position[0], ".3f"))
		self.positionWidget.yLineEdit.setText(format(transform.position[1], ".3f"))
		self.positionWidget.zLineEdit.setText(format(transform.position[2], ".3f"))
		self.positionWidget.previousX = transform.position[0]
		self.positionWidget.previousY = transform.position[1]
		self.positionWidget.previousZ = transform.position[2]
		self.rotationWidget.xLineEdit.setText(format(transform.rotation[0], ".3f"))
		self.rotationWidget.yLineEdit.setText(format(transform.rotation[1], ".3f"))
		self.rotationWidget.zLineEdit.setText(format(transform.rotation[2], ".3f"))
		self.rotationWidget.previousX = transform.rotation[0]
		self.rotationWidget.previousY = transform.rotation[1]
		self.rotationWidget.previousZ = transform.rotation[2]
		self.scaleWidget.xLineEdit.setText(format(transform.scale[0], ".3f"))
		self.scaleWidget.yLineEdit.setText(format(transform.scale[1], ".3f"))
		self.scaleWidget.zLineEdit.setText(format(transform.scale[2], ".3f"))
		self.scaleWidget.previousX = transform.scale[0]
		self.scaleWidget.previousY = transform.scale[1]
		self.scaleWidget.previousZ = transform.scale[2]

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
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newTransform))

class CameraComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Camera")
		self.layout().addWidget(self.componentTitle)
		self.forwardWidget = Vector3Widget("Forward")
		self.layout().addWidget(self.forwardWidget)
		self.upWidget = Vector3Widget("Up")
		self.layout().addWidget(self.upWidget)
		self.fovWidget = ScalarWidget("FOV")
		self.layout().addWidget(self.fovWidget)
		self.nearPlaneWidget = ScalarWidget("Near Plane")
		self.layout().addWidget(self.nearPlaneWidget)
		self.farPlaneWidget = ScalarWidget("Far Plane")
		self.layout().addWidget(self.farPlaneWidget)
		self.layout().addWidget(SeparatorLine())
		self.forwardWidget.editingFinished.connect(self.onCameraVector3Updated)
		self.upWidget.editingFinished.connect(self.onCameraVector3Updated)
		self.fovWidget.editingFinished.connect(self.onCameraScalarUpdated)
		self.nearPlaneWidget.editingFinished.connect(self.onCameraScalarUpdated)
		self.farPlaneWidget.editingFinished.connect(self.onCameraScalarUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityCameraSignal.connect(self.onAddEntityCamera)
		globalInfo.signalEmitter.removeEntityCameraSignal.connect(self.onRemoveEntityCamera)
		globalInfo.signalEmitter.changeEntityCameraSignal.connect(self.onChangeEntityCamera)

	def updateWidgets(self, camera):
		self.forwardWidget.xLineEdit.setText(format(camera.forward[0], ".3f"))
		self.forwardWidget.yLineEdit.setText(format(camera.forward[1], ".3f"))
		self.forwardWidget.zLineEdit.setText(format(camera.forward[2], ".3f"))
		self.forwardWidget.previousX = camera.forward[0]
		self.forwardWidget.previousY = camera.forward[1]
		self.forwardWidget.previousZ = camera.forward[2]
		self.upWidget.xLineEdit.setText(format(camera.up[0], ".3f"))
		self.upWidget.yLineEdit.setText(format(camera.up[1], ".3f"))
		self.upWidget.zLineEdit.setText(format(camera.up[2], ".3f"))
		self.upWidget.previousX = camera.up[0]
		self.upWidget.previousY = camera.up[1]
		self.upWidget.previousZ = camera.up[2]
		self.fovWidget.valueLineEdit.setText(format(camera.fov, ".3f"))
		self.fovWidget.previousValue = camera.fov
		self.nearPlaneWidget.valueLineEdit.setText(format(camera.nearPlane, ".3f"))
		self.nearPlaneWidget.previousValue = camera.nearPlane
		self.farPlaneWidget.valueLineEdit.setText(format(camera.farPlane, ".3f"))
		self.farPlaneWidget.previousValue = camera.farPlane

	def onAddEntityCamera(self, entityID):
		if entityID == globalInfo.currentEntityID:
			camera = globalInfo.entities[globalInfo.findEntityById(entityID)].components["camera"]
			self.updateWidgets(camera)
			self.show()

	def onRemoveEntityCamera(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityCamera(self, entityID, camera):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(camera)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "camera" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				camera = globalInfo.entities[globalInfo.findEntityById(entityID)].components["camera"]
				self.updateWidgets(camera)
			else:
				self.hide()

	def onCameraVector3Updated(self, x, y, z):
		newCamera = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["camera"])
		sender = self.sender()
		if sender == self.forwardWidget:
			newCamera.forward = [x, y, z]
		elif sender == self.upWidget:
			newCamera.up = [x, y, z]
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCamera))

	def onCameraScalarUpdated(self, value):
		newCamera = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["camera"])
		sender = self.sender()
		if sender == self.fovWidget:
			newCamera.fov = value
		elif sender == self.nearPlaneWidget:
			newCamera.nearPlane = value
		elif sender == self.farPlaneWidget:
			newCamera.farPlane = value
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCamera))

class LightComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Light")
		self.layout().addWidget(self.componentTitle)
		self.typeWidget = ComboBoxWidget("Type", ["Directional", "Point", "Spot"])
		self.layout().addWidget(self.typeWidget)
		self.colorWidget = ColorPickerWidget("Color", [1.0, 1.0, 1.0, 1.0])
		self.layout().addWidget(self.colorWidget)
		self.directionWidget = Vector3Widget("Direction")
		self.layout().addWidget(self.directionWidget)
		self.cutoffWidget = Vector2Widget("Cutoff")
		self.layout().addWidget(self.cutoffWidget)
		self.layout().addWidget(SeparatorLine())
		self.typeWidget.elementSelected.connect(self.onLightElementUpdated)
		self.colorWidget.colorChanged.connect(self.onLightColorUpdated)
		self.directionWidget.editingFinished.connect(self.onLightVector3Updated)
		self.cutoffWidget.editingFinished.connect(self.onLightVector2Updated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityLightSignal.connect(self.onAddEntityLight)
		globalInfo.signalEmitter.removeEntityLightSignal.connect(self.onRemoveEntityLight)
		globalInfo.signalEmitter.changeEntityLightSignal.connect(self.onChangeEntityLight)

	def updateWidgets(self, light):
		with QSignalBlocker(self.typeWidget.comboBox) as signalBlocker:
			self.typeWidget.comboBox.setCurrentText(light.type)
		self.colorWidget.colorButton.setText("(" + format(light.color[0], ".2f") + ", " + format(light.color[1], ".2f") + ", " + format(light.color[2], ".2f") + ", 1.00)")
		buttonPalette = self.colorWidget.colorButton.palette()
		buttonPalette.setColor(QPalette.ColorRole.Button, QColor.fromRgbF(light.color[0], light.color[1], light.color[2]))
		buttonPalette.setColor(QPalette.ColorRole.ButtonText, QColor.fromRgbF(1.0 - np.clip(light.color[0], 0.0, 1.0), 1.0 - np.clip(light.color[1], 0.0, 1.0), 1.0 - np.clip(light.color[2], 0.0, 1.0)))
		self.colorWidget.colorButton.setAutoFillBackground(True)
		self.colorWidget.colorButton.setPalette(buttonPalette)
		self.colorWidget.colorButton.update()
		self.colorWidget.previousR = light.color[0]
		self.colorWidget.previousG = light.color[1]
		self.colorWidget.previousB = light.color[2]
		self.colorWidget.previousA = 1.0
		self.directionWidget.xLineEdit.setText(format(light.direction[0], ".3f"))
		self.directionWidget.yLineEdit.setText(format(light.direction[1], ".3f"))
		self.directionWidget.zLineEdit.setText(format(light.direction[2], ".3f"))
		self.directionWidget.previousX = light.direction[0]
		self.directionWidget.previousY = light.direction[1]
		self.directionWidget.previousZ = light.direction[2]
		if (light.type == "Directional") or (light.type == "Spot"):
			self.directionWidget.setEnabled(True)
		else:
			self.directionWidget.setEnabled(False)
		self.cutoffWidget.xLineEdit.setText(format(light.cutoff[0], ".3f"))
		self.cutoffWidget.yLineEdit.setText(format(light.cutoff[1], ".3f"))
		self.cutoffWidget.previousX = light.cutoff[0]
		self.cutoffWidget.previousY = light.cutoff[1]
		if light.type == "Spot":
			self.cutoffWidget.setEnabled(True)
		else:
			self.cutoffWidget.setEnabled(False)

	def onAddEntityLight(self, entityID):
		if entityID == globalInfo.currentEntityID:
			light = globalInfo.entities[globalInfo.findEntityById(entityID)].components["light"]
			self.updateWidgets(light)
			self.show()

	def onRemoveEntityLight(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityLight(self, entityID, light):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(light)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "light" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				light = globalInfo.entities[globalInfo.findEntityById(entityID)].components["light"]
				self.updateWidgets(light)
			else:
				self.hide()

	def onLightElementUpdated(self, element):
		newLight = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["light"])
		sender = self.sender()
		if sender == self.typeWidget:
			newLight.type = element
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newLight))

	def onLightColorUpdated(self, r, g, b, a):
		newLight = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["light"])
		sender = self.sender()
		if sender == self.colorWidget:
			newLight.color = [r, g, b]
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newLight))

	def onLightVector3Updated(self, x, y, z):
		newLight = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["light"])
		sender = self.sender()
		if sender == self.directionWidget:
			newLight.direction = [x, y, z]
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newLight))

	def onLightVector2Updated(self, x, y):
		newLight = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["light"])
		sender = self.sender()
		if sender == self.cutoffWidget:
			newLight.cutoff = [x, y]
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newLight))

class RenderableComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Renderable")
		self.layout().addWidget(self.componentTitle)
		self.modelPathWidget = FileSelectorWidget("No model path", "Select a model")
		self.layout().addWidget(self.modelPathWidget)
		self.layout().addWidget(SeparatorLine())
		self.modelPathWidget.fileSelected.connect(self.onRenderableStringUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityRenderableSignal.connect(self.onAddEntityRenderable)
		globalInfo.signalEmitter.removeEntityRenderableSignal.connect(self.onRemoveEntityRenderable)
		globalInfo.signalEmitter.changeEntityRenderableSignal.connect(self.onChangeEntityRenderable)

	def updateWidgets(self, renderable):
		if renderable.modelPath != "":
			modelPath = os.path.normpath(renderable.modelPath).replace("\\", "/")
			if globalInfo.projectDirectory != ".":
				if os.path.isabs(modelPath):
					if modelPath.startswith(globalInfo.projectDirectory):
						modelPath = modelPath[len(globalInfo.projectDirectory) + 1:]
			self.modelPathWidget.filePathLabel.setText(modelPath.rsplit("/")[-1])
			self.modelPathWidget.filePathLabel.setToolTip(modelPath)
		else:
			self.modelPathWidget.filePathLabel.setText("No model path")
			self.modelPathWidget.filePathLabel.setToolTip("")

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

	def onRenderableStringUpdated(self, string):
		newRenderable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["renderable"])
		sender = self.sender()
		if sender == self.modelPathWidget:
			newRenderable.modelPath = os.path.normpath(os.path.abspath(string)).replace("\\", "/")
			globalInfo.rendererResourceManager.loadModel(newRenderable.modelPath)
			globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newRenderable))

class RigidbodyComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Rigidbody")
		self.layout().addWidget(self.componentTitle)
		self.isStaticWidget = BooleanWidget("Is Static")
		self.layout().addWidget(self.isStaticWidget)
		self.isAffectedByConstantsWidget = BooleanWidget("Is Affected By Constants")
		self.layout().addWidget(self.isAffectedByConstantsWidget)
		self.lockRotationWidget = BooleanWidget("Lock Rotation")
		self.layout().addWidget(self.lockRotationWidget)
		self.massWidget = ScalarWidget("Mass")
		self.layout().addWidget(self.massWidget)
		self.inertiaWidget = ScalarWidget("Inertia")
		self.layout().addWidget(self.inertiaWidget)
		self.restitutionWidget = ScalarWidget("Restitution")
		self.layout().addWidget(self.restitutionWidget)
		self.staticFrictionWidget = ScalarWidget("Static Friction")
		self.layout().addWidget(self.staticFrictionWidget)
		self.dynamicFrictionWidget = ScalarWidget("Dynamic Friction")
		self.layout().addWidget(self.dynamicFrictionWidget)
		self.layout().addWidget(SeparatorLine())
		self.isStaticWidget.stateChanged.connect(self.onRigidbodyBooleanUpdated)
		self.isAffectedByConstantsWidget.stateChanged.connect(self.onRigidbodyBooleanUpdated)
		self.lockRotationWidget.stateChanged.connect(self.onRigidbodyBooleanUpdated)
		self.massWidget.editingFinished.connect(self.onRigidbodyScalarUpdated)
		self.inertiaWidget.editingFinished.connect(self.onRigidbodyScalarUpdated)
		self.restitutionWidget.editingFinished.connect(self.onRigidbodyScalarUpdated)
		self.staticFrictionWidget.editingFinished.connect(self.onRigidbodyScalarUpdated)
		self.dynamicFrictionWidget.editingFinished.connect(self.onRigidbodyScalarUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityRigidbodySignal.connect(self.onAddEntityRigidbody)
		globalInfo.signalEmitter.removeEntityRigidbodySignal.connect(self.onRemoveEntityRigidbody)
		globalInfo.signalEmitter.changeEntityRigidbodySignal.connect(self.onChangeEntityRigidbody)

	def updateWidgets(self, rigidbody):
		with QSignalBlocker(self.isStaticWidget.checkBox) as signalBlocker:
			self.isStaticWidget.checkBox.setChecked(rigidbody.isStatic)
		with QSignalBlocker(self.isAffectedByConstantsWidget.checkBox) as signalBlocker:
			self.isAffectedByConstantsWidget.checkBox.setChecked(rigidbody.isAffectedByConstants)
		with QSignalBlocker(self.lockRotationWidget.checkBox) as signalBlocker:
			self.lockRotationWidget.checkBox.setChecked(rigidbody.lockRotation)
		self.massWidget.valueLineEdit.setText(format(rigidbody.mass, ".3f"))
		self.inertiaWidget.valueLineEdit.setText(format(rigidbody.inertia, ".3f"))
		self.restitutionWidget.valueLineEdit.setText(format(rigidbody.restitution, ".3f"))
		self.staticFrictionWidget.valueLineEdit.setText(format(rigidbody.staticFriction, ".3f"))
		self.dynamicFrictionWidget.valueLineEdit.setText(format(rigidbody.dynamicFriction, ".3f"))

	def onAddEntityRigidbody(self, entityID):
		if entityID == globalInfo.currentEntityID:
			rigidbody = globalInfo.entities[globalInfo.findEntityById(entityID)].components["rigidbody"]
			self.updateWidgets(rigidbody)
			self.show()

	def onRemoveEntityRigidbody(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityRigidbody(self, entityID, rigidbody):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(rigidbody)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "rigidbody" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				rigidbody = globalInfo.entities[globalInfo.findEntityById(entityID)].components["rigidbody"]
				self.updateWidgets(rigidbody)
			else:
				self.hide()

	def onRigidbodyBooleanUpdated(self, boolean):
		newRigidbody = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["rigidbody"])
		sender = self.sender()
		if sender == self.isStaticWidget:
			newRigidbody.isStatic = boolean
		elif sender == self.isAffectedByConstantsWidget:
			newRigidbody.isAffectedByConstants = boolean
		elif sender == self.lockRotationWidget:
			newRigidbody.lockRotation = boolean
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newRigidbody))

	def onRigidbodyScalarUpdated(self, scalar):
		newRigidbody = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["rigidbody"])
		sender = self.sender()
		if sender == self.massWidget:
			newRigidbody.mass = scalar
		elif sender == self.inertiaWidget:
			newRigidbody.inertia = scalar
		elif sender == self.restitutionWidget:
			newRigidbody.restitution = scalar
		elif sender == self.staticFrictionWidget:
			newRigidbody.staticFriction = scalar
		elif sender == self.dynamicFrictionWidget:
			newRigidbody.dynamicFriction = scalar
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newRigidbody))

class CollidableComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Collidable")
		self.layout().addWidget(self.componentTitle)
		self.typeWidget = ComboBoxWidget("Type", ["Box", "Sphere", "Capsule"])
		self.layout().addWidget(self.typeWidget)
		self.fromRenderableWidget = BooleanWidget("From Renderable")
		self.layout().addWidget(self.fromRenderableWidget)
		self.centerWidget = Vector3Widget("Center")
		self.layout().addWidget(self.centerWidget)
		self.radiusWidget = ScalarWidget("Radius")
		self.layout().addWidget(self.radiusWidget)
		self.halfExtentWidget = Vector3Widget("Half Extent")
		self.layout().addWidget(self.halfExtentWidget)
		self.rotationWidget = Vector3Widget("Rotation")
		self.layout().addWidget(self.rotationWidget)
		self.baseWidget = Vector3Widget("Base")
		self.layout().addWidget(self.baseWidget)
		self.tipWidget = Vector3Widget("Tip")
		self.layout().addWidget(self.tipWidget)
		self.layout().addWidget(SeparatorLine())
		self.typeWidget.elementSelected.connect(self.onCollidableElementUpdated)
		self.fromRenderableWidget.stateChanged.connect(self.onCollidableBooleanUpdated)
		self.centerWidget.editingFinished.connect(self.onCollidableVector3Updated)
		self.radiusWidget.editingFinished.connect(self.onCollidableScalarUpdated)
		self.halfExtentWidget.editingFinished.connect(self.onCollidableVector3Updated)
		self.rotationWidget.editingFinished.connect(self.onCollidableVector3Updated)
		self.baseWidget.editingFinished.connect(self.onCollidableVector3Updated)
		self.tipWidget.editingFinished.connect(self.onCollidableVector3Updated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityCollidableSignal.connect(self.onAddEntityCollidable)
		globalInfo.signalEmitter.removeEntityCollidableSignal.connect(self.onRemoveEntityCollidable)
		globalInfo.signalEmitter.changeEntityCollidableSignal.connect(self.onChangeEntityCollidable)

	def updateWidgets(self, collidable):
		with QSignalBlocker(self.typeWidget.comboBox) as signalBlocker:
			self.typeWidget.comboBox.setCurrentText(collidable.type)
		with QSignalBlocker(self.fromRenderableWidget.checkBox) as signalBlocker:
			self.fromRenderableWidget.checkBox.setChecked(collidable.fromRenderable)
		self.centerWidget.xLineEdit.setText(format(collidable.center[0], ".3f"))
		self.centerWidget.yLineEdit.setText(format(collidable.center[1], ".3f"))
		self.centerWidget.zLineEdit.setText(format(collidable.center[2], ".3f"))
		self.centerWidget.previousX = collidable.center[0]
		self.centerWidget.previousY = collidable.center[1]
		self.centerWidget.previousZ = collidable.center[2]
		if ((collidable.type == "Box") or (collidable.type == "Sphere")) and (not collidable.fromRenderable):
			self.centerWidget.setEnabled(True)
		else:
			self.centerWidget.setEnabled(False)
		self.radiusWidget.valueLineEdit.setText(format(collidable.radius, ".3f"))
		self.radiusWidget.previousValue = collidable.radius
		if ((collidable.type == "Sphere") or (collidable.type == "Capsule")) and (not collidable.fromRenderable):
			self.radiusWidget.setEnabled(True)
		else:
			self.radiusWidget.setEnabled(False)
		self.halfExtentWidget.xLineEdit.setText(format(collidable.halfExtent[0], ".3f"))
		self.halfExtentWidget.yLineEdit.setText(format(collidable.halfExtent[1], ".3f"))
		self.halfExtentWidget.zLineEdit.setText(format(collidable.halfExtent[2], ".3f"))
		self.halfExtentWidget.previousX = collidable.halfExtent[0]
		self.halfExtentWidget.previousY = collidable.halfExtent[1]
		self.halfExtentWidget.previousZ = collidable.halfExtent[2]
		if (collidable.type == "Box") and (not collidable.fromRenderable):
			self.halfExtentWidget.setEnabled(True)
		else:
			self.halfExtentWidget.setEnabled(False)
		self.rotationWidget.xLineEdit.setText(format(collidable.rotation[0], ".3f"))
		self.rotationWidget.yLineEdit.setText(format(collidable.rotation[1], ".3f"))
		self.rotationWidget.zLineEdit.setText(format(collidable.rotation[2], ".3f"))
		self.rotationWidget.previousX = collidable.rotation[0]
		self.rotationWidget.previousY = collidable.rotation[1]
		self.rotationWidget.previousZ = collidable.rotation[2]
		if (collidable.type == "Box") and (not collidable.fromRenderable):
			self.rotationWidget.setEnabled(True)
		else:
			self.rotationWidget.setEnabled(False)
		self.baseWidget.xLineEdit.setText(format(collidable.base[0], ".3f"))
		self.baseWidget.yLineEdit.setText(format(collidable.base[1], ".3f"))
		self.baseWidget.zLineEdit.setText(format(collidable.base[2], ".3f"))
		self.baseWidget.previousX = collidable.base[0]
		self.baseWidget.previousY = collidable.base[1]
		self.baseWidget.previousZ = collidable.base[2]
		if (collidable.type == "Capsule") and (not collidable.fromRenderable):
			self.baseWidget.setEnabled(True)
		else:
			self.baseWidget.setEnabled(False)
		self.tipWidget.xLineEdit.setText(format(collidable.tip[0], ".3f"))
		self.tipWidget.yLineEdit.setText(format(collidable.tip[1], ".3f"))
		self.tipWidget.zLineEdit.setText(format(collidable.tip[2], ".3f"))
		self.tipWidget.previousX = collidable.tip[0]
		self.tipWidget.previousY = collidable.tip[1]
		self.tipWidget.previousZ = collidable.tip[2]
		if (collidable.type == "Capsule") and (not collidable.fromRenderable):
			self.tipWidget.setEnabled(True)
		else:
			self.tipWidget.setEnabled(False)

	def onAddEntityCollidable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			collidable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["collidable"]
			self.updateWidgets(collidable)
			self.show()

	def onRemoveEntityCollidable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityCollidable(self, entityID, collidable):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(collidable)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "collidable" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				collidable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["collidable"]
				self.updateWidgets(collidable)
			else:
				self.hide()

	def onCollidableBooleanUpdated(self, boolean):
		newCollidable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["collidable"])
		sender = self.sender()
		if sender == self.fromRenderableWidget:
			newCollidable.fromRenderable = boolean
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCollidable))

	def onCollidableElementUpdated(self, element):
		newCollidable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["collidable"])
		sender = self.sender()
		if sender == self.typeWidget:
			newCollidable.type = element
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCollidable))

	def onCollidableVector3Updated(self, x, y, z):
		newCollidable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["collidable"])
		sender = self.sender()
		if sender == self.centerWidget:
			newCollidable.center = [x, y, z]
		elif sender == self.halfExtentWidget:
			newCollidable.halfExtent = [x, y, z]
		elif sender == self.rotationWidget:
			newCollidable.rotation = [x, y, z]
		elif sender == self.baseWidget:
			newCollidable.base = [x, y, z]
		elif sender == self.tipWidget:
			newCollidable.tip = [x, y, z]
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCollidable))

	def onCollidableScalarUpdated(self, value):
		newCollidable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["collidable"])
		sender = self.sender()
		if sender == self.radiusWidget:
			newCollidable.radius = value
		globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newCollidable))

class ScriptableComponentWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.componentTitle = ComponentTitleWidget("Scriptable")
		self.layout().addWidget(self.componentTitle)
		self.scriptPathWidget = FileSelectorWidget("No script selected", "Select a script")
		self.layout().addWidget(self.scriptPathWidget)
		self.layout().addWidget(SeparatorLine())
		self.scriptPathWidget.fileSelected.connect(self.onScriptableStringUpdated)
		globalInfo.signalEmitter.selectEntitySignal.connect(self.onSelectEntity)
		globalInfo.signalEmitter.addEntityScriptableSignal.connect(self.onAddEntityScriptable)
		globalInfo.signalEmitter.removeEntityScriptableSignal.connect(self.onRemoveEntityScriptable)
		globalInfo.signalEmitter.changeEntityScriptableSignal.connect(self.onChangeEntityScriptable)

	def updateWidgets(self, scriptable):
		if scriptable.scriptName != "":
			self.scriptPathWidget.filePathLabel.setText(scriptable.scriptName)
		else:
			self.scriptPathWidget.filePathLabel.setText("No script selected")

	def onAddEntityScriptable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			scriptable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["scriptable"]
			self.updateWidgets(scriptable)
			self.show()

	def onRemoveEntityScriptable(self, entityID):
		if entityID == globalInfo.currentEntityID:
			self.hide()

	def onChangeEntityScriptable(self, entityID, scriptable):
		if self.sender != self:
			if entityID == globalInfo.currentEntityID:
				self.updateWidgets(scriptable)

	def onSelectEntity(self, entityID):
		if entityID != -1:
			if "scriptable" in globalInfo.entities[globalInfo.findEntityById(entityID)].components.keys():
				self.show()
				scriptable = globalInfo.entities[globalInfo.findEntityById(entityID)].components["scriptable"]
				self.updateWidgets(scriptable)
			else:
				self.hide()

	def onScriptableStringUpdated(self, filePath):
		newScriptable = copy.deepcopy(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["scriptable"])
		sender = self.sender()
		if sender == self.scriptPathWidget:
			with open(filePath, 'r') as f:
				fileContent = f.read()
				scriptName = re.search("NTSHENGN_SCRIPT(.*)", fileContent)
				if scriptName != None:
					newScriptable.scriptPath = filePath
					newScriptable.scriptName = scriptName.group()[16:len(scriptName.group()) - 2].strip()
					globalInfo.undoStack.push(ChangeComponentEntityCommand(globalInfo.currentEntityID, newScriptable))
				else:
					if globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["scriptable"].scriptName != "":
						self.scriptPathWidget.filePathLabel.setText(globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components["scriptable"].scriptName)
					else:
						self.scriptPathWidget.filePathLabel.setText("No script selected")
					print(filePath + " is not a valid Script (missing NTSHENGN_SCRIPT(scriptName) macro")

class AddComponentMenu(QMenu):
	def __init__(self):
		super().__init__()
		self.addCameraComponentAction = self.addAction("Add Camera Component", self.addCameraComponent)
		self.addLightComponentAction = self.addAction("Add Light Component", self.addLightComponent)
		self.addRenderableComponentAction = self.addAction("Add Renderable Component", self.addRenderableComponent)
		self.addRigidbodyComponentAction = self.addAction("Add Rigidbody Component", self.addRigidbodyComponent)
		self.addCollidableComponentAction = self.addAction("Add Collidable Component", self.addCollidableComponent)
		self.addScriptableComponentAction = self.addAction("Add Scriptable Component", self.addScriptableComponent)

	def addCameraComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Camera"))

	def addLightComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Light"))

	def addRenderableComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Renderable"))

	def addRigidbodyComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Rigidbody"))

	def addCollidableComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Collidable"))

	def addScriptableComponent(self):
		globalInfo.undoStack.push(AddComponentEntityCommand(globalInfo.currentEntityID, "Scriptable"))

class AddComponentButton(QPushButton):
	def __init__(self):
		super().__init__()
		self.setText("Add Component")
		self.menu = AddComponentMenu()
		self.clicked.connect(self.onClick)

	def onClick(self):
		if "camera" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addCameraComponentAction.setEnabled(True)
		else:
			self.menu.addCameraComponentAction.setEnabled(False)
		if "light" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addLightComponentAction.setEnabled(True)
		else:
			self.menu.addLightComponentAction.setEnabled(False)
		if "renderable" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addRenderableComponentAction.setEnabled(True)
		else:
			self.menu.addRenderableComponentAction.setEnabled(False)
		if "rigidbody" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addRigidbodyComponentAction.setEnabled(True)
		else:
			self.menu.addRigidbodyComponentAction.setEnabled(False)
		if "collidable" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addCollidableComponentAction.setEnabled(True)
		else:
			self.menu.addCollidableComponentAction.setEnabled(False)
		if "scriptable" not in globalInfo.entities[globalInfo.findEntityById(globalInfo.currentEntityID)].components:
			self.menu.addScriptableComponentAction.setEnabled(True)
		else:
			self.menu.addScriptableComponentAction.setEnabled(False)
		self.menu.popup(QCursor.pos())

class ComponentList(QWidget):
	def __init__(self):
		super().__init__()
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignTop)
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.transformWidget = TransformComponentWidget()
		self.layout().addWidget(self.transformWidget)
		self.cameraWidget = CameraComponentWidget()
		self.layout().addWidget(self.cameraWidget)
		self.lightWidget = LightComponentWidget()
		self.layout().addWidget(self.lightWidget)
		self.renderableWidget = RenderableComponentWidget()
		self.layout().addWidget(self.renderableWidget)
		self.rigidbodyWidget = RigidbodyComponentWidget()
		self.layout().addWidget(self.rigidbodyWidget)
		self.collidableWidget = CollidableComponentWidget()
		self.layout().addWidget(self.collidableWidget)
		self.scriptableWidget = ScriptableComponentWidget()
		self.layout().addWidget(self.scriptableWidget)
		self.addComponentWidget = AddComponentButton()
		self.layout().addWidget(self.addComponentWidget)

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
		self.resize(185, self.height())
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
		self.setWindowTitle("NutshellEngine")
		self.setWindowIcon(QIcon("assets/icon.png"))
		self.main = QSplitter()
		self.setCentralWidget(self.main)
		self.createMenuBar()
		self.createEntityPanel()
		self.createRenderer()
		self.createEntityInfoPanel()

	def createMenuBar(self):
		menuBar = self.menuBar()
		self.fileMenu = FileMenu()
		menuBar.addMenu(self.fileMenu)
		self.editMenu = EditMenu()
		menuBar.addMenu(self.editMenu)
		self.viewMenu = ViewMenu()
		menuBar.addMenu(self.viewMenu)

	def createEntityPanel(self):
		self.entityPanel = EntityPanel()
		self.main.addWidget(self.entityPanel)

	def createRenderer(self):
		self.renderer = Renderer()
		self.main.addWidget(self.renderer)

	def createEntityInfoPanel(self):
		self.entityInfoPanel = EntityInfoPanel()
		self.main.addWidget(self.entityInfoPanel)

class OpenProjectWidget(QWidget):
	projectDirectorySelected = pyqtSignal(str)

	def __init__(self):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.directoryPathButton = QPushButton("Select a project directory")
		self.layout().addWidget(self.directoryPathButton)
		self.directoryPathButton.clicked.connect(self.onDirectoryPathButtonClicked)

	def onDirectoryPathButtonClicked(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle(self.directoryPathButton.text())
		fileDialog.setFileMode(QFileDialog.FileMode.Directory)
		if fileDialog.exec():
			self.projectDirectorySelected.emit(fileDialog.directory().path())

class NewProjectDirectoryPathWidget(QWidget):
	directorySelected = pyqtSignal(str)

	def __init__(self):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(QLabel("New project directory:"))
		self.directoryPathButton = QPushButton("Select a project directory")
		self.layout().addWidget(self.directoryPathButton)
		self.directoryPathButton.clicked.connect(self.onDirectoryPathButtonClicked)

	def onDirectoryPathButtonClicked(self):
		fileDialog = QFileDialog()
		fileDialog.setWindowTitle(self.directoryPathButton.text())
		fileDialog.setFileMode(QFileDialog.FileMode.Directory)
		if fileDialog.exec():
			self.directoryPath = fileDialog.directory().path()
			self.directoryPathButton.setText(self.directoryPath)
			self.directorySelected.emit(self.directoryPath)

class NewProjectNameWidget(QWidget):
	textChanged = pyqtSignal(str)

	def __init__(self):
		super().__init__()
		self.setLayout(QHBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.layout().addWidget(QLabel("New project name:"))
		self.projectNameLineEdit = QLineEdit()
		self.layout().addWidget(self.projectNameLineEdit)
		self.projectNameLineEdit.textChanged.connect(self.onTextChanged)

	def onTextChanged(self):
		self.textChanged.emit(self.projectNameLineEdit.text())

class NewProjectWidget(QWidget):
	newProjectButtonClicked = pyqtSignal(str)

	def __init__(self):
		super().__init__()
		self.projectDirectoryPath = ""
		self.projectName = ""
		self.setLayout(QVBoxLayout())
		self.layout().setContentsMargins(0, 0, 0, 0)
		self.newProjectDirectoryPathWidget = NewProjectDirectoryPathWidget()
		self.layout().addWidget(self.newProjectDirectoryPathWidget)
		self.newProjectNameWidget = NewProjectNameWidget()
		self.layout().addWidget(self.newProjectNameWidget)
		self.projectPathLabel = QLabel()
		self.layout().addWidget(self.projectPathLabel)
		self.createNewProjectButton = QPushButton("Create new project")
		self.createNewProjectButton.setEnabled(False)
		self.layout().addWidget(self.createNewProjectButton)
		self.createNewProjectButton.clicked.connect(self.onCreateNewProjectButtonClicked)
		self.newProjectDirectoryPathWidget.directorySelected.connect(self.onDirectorySelected)
		self.newProjectNameWidget.textChanged.connect(self.onTextChanged)

	def onCreateNewProjectButtonClicked(self):
		self.newProjectButtonClicked.emit(self.projectDirectoryPath + "/" + self.projectName)

	def onDirectorySelected(self, directoryPath):
		self.projectDirectoryPath = directoryPath
		self.projectPathLabel.setText(self.projectDirectoryPath + "/" + self.projectName)
		directoryExists = os.path.exists(self.projectDirectoryPath + "/" + self.projectName)
		if directoryExists:
			self.projectPathLabel.setText(self.projectPathLabel.text() + " (directory already exists)")
		if ((self.projectDirectoryPath != "") and (self.projectPathLabel != "")) and not directoryExists:
			self.createNewProjectButton.setEnabled(True)
		else:
			self.createNewProjectButton.setEnabled(False)

	def onTextChanged(self, text):
		self.projectName = text
		if self.projectDirectoryPath != "":
			self.projectPathLabel.setText(self.projectDirectoryPath + "/" + self.projectName)
		else:
			self.projectPathLabel.setText("?/" + self.projectName + " (missing directory)")
			return
		directoryExists = os.path.exists(self.projectDirectoryPath + "/" + self.projectName)
		if directoryExists:
			self.projectPathLabel.setText(self.projectPathLabel.text() + " (directory already exists)")
		if ((self.projectDirectoryPath != "") and (self.projectPathLabel != "")) and not directoryExists:
			self.createNewProjectButton.setEnabled(True)
		else:
			self.createNewProjectButton.setEnabled(False)

class ProjectWindow(QWidget):
	def __init__(self):
		super().__init__()
		self.resize(550, 400)
		self.setFixedSize(self.width(), self.height())
		self.setWindowTitle("NutshellEngine")
		self.setWindowIcon(QIcon("assets/icon.png"))
		self.setLayout(QVBoxLayout())
		self.layout().setAlignment(Qt.AlignmentFlag.AlignCenter)
		self.layout().addWidget(QLabel("<b>Open Project:</b>"))
		self.openProjectWidget = OpenProjectWidget()
		self.layout().addWidget(self.openProjectWidget)
		self.layout().addWidget(SeparatorLine())
		self.layout().addWidget(QLabel("<b>New Project:</b>"))
		self.newProjectWidget = NewProjectWidget()
		self.layout().addWidget(self.newProjectWidget)
		self.openProjectWidget.projectDirectorySelected.connect(self.onProjectDirectorySelected)
		self.newProjectWidget.newProjectButtonClicked.connect(self.onNewProjectButtonClicked)

	def openMainWindow(self, projectDirectory):
		globalInfo.projectDirectory = projectDirectory
		globalInfo.mainWindow.show()
		self.close()

	def onProjectDirectorySelected(self, projectDirectory):
		self.openMainWindow(projectDirectory)

	def onNewProjectButtonClicked(self, projectDirectory):
		os.mkdir(projectDirectory)
		self.openMainWindow(projectDirectory)

if __name__ == "__main__":
	app = QApplication([])
	app.setStyle("Fusion")

	globalInfo.signalEmitter = SignalEmitter()
	globalInfo.devicePixelRatio = app.devicePixelRatio()
	globalInfo.mainWindow = MainWindow()

	projectWindow = ProjectWindow()
	projectWindow.show()

	app.exec()