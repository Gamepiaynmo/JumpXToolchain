#include "stdafx.h"
#include "CharWindow.h"
#include "JumpXEditor.h"
#include <random>

std::default_random_engine re;
std::uniform_real_distribution<float> dis;
float rnd() { return dis(re); }

CharWindow::CharWindow(JumpXEditor *parent) : QOpenGLWidget(parent) {
	setFocusPolicy(Qt::ClickFocus);
	updateMatrix();
}

CharWindow::~CharWindow() {
	setupScene(nullptr, QDir());

	makeCurrent();
	delete m_missingTex;
	delete m_missingImage;
	delete m_geoProgram;
	delete m_normProgram;
	delete m_lineProgram;
	m_lineVao.destroy();
	m_lineVbo.destroy();
	delete m_boneProgram;
	m_boneVao.destroy();
	m_boneVbo.destroy();
	delete m_ribbonProgram;
	m_ribbonVao.destroy();
	m_ribbonVbo.destroy();
	delete m_partProgram;
	m_partVao.destroy();
	m_partVertVbo.destroy();
	m_partDataVbo.destroy();
	doneCurrent();
}

void CharWindow::setupScene(XScene* scene, QDir &texDir) {
	m_scene = scene;
	textureDir = texDir;

	makeCurrent();
	for (QOpenGLVertexArrayObject *vao : m_geoVao)
		delete vao;
	m_geoVao.clear();
	for (QOpenGLVertexArrayObject *vao : m_normVao)
		delete vao;
	m_normVao.clear();
	for (QOpenGLBuffer *vbo : m_geoVbo)
		delete vbo;
	m_geoVbo.clear();
	updateTextures();

	m_curFrame = 0;

	if (scene != nullptr) {
		auto createVbo = [](const void *data, int size, QOpenGLBuffer::Type type = QOpenGLBuffer::VertexBuffer) {
			QOpenGLBuffer *vbo = new QOpenGLBuffer(type);
			vbo->create(); vbo->bind();
			vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
			vbo->allocate(data, size);
			return vbo;
		};

		for (XGeometry &geo : scene->m_geometries) {
			QOpenGLVertexArrayObject *geoVao = new QOpenGLVertexArrayObject; geoVao->create();
			QOpenGLVertexArrayObject *normVao = new QOpenGLVertexArrayObject; normVao->create();

			if (geo.vertexData) {
				QOpenGLBuffer *posVbo = createVbo(geo.vertexData, sizeof(QVector3D) * geo.numVertex);
				geoVao->bind();
				m_geoProgram->enableAttributeArray(0);
				m_geoProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);
				normVao->bind();
				m_geoProgram->enableAttributeArray(0);
				m_geoProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);
				m_geoVbo.append(posVbo);
				posVbo->release();
			}
			if (geo.normalData) {
				QOpenGLBuffer *normalVbo = createVbo(geo.normalData, sizeof(QVector3D) * geo.numVertex);
				geoVao->bind();
				m_geoProgram->enableAttributeArray(1);
				m_geoProgram->setAttributeBuffer(1, GL_FLOAT, 0, 3);
				normVao->bind();
				m_geoProgram->enableAttributeArray(1);
				m_geoProgram->setAttributeBuffer(1, GL_FLOAT, 0, 3);
				m_geoVbo.append(normalVbo);
				normalVbo->release();
			}
			if (geo.boneData) {
				QOpenGLBuffer *boneVbo = createVbo(geo.boneData, sizeof(XBonePaletteEditor) * geo.numVertex);
				geoVao->bind();
				m_geoProgram->enableAttributeArray(5);
				glVertexAttribIPointer(5, 1, GL_INT, sizeof(XBonePaletteEditor), 0);
				m_geoProgram->enableAttributeArray(6);
				glVertexAttribIPointer(6, 4, GL_INT, sizeof(XBonePaletteEditor), (void *) 4);
				m_geoProgram->enableAttributeArray(7);
				m_geoProgram->setAttributeBuffer(7, GL_FLOAT, 20, 4, sizeof(XBonePaletteEditor));
				normVao->bind();
				m_geoProgram->enableAttributeArray(2);
				glVertexAttribIPointer(2, 1, GL_INT, sizeof(XBonePaletteEditor), 0);
				m_geoProgram->enableAttributeArray(3);
				glVertexAttribIPointer(3, 4, GL_INT, sizeof(XBonePaletteEditor), (void *) 4);
				m_geoProgram->enableAttributeArray(4);
				m_geoProgram->setAttributeBuffer(4, GL_FLOAT, 20, 4, sizeof(XBonePaletteEditor));
				m_geoVbo.append(boneVbo);
				boneVbo->release();
			}

			geoVao->bind();
			if (geo.uvData[0]) {
				QOpenGLBuffer *uvVbo = createVbo(geo.uvData[0], sizeof(QVector2D) * geo.numVertex);
				m_geoProgram->enableAttributeArray(2);
				m_geoProgram->setAttributeBuffer(2, GL_FLOAT, 0, 2);
				m_geoVbo.append(uvVbo);
				uvVbo->release();
			}
			if (geo.uvData[1]) {
				QOpenGLBuffer *uv1Vbo = createVbo(geo.uvData[1], sizeof(QVector2D) * geo.numVertex);
				m_geoProgram->enableAttributeArray(3);
				m_geoProgram->setAttributeBuffer(3, GL_FLOAT, 0, 2);
				m_geoVbo.append(uv1Vbo);
				uv1Vbo->release();
			}
			if (geo.vertColData) {
				QOpenGLBuffer *colorVbo = createVbo(geo.vertColData, sizeof(XCOLOR) * geo.numVertex);
				m_geoProgram->enableAttributeArray(4);
				m_geoProgram->setAttributeBuffer(4, GL_UNSIGNED_BYTE, 0, 4);
				m_geoVbo.append(colorVbo);
				colorVbo->release();
			} else m_geoProgram->setAttributeValue(4, QColor(255, 255, 255));
			if (geo.indicesData) {
				QOpenGLBuffer *indicesVbo = createVbo(geo.indicesData, sizeof(ushort) * 3 * geo.numFace, QOpenGLBuffer::IndexBuffer);
				m_geoVbo.append(indicesVbo);
			}

			m_geoVao.append(geoVao); geoVao->release();
			m_normVao.append(normVao); normVao->release();
		}

		QVector<int> parentBones;
		for (XBone &bone : scene->m_bones)
			parentBones.append(bone.parentId);
		m_boneVbo.bind();
		m_boneVbo.allocate(&parentBones[0], sizeof(int) * scene->m_head.nbon);
		m_boneVbo.release();

		m_ribbons.clear();
		m_ribbons.resize(scene->m_head.nrib);
		m_particles.clear();
		m_particles.resize(scene->m_head.nprt);
	}

	doneCurrent();
}

void CharWindow::initializeGL() {
#ifndef NDEBUG
	m_debugLogger = new QOpenGLDebugLogger(this);
	m_debugLogger->initialize();
#endif

	initializeOpenGLFunctions();

	m_missingImage = new QImage(64, 64, QImage::Format_RGBA8888);
	m_missingImage->fill(QColor(255, 255, 255));
	m_missingTex = new QOpenGLTexture(*m_missingImage, QOpenGLTexture::DontGenerateMipMaps);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

#ifdef NDEBUG
#define SHADER_PREFIX ":/"
#else
#define SHADER_PREFIX
#endif

	m_geoProgram = new QOpenGLShaderProgram();
	m_geoProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/model_vert.glsl");
	m_geoProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/model_frag.glsl");
	m_geoProgram->link();

	gMatCam = m_geoProgram->uniformLocation("matCam");
	gMatView = m_geoProgram->uniformLocation("matView");
	gLightDir = m_geoProgram->uniformLocation("lightDir");
	gAlphaBlend = m_geoProgram->uniformLocation("alphaBlend");
	gAlphaTest = m_geoProgram->uniformLocation("alphaTest");
	gUvClamp = m_geoProgram->uniformLocation("uvClamp");
	gSelected = m_geoProgram->uniformLocation("selected");
	gUvOffset = m_geoProgram->uniformLocation("uvOffset");
	gBoneMatrix = m_geoProgram->uniformLocation("boneMatrix");
	gMtlColor = m_geoProgram->uniformLocation("mtlColor");
	gUvAnim = m_geoProgram->uniformLocation("uvAnim");
	gUvTile = m_geoProgram->uniformLocation("uvTile");
	gMatBoneInv = m_geoProgram->uniformLocation("matBoneInv");
	gMatBone = m_geoProgram->uniformLocation("matBone");
	gBillboard = m_geoProgram->uniformLocation("billboard");
	gCenter = m_geoProgram->uniformLocation("center");
	gUseColor = m_geoProgram->uniformLocation("useColor");
	gUV2 = m_geoProgram->uniformLocation("uv2");
	gCartoon = m_geoProgram->uniformLocation("cartoon");
	gAmbientColor = m_geoProgram->uniformLocation("ambientColor");
	gShadowColor = m_geoProgram->uniformLocation("shadowColor");
	gAmbientInt = m_geoProgram->uniformLocation("ambientInt");
	gShadowTh = m_geoProgram->uniformLocation("shadowTh");
	gSpecularSmth = m_geoProgram->uniformLocation("specularSmth");
	gSpecular = m_geoProgram->uniformLocation("specular");

	m_geoProgram->bind();
	m_geoProgram->setUniformValue("texDiffuse", 0);
	m_geoProgram->setUniformValue("texBump", 1);
	m_geoProgram->setUniformValue("texSpecular", 2);
	m_geoProgram->setUniformValue("texLight", 3);
	m_geoProgram->setUniformValue("texCartSpec", 4);
	m_geoProgram->setUniformValue("texCartShadow", 5);
	m_geoProgram->setUniformValue("texDissolve", 6);

	m_normProgram = new QOpenGLShaderProgram();
	m_normProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/norm_vert.glsl");
	m_normProgram->addShaderFromSourceFile(QOpenGLShader::Geometry, SHADER_PREFIX"shaders/norm_geom.glsl");
	m_normProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/norm_frag.glsl");
	m_normProgram->link();

	nMatCam = m_normProgram->uniformLocation("matCam");
	nMatView = m_normProgram->uniformLocation("matView");
	nBoneMatrix = m_normProgram->uniformLocation("boneMatrix");
	nLineColor = m_normProgram->uniformLocation("lineColor");
	nMatBoneInv = m_normProgram->uniformLocation("matBoneInv");
	nMatBone = m_normProgram->uniformLocation("matBone");
	nBillboard = m_normProgram->uniformLocation("billboard");
	nCenter = m_normProgram->uniformLocation("center");

	m_lineProgram = new QOpenGLShaderProgram();
	m_lineProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/line_vert.glsl");
	m_lineProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/line_frag.glsl");
	m_lineProgram->link();
	m_lineVao.create(); m_lineVao.bind();
	m_lineVbo.create(); m_lineVbo.bind();
	m_lineVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_lineProgram->enableAttributeArray(0);
	m_lineProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D) * 2);
	m_lineProgram->enableAttributeArray(1);
	m_lineProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 3, sizeof(QVector3D) * 2);
	m_lineVbo.release();
	m_lineVao.release();

	lMatCam = m_lineProgram->uniformLocation("matCam");
	lMatView = m_lineProgram->uniformLocation("matView");

	m_boneProgram = new QOpenGLShaderProgram();
	m_boneProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/bone_vert.glsl");
	m_boneProgram->addShaderFromSourceFile(QOpenGLShader::Geometry, SHADER_PREFIX"shaders/bone_geom.glsl");
	m_boneProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/bone_frag.glsl");
	m_boneProgram->link();
	m_boneVao.create(); m_boneVao.bind();
	m_boneVbo.create(); m_boneVbo.bind();
	m_boneVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_boneProgram->enableAttributeArray(0);
	glVertexAttribIPointer(0, 1, GL_INT, 4, 0);
	m_boneVbo.release();
	m_boneVao.release();

	bMatCam = m_boneProgram->uniformLocation("matCam");
	bMatView = m_boneProgram->uniformLocation("matView");
	bBoneMatrix = m_boneProgram->uniformLocation("boneMatrix");
	bSelectId = m_boneProgram->uniformLocation("selectId");

	m_ribbonProgram = new QOpenGLShaderProgram();
	m_ribbonProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/ribbon_vert.glsl");
	m_ribbonProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/ribbon_frag.glsl");
	m_ribbonProgram->link();
	m_ribbonVao.create(); m_ribbonVao.bind();
	m_ribbonVbo.create(); m_ribbonVbo.bind();
	m_ribbonVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_ribbonProgram->enableAttributeArray(0);
	m_ribbonProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D) * 2);
	m_ribbonProgram->enableAttributeArray(1);
	m_ribbonProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 2, sizeof(QVector3D) * 2);
	m_ribbonVbo.release();
	m_ribbonVao.release();

	rMatCam = m_ribbonProgram->uniformLocation("matCam");
	rMatView = m_ribbonProgram->uniformLocation("matView");
	rRibColor = m_ribbonProgram->uniformLocation("ribColor");

	m_partProgram = new QOpenGLShaderProgram();
	m_partProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SHADER_PREFIX"shaders/part_vert.glsl");
	m_partProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SHADER_PREFIX"shaders/part_frag.glsl");
	m_partProgram->link();
	m_partVao.create(); m_partVao.bind();
	m_partVertVbo.create(); m_partVertVbo.bind();
	m_partVertVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	QVector2D partVertexData[] = {
		QVector2D(-1.0f, -1.0f), QVector2D(0.0f, 1.0f),
		QVector2D( 1.0f, -1.0f), QVector2D(1.0f, 1.0f),
		QVector2D(-1.0f,  1.0f), QVector2D(0.0f, 0.0f),
		QVector2D( 1.0f,  1.0f), QVector2D(1.0f, 0.0f),
	};
	m_partVertVbo.allocate(&partVertexData, sizeof(partVertexData));
	m_partProgram->enableAttributeArray(0);
	m_partProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(QVector2D) * 2);
	m_partProgram->enableAttributeArray(1);
	m_partProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector2D), 2, sizeof(QVector2D) * 2);
	m_partVertVbo.release();
	m_partDataVbo.create(); m_partDataVbo.bind();
	m_partDataVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_partProgram->enableAttributeArray(2);
	m_partProgram->setAttributeBuffer(2, GL_FLOAT, 0, 3, sizeof(ParticleInstance));
	glVertexAttribDivisor(2, 1);
	m_partProgram->enableAttributeArray(3);
	m_partProgram->setAttributeBuffer(3, GL_FLOAT, 12, 3, sizeof(ParticleInstance));
	glVertexAttribDivisor(3, 1);
	m_partProgram->enableAttributeArray(4);
	m_partProgram->setAttributeBuffer(4, GL_FLOAT, 24, 1, sizeof(ParticleInstance));
	glVertexAttribDivisor(4, 1);
	m_partProgram->enableAttributeArray(5);
	m_partProgram->setAttributeBuffer(5, GL_FLOAT, 28, 1, sizeof(ParticleInstance));
	glVertexAttribDivisor(5, 1);
	m_partProgram->enableAttributeArray(6);
	glVertexAttribIPointer(6, 1, GL_INT, sizeof(ParticleInstance), (void *) 32);
	glVertexAttribDivisor(6, 1);
	m_partProgram->enableAttributeArray(7);
	m_partProgram->setAttributeBuffer(7, GL_FLOAT, 60, 4, sizeof(ParticleInstance));
	glVertexAttribDivisor(7, 1);
	m_partProgram->enableAttributeArray(8);
	m_partProgram->setAttributeBuffer(8, GL_FLOAT, 76, 1, sizeof(ParticleInstance));
	glVertexAttribDivisor(8, 1);
	m_partDataVbo.release();
	m_partVao.release();

	pMatCam = m_partProgram->uniformLocation("matCam");
	pMatView = m_partProgram->uniformLocation("matView");
	pMatBone = m_partProgram->uniformLocation("matBone");
	pModelSpace = m_partProgram->uniformLocation("modelSpace");
	pAlignVel = m_partProgram->uniformLocation("alignVel");
	pMidTime = m_partProgram->uniformLocation("midTime");
	pColor = m_partProgram->uniformLocation("colors");
	pAlpha = m_partProgram->uniformLocation("alphas");
	pSize = m_partProgram->uniformLocation("sizes");
	pGrid = m_partProgram->uniformLocation("grid");
	pXyQuad = m_partProgram->uniformLocation("xyQuad");
	pTailMode = m_partProgram->uniformLocation("tailMode");
	pTailLength = m_partProgram->uniformLocation("tailLength");
	pTailSpeed = m_partProgram->uniformLocation("tailSpeed");
}

void CharWindow::updateMatrix() {
	m_matCam.setToIdentity();
	m_matCam.translate(0.0f, 0.0f, -m_distance);
	m_matCam.rotate(m_angle[0], 1.0f, 0.0f, 0.0f);
	m_matCam.rotate(m_angle[1], 0.0f, 0.0f, 1.0f);
	m_matCam.translate(0.0f, 0.0f, m_offset);
	m_lightDir = QVector3D(cosf(m_lightAngle[1] * g_PI / 180.0f) * sinf(m_lightAngle[0] * g_PI / 180.0f),
			sinf(m_lightAngle[1] * g_PI / 180.0f) * sinf(m_lightAngle[0] * g_PI / 180.0f),
			cosf(m_lightAngle[0] * g_PI / 180.0f));
}

void CharWindow::wheelEvent(QWheelEvent *event) {
	if (event->delta() > 0)
		m_distance /= 1.1f;
	else m_distance *= 1.1f;
	m_distance = max(1.0f, min(10000.0f, m_distance));
	updateMatrix();
}

void CharWindow::mouseMoveEvent(QMouseEvent *event) {
	if (moving) m_offset -= (event->localPos().y() - lastY) / 10;
	if (roting) {
		m_angle[0] += (event->localPos().y() - lastY) / 4;
		m_angle[1] += (event->localPos().x() - lastX) / 4;
		m_angle[0] = max(-180.0f, min(0.0f, m_angle[0]));
	}
	if (lightRoting) {
		m_lightAngle[0] -= (event->localPos().y() - lastY) / 4;
		m_lightAngle[1] += (event->localPos().x() - lastX) / 4;
		m_lightAngle[0] = max(-180.0f, min(0.0f, m_lightAngle[0]));
	}
	lastX = event->localPos().x();
	lastY = event->localPos().y();
	updateMatrix();
}

void CharWindow::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) roting = true;
	if (event->button() == Qt::MiddleButton) moving = true;
	if (event->button() == Qt::RightButton) lightRoting = true;
	lastX = event->localPos().x();
	lastY = event->localPos().y();
}

void CharWindow::mouseReleaseEvent(QMouseEvent *event) {
	moving = roting = lightRoting = false;
}

void CharWindow::setCurrentFrame(int frame) {
	if (m_scene) {
		if (m_selection.action == -1) {
			frame = max(0, min(m_scene->m_head.numKey - 1, frame));
			m_curFrame = frame;
		} else {
			int start = m_scene->m_actions[m_selection.action].startFrame,
				end = m_scene->m_actions[m_selection.action].endFrame;
			frame = max(start, min(end - 1, frame));
			m_curFrame = frame - start;
		}
	} else m_curFrame = 0;
}

int CharWindow::getCurrentFrame() {
	if (m_scene) {
		if (m_selection.action == -1) {
			return max(0, min(m_scene->m_head.numKey - 1, m_curFrame));
		} else {
			int start = m_scene->m_actions[m_selection.action].startFrame,
				end = m_scene->m_actions[m_selection.action].endFrame;
			return start + max(0, min(end - start - 1, m_curFrame));
		}
	} else return 0;
}

void CharWindow::resizeGL(int width, int height) {
	if (height == 0) height = 1;

	glViewport(0, 0, width, height);

	m_matView.setToIdentity();
	m_matView.perspective(60.0f, (float) width / height, 0.1f, 10000.0f);
}

void CharWindow::paintGL() {
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float delta = 1.0f / 32;
	int animFrame = 0;

#ifndef NDEBUG
	for (auto &msg : m_debugLogger->loggedMessages())
		messageLogged(msg);
#endif

	if (m_scene != nullptr) {
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, showWire ? GL_LINE : GL_FILL);

		if (m_selection.action == -1) {
			if (m_scene->m_head.numKey == 0) animFrame = -1;
			else animFrame = m_curFrame %= m_scene->m_head.numKey;
		} else {
			XAction &act = m_scene->m_actions[m_selection.action];
			if (act.startFrame == act.endFrame) m_curFrame = 0;
			else m_curFrame %= (act.endFrame - act.startFrame);
			animFrame = m_curFrame + act.startFrame;
		}

		if (animFrame >= m_scene->m_head.numKey)
			animFrame = -1;

		m_boneMatrix.resize(m_scene->m_head.nbon);
		m_boneTrans.resize(m_scene->m_head.nbon);
		for (int i = 0; i < m_scene->m_head.nbon; i++) {
			if (animFrame >= 0) {
				QMatrix4x4 boneMat;
				if (m_scene->m_bones[i].numPosKey > 0)
					boneMat.translate(m_scene->m_bones[i].posData[animFrame]);
				if (m_scene->m_bones[i].numRotKey > 0)
					boneMat.rotate(m_scene->m_bones[i].rotData[animFrame]);
				if (m_scene->m_bones[i].numScaleKey > 0)
					boneMat.scale(m_scene->m_bones[i].scaleData[animFrame]);
				m_boneTrans[i] = boneMat.toGenericMatrix<4, 3>();
				boneMat *= m_scene->m_bones[i].matrixInv;
				m_boneMatrix[i] = boneMat.toGenericMatrix<4, 3>();
			} else {
				m_boneTrans[i] = m_scene->m_bones[i].matrix.toGenericMatrix<4, 3>();
				m_boneMatrix[i] = QMatrix4x3();
			}
		}

		if (animFrame < 0)
			animFrame = 0;

		if (m_scene->m_head.ngeo > 0) {
			m_geoProgram->bind();
			m_geoProgram->setUniformValue(gMatCam, m_matCam);
			m_geoProgram->setUniformValue(gMatView, m_matView);
			m_geoProgram->setUniformValue(gLightDir, m_lightDir);
			glUniformMatrix3x4fv(gBoneMatrix, m_scene->m_head.nbon, GL_TRUE, m_boneMatrix[0].constData());

			for (bool pass : { true, false }) {
				if (pass) {
					glDisable(GL_BLEND);
					glDepthMask(GL_TRUE);
				}
				else glEnable(GL_BLEND);
				m_geoProgram->setUniformValue(gAlphaBlend, !pass);

				for (int i = 0; i < m_scene->m_geometries.size(); i++) {
					XGeometry &geo = m_scene->m_geometries[i];
					XMaterial &mtl = geo.materialId >= 0 ? m_scene->m_materials[geo.materialId] : missingMtl;

					if (bool(mtl.flag & RENDER_ALPHABLEND) == pass) continue;
					if (mtl.flag & RENDER_TWOSIDED)
						glDisable(GL_CULL_FACE);
					else glEnable(GL_CULL_FACE);
					if (!pass) {
						glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
						glDepthMask(mtl.flag & RENDER_ZWRITEENABLE);
					}
					m_geoProgram->setUniformValue(gAlphaTest, mtl.flag & RENDER_ALPHATEST);

					XCOLOR color = { 255, 255, 255, 255 };
					QVector2D uvOffset(mtl.uvSpeed[0] * m_timer, mtl.uvSpeed[1] * m_timer);
					if (mtl.colorKeys != nullptr) {
						XMaterialAnimDataStruct &animData = mtl.colorKeys[animFrame];
						if (geo.type != XGEO_NORMAL_MESH)
							color = animData.color;
						else color.a = animData.color[3]; // > 0 ? 255 : 0;
						uvOffset += QVector2D(animData.uvOffset[0], animData.uvOffset[1]);
						m_geoProgram->setUniformValue(gUvClamp, animData.blend & RENDER_UVCLAMP);

						if (mtl.flag & RENDER_ALPHABLEND) {
							if (animData.blend & RENDER_ADD)
								glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
							//if (animData.blend & RENDER_MODULATE)
							//	glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_DST_ALPHA, GL_ZERO);
							//if (animData.blend & RENDER_MODULATE2X)
							//	glBlendFuncSeparate(GL_DST_COLOR, GL_SRC_COLOR, GL_DST_ALPHA, GL_SRC_ALPHA);
						}
					} else m_geoProgram->setUniformValue(gUvClamp, false);
					m_geoProgram->setUniformValue(gUvOffset, uvOffset);
					if (geo.ancestorBone >= 0 && m_scene->m_bones[geo.ancestorBone].visibleData != 0 &&
						!m_scene->m_bones[geo.ancestorBone].visibleData[animFrame]) color.a = 0;
					if (geo.type == XGEO_BOUND_MESH) color.a = 0;
					m_geoProgram->setUniformValue(gMtlColor, QColor(color.r, color.g, color.b, color.a));
					m_geoProgram->setUniformValue(gUseColor, geo.type != XGEO_NORMAL_MESH);

					bool selected = (geo.objectId >= 0 && m_selection.geometry == geo.objectId) ||
						(geo.materialId >= 0 && m_selection.material == geo.materialId) ||
						(mtl.textureId >= 0 && mtl.textureId == m_selection.texture);
					m_geoProgram->setUniformValue(gSelected, selected);
					glUniform1i(gUvAnim, int(m_timer / delta) % (mtl.uTile * mtl.vTile));
					glUniform2iv(gUvTile, 1, &mtl.uTile);

					if (geo.type == XGEO_BILLBOARD && geo.ancestorBone >= 0) {
						m_geoProgram->setUniformValue(gCenter, geo.center);
						if (geo.flag & XGEO_FLAG_ALWAYSESTAND) {
							m_geoProgram->setUniformValue(gBillboard, 2);
							m_geoProgram->setUniformValue(gCenter, geo.center);
						} else if (geo.flag & XGEO_FLAG_VERTICALGROUND) {
							m_geoProgram->setUniformValue(gBillboard, 3);
							m_geoProgram->setUniformValue(gCenter, m_scene->m_bones[geo.ancestorBone].matrix.column(3).toVector3D());
						} else {
							m_geoProgram->setUniformValue(gBillboard, 1);
							m_geoProgram->setUniformValue(gCenter, geo.center);
						}
						m_geoProgram->setUniformValue(gMatBoneInv, m_scene->m_bones[geo.ancestorBone].matrixInv);
						m_geoProgram->setUniformValue(gMatBone, QMatrix4x4(m_boneTrans[geo.ancestorBone]));
					} else m_geoProgram->setUniformValue(gBillboard, 0);

					(mtl.textureId >= 0 && showTexture ? m_glTextures[mtl.textureId] : m_missingTex)->bind(0);
					m_geoProgram->setUniformValue(gUV2, geo.saveFlag & XFGEOCHUNKSAVE_ENABLE_UV2);

					if (mtl.exData.flag & EFFECT_SPECULAR) {
						m_geoProgram->setUniformValue(gSpecular, true);
						(mtl.exData.specTexId >= 0 ? m_glTextures[mtl.exData.specTexId] : m_missingTex)->bind(2);
					} else m_geoProgram->setUniformValue(gSpecular, false);

					if (mtl.exData.flag & EFFECT_CARTOON) {
						m_geoProgram->setUniformValue(gCartoon, true);
						glUniform3fv(gAmbientColor, 1, mtl.cartoonData.ambientColor);
						glUniform3fv(gShadowColor, 1, mtl.cartoonData.shadowColor);
						m_geoProgram->setUniformValue(gAmbientInt, mtl.cartoonData.ambientIntensity);
						m_geoProgram->setUniformValue(gShadowTh, mtl.cartoonData.shadowThreshold);
						m_geoProgram->setUniformValue(gSpecularSmth, mtl.cartoonData.specularSmoothness);
						(mtl.cartoonData.specTextureID >= 0 ? m_glTextures[mtl.cartoonData.specTextureID] : m_missingTex)->bind(4);
						(mtl.cartoonData.shadowTextureID >= 0 ? m_glTextures[mtl.cartoonData.shadowTextureID] : m_missingTex)->bind(5);
					} else m_geoProgram->setUniformValue(gCartoon, false);

					m_geoVao[i]->bind();
					glDrawElements(GL_TRIANGLES, geo.numFace * 3, GL_UNSIGNED_SHORT, 0);
				}
			}

			m_geoProgram->release();

			if (showNormal) {
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);

				m_normProgram->bind();
				m_normProgram->setUniformValue(nMatCam, m_matCam);
				m_normProgram->setUniformValue(nMatView, m_matView);
				glUniformMatrix3x4fv(nBoneMatrix, m_scene->m_head.nbon, GL_TRUE, m_boneMatrix[0].constData());

				for (int i = 0; i < m_scene->m_geometries.size(); i++) {
					XGeometry &geo = m_scene->m_geometries[i];
					XMaterial &mtl = geo.materialId >= 0 ? m_scene->m_materials[geo.materialId] : missingMtl;

					XCOLOR color = { 255, 255, 255, 255 };
					if (mtl.colorKeys != nullptr) {
						XMaterialAnimDataStruct &animData = mtl.colorKeys[animFrame];
						if (geo.type != XGEO_NORMAL_MESH)
							color = animData.color;
						else color.a = animData.color[3]; // > 0 ? 255 : 0;
					}
					if (geo.ancestorBone >= 0 && m_scene->m_bones[geo.ancestorBone].visibleData != 0 &&
						!m_scene->m_bones[geo.ancestorBone].visibleData[animFrame]) color.a = 0;
					m_normProgram->setUniformValue(nLineColor, QColor(0, 255, 0, color.a));

					if (geo.type == XGEO_BILLBOARD && geo.ancestorBone >= 0) {
						m_normProgram->setUniformValue(nCenter, geo.center);
						if (geo.flag & XGEO_FLAG_ALWAYSESTAND) {
							m_normProgram->setUniformValue(nBillboard, 2);
							m_normProgram->setUniformValue(nCenter, geo.center);
						} else if (geo.flag & XGEO_FLAG_VERTICALGROUND) {
							m_normProgram->setUniformValue(nBillboard, 3);
							m_normProgram->setUniformValue(nCenter, m_scene->m_bones[geo.ancestorBone].matrix.column(3).toVector3D());
						} else {
							m_normProgram->setUniformValue(nBillboard, 1);
							m_normProgram->setUniformValue(nCenter, geo.center);
						}
						m_normProgram->setUniformValue(nMatBoneInv, m_scene->m_bones[geo.ancestorBone].matrixInv);
						m_normProgram->setUniformValue(nMatBone, QMatrix4x4(m_boneTrans[geo.ancestorBone]));
					} else m_normProgram->setUniformValue(nBillboard, 0);

					m_normVao[i]->bind();
					glDrawArrays(GL_POINTS, 0, geo.numVertex);
				}

				m_normProgram->release();
			}
		}
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	if (showAxis) {
		m_lineProgram->bind();
		m_lineProgram->setUniformValue(lMatCam, m_matCam);
		m_lineProgram->setUniformValue(lMatView, m_matView);

		float vertexData[] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			m_lightDir.x() * 100.0f, m_lightDir.y() * 100.0f, m_lightDir.z() * 100.0f, 1.0f, 1.0f, 0.0f,
		};
		m_lineVbo.bind();
		m_lineVbo.allocate(vertexData, sizeof(vertexData));
		m_lineVao.bind();
		glDrawArrays(GL_LINES, 0, 8);

		m_lineProgram->release();
	}

	if (m_scene != nullptr) {
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		const float timeScale = 0.2f;
		if (m_scene->m_head.nrib > 0) {
			m_ribbonProgram->bind();
			m_ribbonProgram->setUniformValue(rMatCam, m_matCam);
			m_ribbonProgram->setUniformValue(rMatView, m_matView);

			for (int i = 0; i < m_scene->m_head.nrib; i++) {
				XRibbon &rib = m_scene->m_ribbons[i];
				RibbonEmitter &emitter = m_ribbons[i];
				if (rib.boneId >= 0) {
					bool visible = m_scene->m_bones[rib.boneId].visibleData == 0
						|| m_scene->m_bones[rib.boneId].visibleData[animFrame];
					if (visible) {
						QMatrix4x4 trans(m_boneMatrix[rib.boneId]);
						RibbonInstance instance;
						instance.startPos = trans * rib.startPos;
						instance.endPos = trans * rib.endPos;
						instance.lifeSpan = rib.edgeLifeSec * timeScale;
						emitter.instances.append(instance);
					}

					if (emitter.instances.size() > 1) {
						for (int i = 0; i < emitter.instances.size(); i++) {
							RibbonInstance &instance = emitter.instances[i];
							float texPos = (float) (emitter.instances.size() - i - 1) / (emitter.instances.size() - 1);
							instance.startUv = { texPos, 1.0f };
							instance.endUv = { texPos, 0.0f };
						}

						if (rib.blendMode)
							glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
						else glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
						m_ribbonProgram->setUniformValue(rRibColor, QColor(rib.ribEx.color[2], rib.ribEx.color[1], rib.ribEx.color[0], rib.ribEx.alpha));

						m_ribbonVbo.bind();
						m_ribbonVbo.allocate(&emitter.instances[0], sizeof(RibbonInstance) * emitter.instances.size());
						m_ribbonVao.bind();
						(rib.textureId >= 0 && showTexture ? m_glTextures[rib.textureId] : m_missingTex)->bind(0);
						glDrawArrays(GL_TRIANGLE_STRIP, 0, emitter.instances.size() * 2);
					}

					for (int i = emitter.instances.size() - 1; i >= 0; i--) {
						RibbonInstance &instance = emitter.instances[i];
						if (instance.lifeSpan < 0) {
							emitter.instances.erase(emitter.instances.begin(), &instance + 1);
							break;
						} else instance.lifeSpan -= delta;
					}
				}
			}

			m_ribbonProgram->release();
		}

		if (m_scene->m_head.nprt > 0) {
			m_partProgram->bind();
			m_partProgram->setUniformValue(pMatCam, m_matCam);
			m_partProgram->setUniformValue(pMatView, m_matView);

			for (int i = 0; i < m_scene->m_head.nprt; i++) {
				XParticle &prt = m_scene->m_particles[i];
				ParticleEmitter &emitter = m_particles[i];
				if (prt.boneId >= 0) {
					auto visibleData = m_scene->m_bones[prt.boneId].visibleData;
					bool visible = visibleData == 0 || visibleData[animFrame];
					if (visible) {
						if (!(prt.flag & PARTICLE_SQUIRT)) emitter.timer += delta;
						if (prt.emissionRate > g_EPS) {
							int emitCnt = emitter.timer * prt.emissionRate;
							if ((prt.flag & PARTICLE_SQUIRT) && (animFrame == 0 || (visibleData != 0 && visibleData[animFrame - 1] == 0)))
								emitCnt = prt.emissionRate;
							QMatrix4x4 boneMat = QMatrix4x4(m_boneMatrix[prt.boneId]);
							while (emitCnt--) {
								if (!(prt.flag & PARTICLE_SQUIRT)) emitter.timer -= 1 / prt.emissionRate;
								if (emitter.instances.size() > 100) continue;

								QMatrix4x4 trans;
								trans.setColumn(0, prt.normal);
								trans.setColumn(1, prt.xAxis.normalized());
								trans.setColumn(2, prt.yAxis.normalized());
								trans.setColumn(3, prt.pivot);
								trans(3, 3) = 1.0f;

								ParticleInstance instance;
								if (prt.flag & PARTICLE_PARTICLEINMODELSPACE) {
									QVector4D yDir = trans.row(1);
									trans.setRow(1, QVector4D(-yDir.toVector3D(), yDir.w()));
								}
								else trans = boneMat * trans;

								instance.position = trans.column(3).toVector3D();
								instance.position += trans.column(1).toVector3D() * (rnd() * 2 - 1) * prt.width;
								instance.position += trans.column(2).toVector3D() * (rnd() * 2 - 1) * prt.height;
								instance.velocity = trans.column(0).toVector3D();
								instance.velocity = QQuaternion::fromAxisAndAngle(trans.column(1).toVector3D().normalized(),
									(rnd() * 2 - 1) * prt.coneAngle).rotatedVector(instance.velocity);
								instance.velocity = QQuaternion::fromAxisAndAngle(trans.column(2).toVector3D().normalized(),
									(rnd() * 2 - 1) * prt.coneAngle).rotatedVector(instance.velocity);
								if (prt.flag & PARTICLE_MOVEALONEEMITTERPLANE)
									instance.velocity -= QVector3D::dotProduct(instance.velocity, trans.column(0).toVector3D().normalized())
										* trans.column(0).toVector3D().normalized();
								instance.velocity *= prt.speed * (1 + (rnd() * 2 - 1) * prt.speedVar / 100) * delta;
								instance.life = 0.0f;
								instance.lifeSpan = prt.lifeSpan;
								if (prt.enableLifeRandom)
									instance.lifeSpan *= prt.lifeRandom[0] + rnd() * (prt.lifeRandom[1] - prt.lifeRandom[0]);
								instance.rotation = prt.rotVec;
								if (prt.randRotVec) {
									instance.rotation[0] += prt.rotVecVar[0] + rnd() * (prt.rotVecVar[1] - prt.rotVecVar[0]);
									instance.rotation[1] += prt.rotVecVar[2] + rnd() * (prt.rotVecVar[3] - prt.rotVecVar[2]);
									instance.rotation[2] += prt.rotVecVar[4] + rnd() * (prt.rotVecVar[5] - prt.rotVecVar[4]);
								}
								instance.rotVelocity = prt.rotVel;
								if (prt.randRotVel) {
									instance.rotVelocity[0] += prt.rotVelVar[0] + rnd() * (prt.rotVelVar[1] - prt.rotVelVar[0]);
									instance.rotVelocity[1] += prt.rotVelVar[2] + rnd() * (prt.rotVelVar[3] - prt.rotVelVar[2]);
									instance.rotVelocity[2] += prt.rotVelVar[4] + rnd() * (prt.rotVelVar[5] - prt.rotVelVar[4]);
								}
								instance.size = 1.0f;
								if (prt.enableRandSize)
									instance.size *= prt.randSize[0] + rnd() * (prt.randSize[1] - prt.randSize[0]);
								instance.uvAnim = 0;
								instance.quatRot = QQuaternion::fromEulerAngles(instance.rotation);
								emitter.instances.append(instance);
							}
						}
					}

					if (!emitter.instances.isEmpty()) {
						QVector3D camPos = -m_matCam.column(3).toVector3D();
						if (prt.flag & PARTICLE_PARTICLEINMODELSPACE)
							camPos = m_scene->m_bones[prt.boneId].matrixInv * camPos;
						if (emitter.instances.size() > 1) {
							qSort(emitter.instances.begin(), emitter.instances.end(), [&camPos](const ParticleInstance &A, const ParticleInstance &B) {
								if (A.visible != B.visible)
									return A.visible > B.visible;
								return A.position.distanceToPoint(camPos) > B.position.distanceToPoint(camPos);
								});
						}

						if (prt.blendMode & RENDER_ADD)
							glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
						else glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
						glUniformMatrix4x3fv(pMatBone, 1, false, m_boneMatrix[prt.boneId].data());
						m_partProgram->setUniformValue(pModelSpace, prt.flag & PARTICLE_PARTICLEINMODELSPACE);
						m_partProgram->setUniformValue(pAlignVel, prt.partFlag & PARTICLE_TAIL);
						m_partProgram->setUniformValue(pMidTime, prt.middleTime);
						m_partProgram->setUniformValue(pXyQuad, prt.flag & PARTICLE_XYQUAD);
						QVector3D params[] = {
							QVector3D(prt.startColor[0], prt.startColor[1], prt.startColor[2]) / 255,
							QVector3D(prt.midColor[0], prt.midColor[1], prt.midColor[2]) / 255,
							QVector3D(prt.endColor[0], prt.endColor[1], prt.endColor[2]) / 255,
							QVector3D(prt.alpha[0], prt.alpha[1], prt.alpha[2]) / 255,
							QVector3D(prt.startSize, prt.midSize, prt.endSize),
						};
						glUniform3fv(pColor, 3, (float *) &params);
						glUniform3fv(pAlpha, 1, (float *) &params[3]);
						glUniform3fv(pSize, 1, (float *) &params[4]);
						glUniform2iv(pGrid, 1, &prt.row);
						if (prt.flag & PARTICLE_ENABLEZWRITE)
							glDepthMask(GL_TRUE);
						else glDepthMask(GL_FALSE);
						if (prt.tailMode) {
							m_partProgram->setUniformValue(pTailMode, true);
							glUniform2f(pTailLength, prt.tailStartLength, prt.tailLength);
							m_partProgram->setUniformValue(pTailSpeed, prt.tailSpeed);
						} else m_partProgram->setUniformValue(pTailMode, false);

						m_partDataVbo.bind();
						m_partDataVbo.allocate(&emitter.instances[0], sizeof(ParticleInstance) * emitter.instances.size());
						m_partVao.bind();
						(prt.textureId >= 0 && showTexture ? m_glTextures[prt.textureId] : m_missingTex)->bind(0);
						glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, emitter.instances.size());
					}

					if (emitter.instances.size() > 1) {
						qSort(emitter.instances.begin(), emitter.instances.end(), [](const ParticleInstance &A, const ParticleInstance &B) {
							return A.lifeSpan - A.life > B.lifeSpan - B.life;
							});
					}
					for (int i = 0; i < emitter.instances.size(); i++) {
						ParticleInstance &instance = emitter.instances[i];
						if (instance.lifeSpan < instance.life) {
							emitter.instances.erase(&instance, emitter.instances.end());
							break;
						} else {
							if (!(prt.flag & PARTICLE_LOCKEMITTER)) {
								instance.velocity += QVector3D(-prt.gravityX, -prt.gravityY, -prt.gravity) * delta / 40;
								instance.position += instance.velocity;
							}
							if (prt.useTimeBasedCell) {
								if (prt.matchLife)
									instance.uvAnim = int(instance.life / instance.lifeSpan * (prt.row * prt.col * prt.numLoop));
								else instance.uvAnim = int(instance.life * prt.uvAnimFps) % (prt.row * prt.col);
							} else instance.uvAnim = 0;
							instance.rotation += instance.rotVelocity;
							instance.quatRot = QQuaternion::fromEulerAngles(instance.rotation);
							instance.life += delta;
						}
					}
				}
			}

			m_partProgram->release();
		}

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		if (showBones && m_scene->m_head.nbon > 0) {
			m_boneProgram->bind();
			m_boneProgram->setUniformValue(bMatCam, m_matCam);
			m_boneProgram->setUniformValue(bMatView, m_matView);
			glUniformMatrix3x4fv(bBoneMatrix, m_scene->m_head.nbon, GL_TRUE, m_boneTrans[0].constData());
			m_boneProgram->setUniformValue(bSelectId, m_selection.bone);

			m_boneVao.bind();
			glDrawArrays(GL_POINTS, 0, m_scene->m_head.nbon);

			m_boneProgram->release();
		}

		if (m_selection.ribbon >= 0) {
			XRibbon &rib = m_scene->m_ribbons[m_selection.ribbon];

			if (rib.boneId >= 0) {
				m_lineProgram->bind();
				m_lineProgram->setUniformValue(lMatCam, m_matCam);
				m_lineProgram->setUniformValue(lMatView, m_matView);

				QMatrix4x4 trans(m_boneMatrix[rib.boneId]);
				QVector3D vertexData[] = {
					trans * rib.startPos, { 1.0, 0.0, 1.0 },
					trans * rib.endPos, { 0.0, 1.0, 1.0 },
				};

				m_lineVbo.bind();
				m_lineVbo.allocate(vertexData, sizeof(vertexData));
				m_lineVao.bind();
				glDrawArrays(GL_LINES, 0, 2);

				m_lineProgram->release();
			}
		}

		if (m_selection.particle >= 0) {
			XParticle &prt = m_scene->m_particles[m_selection.particle];

			if (prt.boneId >= 0) {
				m_lineProgram->bind();
				m_lineProgram->setUniformValue(lMatCam, m_matCam);
				m_lineProgram->setUniformValue(lMatView, m_matView);

				QMatrix4x4 trans(m_boneMatrix[prt.boneId]);
				QVector3D pivot = trans * prt.pivot;
				QVector3D normal = trans.mapVector(prt.normal) * max(1.0f, prt.speed * prt.lifeSpan);
				QVector3D xAxis = trans.mapVector(prt.xAxis);
				QVector3D yAxis = trans.mapVector(prt.yAxis);
				if (prt.flag & PARTICLE_PARTICLEINMODELSPACE) {
					normal.setY(-normal.y());
					xAxis.setY(-xAxis.y());
					yAxis.setY(-yAxis.y());
				}
				QVector3D xCone1 = QQuaternion::fromAxisAndAngle(xAxis.normalized(), prt.coneAngle).rotatedVector(normal);
				QVector3D yCone1 = QQuaternion::fromAxisAndAngle(yAxis.normalized(), prt.coneAngle).rotatedVector(normal);
				QVector3D xCone2 = QQuaternion::fromAxisAndAngle(xAxis.normalized(), -prt.coneAngle).rotatedVector(normal);
				QVector3D yCone2 = QQuaternion::fromAxisAndAngle(yAxis.normalized(), -prt.coneAngle).rotatedVector(normal);
				QVector3D vertexData[] = {
					pivot, { 0.0, 0.0, 1.0 },
					pivot + normal, { 0.0, 0.0, 1.0 },
					pivot - xAxis, { 1.0, 0.0, 0.0 },
					pivot + xAxis, { 1.0, 0.0, 0.0 },
					pivot - yAxis, { 0.0, 1.0, 0.0 },
					pivot + yAxis, { 0.0, 1.0, 0.0 },
					pivot, { 1.0, 0.0, 1.0 },
					pivot + xCone1, { 1.0, 0.0, 1.0 },
					pivot, { 1.0, 0.0, 1.0 },
					pivot + yCone1, { 1.0, 0.0, 1.0 },
					pivot, { 1.0, 0.0, 1.0 },
					pivot + xCone2, { 1.0, 0.0, 1.0 },
					pivot, { 1.0, 0.0, 1.0 },
					pivot + yCone2, { 1.0, 0.0, 1.0 },
				};

				m_lineVbo.bind();
				m_lineVbo.allocate(vertexData, sizeof(vertexData));
				m_lineVao.bind();
				glDrawArrays(GL_LINES, 0, (prt.flag & PARTICLE_MOVEALONEEMITTERPLANE) || prt.coneAngle < g_EPS ? 6 : 14);

				m_lineProgram->release();
			}
		}

		if (showAnim)
			m_curFrame++;
		m_timer += delta;
	}
}

void CharWindow::updateSelection(Selection &select) {
	if (m_selection.action != select.action)
		m_curFrame = 0;
	m_selection = select;
}

void CharWindow::updateTextures() {
	for (auto glTex : m_glTextures)
		if (glTex != m_missingTex)
			delete glTex;
	m_glTextures.clear();
	m_qImages.clear();

	if (m_scene) {
		QList<QDir> texPaths;
		QFile texPathConf("texpath.txt");
		if (texPathConf.open(QIODevice::ReadOnly)) {
			while (!texPathConf.atEnd()) {
				QString line = QString::fromLocal8Bit(texPathConf.readLine()).replace("\r\n", "");
				if (QDir(line).exists())
					texPaths.append(QDir(line));
			}
			texPathConf.close();
		}

		for (XTexture &tex : m_scene->m_textures) {
			QString texName = QFileInfo(tex.name).baseName() + ".dds";
			QString texFile = textureDir.filePath(texName);
			for (QDir &dir : texPaths) {
				if (!QFile(texFile).exists())
					texFile = dir.filePath(texName);
				else break;
			}

			if (QFile(texFile).exists()) {
				QImage image;
				if (image.load(texFile)) {
					m_qImages.push_back(image);
					QOpenGLTexture *tex = new QOpenGLTexture(image);
					tex->bind();
					tex->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
					m_glTextures.push_back(tex);
					continue;
				}
			}

			m_qImages.push_back(*m_missingImage);
			m_glTextures.push_back(m_missingTex);
		}
	}
}

const QImage& CharWindow::getImage(int texId) {
	return m_qImages[texId];
}

#ifndef NDEBUG
void CharWindow::messageLogged(const QOpenGLDebugMessage &msg) {
	QString error;

	// Format based on severity
	switch (msg.severity()) {
		case QOpenGLDebugMessage::NotificationSeverity:
			error += "--";
			return;
		case QOpenGLDebugMessage::HighSeverity:
			error += "!!";
			break;
		case QOpenGLDebugMessage::MediumSeverity:
			error += "!~";
			break;
		case QOpenGLDebugMessage::LowSeverity:
			error += "~~";
			break;
		case QOpenGLDebugMessage::InvalidSeverity:
		case QOpenGLDebugMessage::AnySeverity:
			error += "??";
			break;
	}

	error += " (";

	// Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.source()) {
		CASE(APISource);
		CASE(WindowSystemSource);
		CASE(ShaderCompilerSource);
		CASE(ThirdPartySource);
		CASE(ApplicationSource);
		CASE(OtherSource);
		CASE(InvalidSource);
		CASE(AnySource);
	}
#undef CASE

	error += " : ";

	// Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
	switch (msg.type()) {
		CASE(ErrorType);
		CASE(DeprecatedBehaviorType);
		CASE(UndefinedBehaviorType);
		CASE(PortabilityType);
		CASE(PerformanceType);
		CASE(OtherType);
		CASE(MarkerType);
		CASE(GroupPushType);
		CASE(GroupPopType);
		CASE(AnyType);
		CASE(InvalidType);
	}
#undef CASE

	error += ")";
	qDebug() << qPrintable(error) << " " << qPrintable(msg.message());
}
#endif