#pragma once

#include "stdafx.h"

class JumpXEditor;

struct VertexData {
	QVector3D position;
	QVector3D normal;
	QVector2D uv, uv1;
	QVector4D color;
	uint numBone;
	uint boneId[4];
	QVector4D weight;
};

struct Selection {
	int texture = -1;
	int material = -1;
	int geometry = -1;
	int bone = -1;
	int attachment = -1;
	int ribbon = -1;
	int particle = -1;
	int action = -1;
	bool operator==(const Selection &o) {
		return texture == o.texture
			&& material == o.material
			&& geometry == o.geometry
			&& bone == o.bone
			&& attachment == o.attachment
			&& ribbon == o.ribbon
			&& particle == o.particle
			&& action == o.action;
	}
};

struct RibbonInstance {
	QVector3D startPos;
	QVector2D startUv;
	float lifeSpan;
	QVector3D endPos;
	QVector2D endUv;
	float padding;
};

struct RibbonEmitter {
	QVector<RibbonInstance> instances;
};

struct ParticleInstance {
	QVector3D position;
	QVector3D velocity;
	float life;
	float lifeSpan;
	int uvAnim;
	QVector3D rotation;
	QVector3D rotVelocity;
	QQuaternion quatRot;
	float size;
	bool visible;
};

struct ParticleEmitter {
	QVector<ParticleInstance> instances;
	float timer = 0.0f;
};

class CharWindow : public QOpenGLWidget, protected QOpenGLExtraFunctions {
	Q_OBJECT
public:
	explicit CharWindow(JumpXEditor *parent);
	~CharWindow();

	void setupScene(XScene* scene, QDir &texDir);
	void updateSelection(Selection &select);
	void updateTextures();
	void updateMatrix();

	void setShowBones(bool show) { showBones = show; }
	void setShowWire(bool show) { showWire = show; }
	void setShowNormal(bool show) { showNormal = show; }
	void setShowTexture(bool show) { showTexture = show; }
	void setShowAxis(bool show) { showAxis = show; }
	void setShowAnim(bool show) { showAnim = show; }

	void setCurrentFrame(int frame);
	int getCurrentFrame();

	const QImage& getImage(int texId);

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int width, int height) Q_DECL_OVERRIDE;

	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

protected slots:
#ifndef NDEBUG
	void messageLogged(const QOpenGLDebugMessage &msg);
#endif

private:
	QVector<QImage> m_qImages;
	QVector<QOpenGLTexture*> m_glTextures;
	XScene *m_scene = nullptr;
	QDir textureDir;

	QOpenGLTexture *m_missingTex;
	QImage *m_missingImage;
	XMaterial missingMtl;

	int m_curFrame = 0;
	float m_timer = 0.0f;

	float m_distance = 100.0f, m_offset = -60.0f, m_angle[2] = { -60.0f, 30.0f };
	bool moving = false, roting = false, lightRoting = false; float lastX = -1, lastY = -1;
	float m_lightAngle[2] = { -30.0f, 135.0f };
	QMatrix4x4 m_matCam, m_matView;
	QVector3D m_lightDir;
	QVector<QMatrix4x3> m_boneMatrix, m_boneTrans;

	QVector<RibbonEmitter> m_ribbons;
	QVector<ParticleEmitter> m_particles;

	Selection m_selection;
	bool showBones = false, showWire = false, showNormal = false,
		showTexture = true, showAxis = true, showAnim = true;

	QOpenGLShaderProgram *m_geoProgram, *m_normProgram, *m_lineProgram, *m_boneProgram, *m_ribbonProgram, *m_partProgram;
	QOpenGLVertexArrayObject m_lineVao, m_boneVao, m_attachVao, m_ribbonVao, m_partVao;
	QOpenGLBuffer m_lineVbo, m_boneVbo, m_attachVbo, m_ribbonVbo, m_partVertVbo, m_partDataVbo;
	QVector<QOpenGLVertexArrayObject*> m_geoVao, m_normVao;
	QVector<QOpenGLBuffer*> m_geoVbo;
	int gMatCam, gMatView, gLightDir, gAlphaBlend, gAlphaTest, gUvClamp, gSelected, gUvOffset, gBoneMatrix, gMtlColor, gUvAnim, gUvTile, gMatBoneInv, gMatBone, gBillboard, gCenter,
		gUseColor, gUV2, gCartoon, gAmbientColor, gShadowColor, gAmbientInt, gShadowTh, gSpecularSmth, gSpecular;
	int nMatCam, nMatView, nBoneMatrix, nLineColor, nMatBoneInv, nMatBone, nBillboard, nCenter;
	int lMatCam, lMatView;
	int bMatCam, bMatView, bBoneMatrix, bSelectId;
	int rMatCam, rMatView, rRibColor;
	int pMatCam, pMatView, pMatBone, pModelSpace, pAlignVel, pMidTime, pColor, pAlpha, pSize, pGrid, pXyQuad, pTailMode, pTailLength, pTailSpeed;

#ifndef NDEBUG
	QOpenGLDebugLogger *m_debugLogger;
#endif
};