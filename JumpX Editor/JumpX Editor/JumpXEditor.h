#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_JumpXEditor.h"
#include "stdafx.h"
#include "CharWindow.h"

class JumpXEditor : public QMainWindow
{
	Q_OBJECT

public:
	JumpXEditor(QWidget *parent = Q_NULLPTR);

	void updateTexturePreview();

private slots:
	void onActionOpen();
	void onActionCombine();
	void onActionClose();
	void onActionSave();
	void onActionSaveAs();
	void onActionQuit();

	void onGLTimer();
	void onSelectionChanged();

	void onActionDelete();
	void onActionCreate();

	void onTextureDelete();
	void onTextureCreate();
	void onTextureRefresh();

	void onMaterialDelete();
	void onMaterialCreate();
	void onMaterialColor();

	void onParticleDelete();
	void onParticleSave();
	void onParticleLoad();

	void onRibbonDelete();
	void onRibbonInverse();

	void onGeometryDelete();
	void onGeometryFlip();
	void onGeometryMoveup();
	void onGeometryMovedown();

	void onBoneDelete();
	void onBoneShowHide();

	void onAttachmentDelete();

	void onShowBone(bool show) { charWindow->setShowBones(show); }
	void onShowWire(bool show) { charWindow->setShowWire(show); }
	void onShowNormal(bool show) { charWindow->setShowNormal(show); }
	void onShowTexture(bool show) { charWindow->setShowTexture(show); }
	void onShowAxis(bool show) { charWindow->setShowAxis(show); }
	void onShowAnim(bool show) { charWindow->setShowAnim(show); }

	void onSaveAnimTemplate();
	void onLoadAnimTemplate();

	void onTranslate();
	void onRotate();
	void onScale();

	void onCutAnim();
	void onChangeVersion();

	void onAbout();

	void onDescChanged();
	void onAnimSlider(int value);

protected:
	void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
	void setupScene();
	void clearScene();

	bool saveScene(QString path);
	void checkWarnings(XScene *scene);

	void updateInfoString();
	void clearEditArea();
	void tryOpenFile(QString path);

	Ui::JumpXEditorClass ui;
	CharWindow* charWindow;
	QTimer* glUpdateTimer;
	QScrollArea *m_editArea;

	XScene* m_scene = nullptr;
	QFileInfo savePath;
	Selection m_selected;
};
