#include "stdafx.h"
#include <QDesktopServices>
#include "JumpXEditor.h"
#include "WidgetAction.h"
#include "WidgetTexture.h"
#include "WidgetMaterial.h"
#include "WidgetParticle.h"
#include "WidgetRibbon.h"
#include "WidgetGeometry.h"
#include "WidgetBone.h"
#include "WidgetAttachment.h"
#include "DialogAbout.h"

template<typename T> T* CutArray(T* orinArr, int orinSize, int start, int end) {
	T* newArr = new T[end - start];
	if (start < 0) fill(newArr, newArr - start, orinArr[0]);
	if (end > orinSize) fill(newArr - start + orinSize, newArr - start + end, orinArr[orinSize - 1]);
	copy(orinArr + max(0, start), orinArr + min(orinSize, end), newArr - min(0, start));
	delete[] orinArr;
	return newArr;
}

JumpXEditor::JumpXEditor(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowIcon(QIcon(":/JumpXEditor/icon.ico"));

	QSurfaceFormat format;
	format.setMajorVersion(3);
	format.setMinorVersion(3);
	format.setProfile(QSurfaceFormat::CoreProfile);
#ifndef NDEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif

#ifdef NDEBUG
	showMaximized();
#endif
	ui.splitter->setStretchFactor(1, 1);
	ui.splitter->setSizes(QList<int>({ 0 }));
	ui.tree_Bone->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	this->setWindowTitle("JumpX Editor X模型编辑工具 " EDITOR_VERSION);

	ui.action_Open->setIcon(QIcon(":/icon/edit-file.png"));
	connect(ui.action_Open, SIGNAL(triggered()), this, SLOT(onActionOpen()));
	ui.action_Combine->setIcon(QIcon(":/icon/combine.png"));
	connect(ui.action_Combine, SIGNAL(triggered()), this, SLOT(onActionCombine()));
	ui.action_Close->setIcon(QIcon(":/icon/close.png"));
	connect(ui.action_Close, SIGNAL(triggered()), this, SLOT(onActionClose()));
	ui.action_Save->setIcon(QIcon(":/icon/save.png"));
	connect(ui.action_Save, SIGNAL(triggered()), this, SLOT(onActionSave()));
	ui.action_SaveAll->setIcon(QIcon(":/icon/save-as.png"));
	connect(ui.action_SaveAll, SIGNAL(triggered()), this, SLOT(onActionSaveAs()));
	ui.action_Quit->setIcon(QIcon(":/icon/quit.png"));
	connect(ui.action_Quit, SIGNAL(triggered()), this, SLOT(onActionQuit()));
	connect(ui.action_ShowBone, SIGNAL(toggled(bool)), this, SLOT(onShowBone(bool)));
	connect(ui.action_ShowWire, SIGNAL(toggled(bool)), this, SLOT(onShowWire(bool)));
	connect(ui.action_ShowNormal, SIGNAL(toggled(bool)), this, SLOT(onShowNormal(bool)));
	connect(ui.action_ShowTexture, SIGNAL(toggled(bool)), this, SLOT(onShowTexture(bool)));
	connect(ui.action_ShowAxis, SIGNAL(toggled(bool)), this, SLOT(onShowAxis(bool)));
	connect(ui.action_ShowAnim, SIGNAL(toggled(bool)), this, SLOT(onShowAnim(bool)));
	ui.action_SaveAnim->setIcon(QIcon(":/icon/export.png"));
	connect(ui.action_SaveAnim, SIGNAL(triggered()), this, SLOT(onSaveAnimTemplate()));
	ui.action_LoadAnim->setIcon(QIcon(":/icon/import.png"));
	connect(ui.action_LoadAnim, SIGNAL(triggered()), this, SLOT(onLoadAnimTemplate()));
	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(onAbout()));
	ui.action_Translate->setIcon(QIcon(":/icon/translate.png"));
	ui.action_Rotate->setIcon(QIcon(":/icon/rotate.png"));
	ui.action_Scale->setIcon(QIcon(":/icon/scale.png"));
	connect(ui.action_Translate, SIGNAL(triggered()), this, SLOT(onTranslate()));
	connect(ui.action_Rotate, SIGNAL(triggered()), this, SLOT(onRotate()));
	connect(ui.action_Scale, SIGNAL(triggered()), this, SLOT(onScale()));
	connect(ui.action_CutAnim, SIGNAL(triggered()), this, SLOT(onCutAnim()));
	connect(ui.action_Version, SIGNAL(triggered()), this, SLOT(onChangeVersion()));

	connect(ui.list_Tex, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Mtl, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Mesh, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.tree_Bone, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Attach, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Ribbon, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Part, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui.list_Anim, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));

	connect(ui.fileDescEdit, SIGNAL(textChanged()), this, SLOT(onDescChanged()));
	connect(ui.slider_Frame, SIGNAL(valueChanged(int)), this, SLOT(onAnimSlider(int)));

	charWindow = new CharWindow(this);
	ui.glLayout->addWidget(charWindow, 1);
	charWindow->setFormat(format);
	m_editArea = new QScrollArea(this);
	m_editArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_editArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_editArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	m_editArea->hide();
	ui.glLayout->addWidget(m_editArea);

	glUpdateTimer = new QTimer();
	glUpdateTimer->setInterval(1000 / 32);
	connect(glUpdateTimer, SIGNAL(timeout()), this, SLOT(onGLTimer()));
	glUpdateTimer->start();

	QWidgetAction *sliderAction = new QWidgetAction(ui.mainToolBar);
	QSlider *slider = new QSlider(Qt::Horizontal, this);
	slider->setMaximumWidth(160);
	slider->setToolTip("播放速度：1.0");
	slider->setRange(-10, 10);
	sliderAction->setDefaultWidget(slider);
	ui.mainToolBar->addAction(sliderAction);
	connect(slider, &QAbstractSlider::sliderMoved, [this, slider](int value) {
		float times = pow(10, value / 15.0f);
		slider->setToolTip(QString("播放速度：%1").arg(times, 0, 'f', 1));
		float fps = 32 * times;
		glUpdateTimer->setInterval(1000 / fps);
		});

	ui.btn_TexNew->setIcon(QIcon(":/icon/create.png"));
	connect(ui.btn_TexNew, &QAbstractButton::clicked, this, &JumpXEditor::onTextureCreate);
	ui.btn_TexDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_TexDel, &QAbstractButton::clicked, this, &JumpXEditor::onTextureDelete);
	ui.btn_TexRefresh->setIcon(QIcon(":/icon/refresh.png"));
	connect(ui.btn_TexRefresh, &QAbstractButton::clicked, this, &JumpXEditor::onTextureRefresh);

	ui.btn_MtlNew->setIcon(QIcon(":/icon/create.png"));
	connect(ui.btn_MtlNew, &QAbstractButton::clicked, this, &JumpXEditor::onMaterialCreate);
	ui.btn_MtlDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_MtlDel, &QAbstractButton::clicked, this, &JumpXEditor::onMaterialDelete);
	ui.btn_MtlColor->setIcon(QIcon(":/icon/color.png"));
	connect(ui.btn_MtlColor, &QAbstractButton::clicked, this, &JumpXEditor::onMaterialColor);

	ui.btn_GeoDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_GeoDel, &QAbstractButton::clicked, this, &JumpXEditor::onGeometryDelete);
	ui.btn_GeoFlip->setIcon(QIcon(":/icon/flip.png"));
	ui.btn_GeoUp->setIcon(QIcon(":/icon/up.png"));
	ui.btn_GeoDown->setIcon(QIcon(":/icon/down.png"));
	connect(ui.btn_GeoFlip, &QAbstractButton::clicked, this, &JumpXEditor::onGeometryFlip);
	connect(ui.btn_GeoUp, &QAbstractButton::clicked, this, &JumpXEditor::onGeometryMoveup);
	connect(ui.btn_GeoDown, &QAbstractButton::clicked, this, &JumpXEditor::onGeometryMovedown);

	ui.btn_BonDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_BonDel, &QAbstractButton::clicked, this, &JumpXEditor::onBoneDelete);
	ui.btn_BonShow->setIcon(QIcon(":/icon/showhide.png"));
	connect(ui.btn_BonShow, &QAbstractButton::clicked, this, &JumpXEditor::onBoneShowHide);

	ui.btn_AttDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_AttDel, &QAbstractButton::clicked, this, &JumpXEditor::onAttachmentDelete);

	ui.btn_RibDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_RibDel, &QAbstractButton::clicked, this, &JumpXEditor::onRibbonDelete);
	ui.btn_RibFlip->setIcon(QIcon(":/icon/flip.png"));
	connect(ui.btn_RibFlip, &QAbstractButton::clicked, this, &JumpXEditor::onRibbonInverse);

	ui.btn_PrtDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_PrtDel, &QAbstractButton::clicked, this, &JumpXEditor::onParticleDelete);
	ui.btn_PrtSave->setIcon(QIcon(":/icon/export.png"));
	ui.btn_PrtLoad->setIcon(QIcon(":/icon/import.png"));
	connect(ui.btn_PrtSave, &QAbstractButton::clicked, this, &JumpXEditor::onParticleSave);
	connect(ui.btn_PrtLoad, &QAbstractButton::clicked, this, &JumpXEditor::onParticleLoad);

	ui.btn_ActNew->setIcon(QIcon(":/icon/create.png"));
	connect(ui.btn_ActNew, &QAbstractButton::clicked, this, &JumpXEditor::onActionCreate);
	ui.btn_ActDel->setIcon(QIcon(":/icon/delete.png"));
	connect(ui.btn_ActDel, &QAbstractButton::clicked, this, &JumpXEditor::onActionDelete);

	QShortcut* tempSc;
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Tex); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onTextureDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Mtl); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onMaterialDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Mesh); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onGeometryDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.tree_Bone); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onBoneDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Attach); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onAttachmentDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Ribbon); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onRibbonDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Part); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onParticleDelete()));
	tempSc = new QShortcut(QKeySequence(Qt::Key_Delete), ui.list_Anim); tempSc->setContext(Qt::WidgetShortcut); connect(tempSc, SIGNAL(activated()), this, SLOT(onActionDelete()));

	if (qApp->arguments().size() > 1)
		tryOpenFile(qApp->arguments().at(1));
}

void JumpXEditor::onSaveAnimTemplate() {
	if (m_scene != nullptr) {
		QString fileName = QFileDialog::getSaveFileName(this, "选择保存位置", "", "Json (*.json)");
		if (!fileName.isEmpty()) {
			QFile file = fileName;
			if (!file.open(QIODevice::WriteOnly)) {
				QMessageBox::critical(this, "错误", "打开文件失败");
				return;
			}

			QJsonArray arr;
			for (XAction &act : m_scene->m_actions) {
				QJsonObject obj;
				obj["name"] = act.name;
				obj["start"] = act.startFrame;
				obj["end"] = act.endFrame;
				obj["hit"] = act.hitPoint;
				arr.append(obj);
			}

			file.write(QJsonDocument(arr).toJson());
			file.close();
		}
	}
}

void JumpXEditor::onLoadAnimTemplate() {
	if (m_scene != nullptr) {
		QString fileName = QFileDialog::getOpenFileName(this, "选择Json文件", "", "Json (*.json)");
		if (!fileName.isEmpty()) {
			QFile file = fileName;
			if (!file.open(QIODevice::ReadOnly)) {
				QMessageBox::critical(this, "错误", "打开文件失败");
				return;
			}

			m_scene->m_head.nact = 0;
			m_scene->m_actions.clear();
			ui.list_Anim->clear();
			QJsonParseError jsonError;
			QJsonArray arr = QJsonDocument::fromJson(file.readAll(), &jsonError).array();
			if (jsonError.error != QJsonParseError::NoError) {
				QMessageBox::critical(this, "错误", "读取Json失败：\n" + jsonError.errorString());
				file.close();
				return;
			}

			for (QJsonValueRef val : arr) {
				QJsonObject obj = val.toObject();
				XAction act;
				act.name = obj["name"].toString();
				act.startFrame = obj["start"].toInt();
				act.endFrame = obj["end"].toInt();
				act.hitPoint = obj["hitPoint"].toInt();
				(new QListWidgetItem(act.name, ui.list_Anim))->setData(Qt::UserRole, m_scene->m_head.nact++);
				m_scene->m_actions.push_back(std::move(act));
			}

			file.close();
		}
	}
}

void JumpXEditor::onAbout() {
	(new DialogAbout(this))->exec();
}

void JumpXEditor::onDescChanged() {
	if (m_scene != nullptr) m_scene->m_desc = ui.fileDescEdit->toPlainText();
}

void JumpXEditor::onAnimSlider(int value) {
	bool blocked = ui.slider_Frame->blockSignals(true);

	charWindow->setCurrentFrame(ui.slider_Frame->sliderPosition());
	ui.slider_Frame->setSliderPosition(charWindow->getCurrentFrame());

	ui.slider_Frame->blockSignals(blocked);
}

void JumpXEditor::onGLTimer() {
	charWindow->update();
	if (m_scene)
		updateInfoString();
}

void JumpXEditor::tryOpenFile(QString path) {
	if (!path.isEmpty()) {
		m_scene = new XScene();
		try {
			m_scene->loadFromFile(QFile(path));
		} catch (QString msg) {
			m_scene->endProgress();
			checkWarnings(m_scene);
			QMessageBox::critical(this, "错误", msg);
			delete m_scene;
			m_scene = nullptr;
		}

		if (m_scene != nullptr) {
			savePath = path;
			setupScene();
			checkWarnings(m_scene);
		}
	}
}

void JumpXEditor::onActionOpen() {
	if (m_scene != nullptr)
		onActionClose();

	if (m_scene == nullptr) {
		QString fileName = QFileDialog::getOpenFileName(this, "选择X文件", "", "JumpX (*.x)");
		tryOpenFile(fileName);
	}
}

void JumpXEditor::onActionCombine() {
	if (m_scene != nullptr) {
		QString fileName = QFileDialog::getOpenFileName(this, "选择X文件", "", "JumpX (*.x)");
		if (!fileName.isEmpty()) {
			XScene *newScene = new XScene();
			try {
				newScene->loadFromFile(QFile(fileName));
			} catch (QString msg) {
				m_scene->endProgress();
				checkWarnings(newScene);
				QMessageBox::critical(this, "错误", msg);
				delete newScene;
				newScene = nullptr;
			}

			if (newScene != nullptr) {
				checkWarnings(newScene);
				m_scene->combineScene(newScene);
				delete newScene;
				setupScene();
				checkWarnings(m_scene);
			}
		}
	}
}

void JumpXEditor::clearEditArea() {
	if (m_editArea->isVisible()) {
		delete m_editArea->takeWidget();
		m_editArea->hide();
	}
}

void JumpXEditor::onSelectionChanged() {
	Selection lastSel = m_selected;
	m_selected.texture = ui.list_Tex->selectedItems().size() == 0 ? -1 : ui.list_Tex->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.material = ui.list_Mtl->selectedItems().size() == 0 ? -1 : ui.list_Mtl->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.geometry = ui.list_Mesh->selectedItems().size() == 0 ? -1 : ui.list_Mesh->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.bone = ui.tree_Bone->selectedItems().size() == 0 ? -1 : ui.tree_Bone->selectedItems()[0]->data(0, Qt::UserRole).toInt();
	m_selected.attachment = ui.list_Attach->selectedItems().size() == 0 ? -1 : ui.list_Attach->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.ribbon = ui.list_Ribbon->selectedItems().size() == 0 ? -1 : ui.list_Ribbon->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.particle = ui.list_Part->selectedItems().size() == 0 ? -1 : ui.list_Part->selectedItems()[0]->data(Qt::UserRole).toInt();
	m_selected.action = ui.list_Anim->selectedItems().size() == 0 ? -1 : ui.list_Anim->selectedItems()[0]->data(Qt::UserRole).toInt();
	charWindow->updateSelection(m_selected);

	if (m_selected == lastSel)
		return;
	clearEditArea();

	QList<QString> textures, materials, bones;
	for (XTexture &tex : m_scene->m_textures)
		textures.push_back(tex.name);
	for (int i = 0; i < m_scene->m_head.nmtl; i++)
		materials.push_back("Mtl_" + QString::number(i));
	for (XBone &bon : m_scene->m_bones)
		bones.push_back(bon.name);

	switch (ui.tabWidget->currentIndex()) {
		case 0:
			if (m_selected.texture >= 0)
				m_editArea->setWidget(new WidgetTexture(m_scene->m_textures[m_selected.texture], ui.list_Tex->currentItem(), charWindow, this));
			break;
		case 1:
			if (m_selected.material >= 0)
				m_editArea->setWidget(new WidgetMaterial(m_scene->m_materials[m_selected.material], ui.list_Mtl->currentItem(), textures, this));
			break;
		case 2:
			if (m_selected.geometry >= 0)
				m_editArea->setWidget(new WidgetGeometry(m_scene->m_geometries[m_selected.geometry], ui.list_Mesh->currentItem(), materials, bones, this));
			break;
		case 3:
			if (m_selected.bone >= 0)
				m_editArea->setWidget(new WidgetBone(m_scene->m_bones[m_selected.bone], ui.tree_Bone->currentItem(), this));
			break;
		case 4:
			if (m_selected.attachment >= 0)
				m_editArea->setWidget(new WidgetAttachment(m_scene->m_attachments[m_selected.attachment], ui.list_Attach->currentItem(), this));
			break;
		case 5:
			if (m_selected.ribbon >= 0)
				m_editArea->setWidget(new WidgetRibbon(m_scene->m_ribbons[m_selected.ribbon], ui.list_Ribbon->currentItem(), textures, bones, this));
			break;
		case 6:
			if (m_selected.particle >= 0)
				m_editArea->setWidget(new WidgetParticle(m_scene->m_particles[m_selected.particle], ui.list_Part->currentItem(), textures, bones, this));
			break;
		case 7:
			if (m_selected.action >= 0)
				m_editArea->setWidget(new WidgetAction(m_scene->m_actions[m_selected.action], ui.list_Anim->currentItem(), this));
			break;
	}

	if (m_editArea->widget()) {
		m_editArea->show();
		m_editArea->updateGeometry();
	}

	if (lastSel.texture != m_selected.texture) {
		updateTexturePreview();
	}
}

void JumpXEditor::updateTexturePreview() {
	if (m_selected.texture != -1) {
		const QImage &img = charWindow->getImage(m_selected.texture);
		QImage scaled = img.height() <= 512 ? img : img.scaledToHeight(512);
		QImage back(scaled.width(), scaled.height(), QImage::Format_RGBA8888);
		back.fill(QColor(255, 255, 255));
		QPainter painter(&back);
		for (int x = 0; x < scaled.width(); x += 16) {
			for (int y = x % 32; y < scaled.height(); y += 32) {
				painter.fillRect(x, y, min(16, scaled.width() - x), min(16, scaled.height() - y), QColor(230, 230, 230));
			}
		}
		painter.drawImage(0, 0, scaled);

		ui.labelBitmap->setPixmap(QPixmap::fromImage(back));
	} else ui.labelBitmap->clear();
}

void JumpXEditor::setupScene() {
	ui.list_Tex->clear();
	ui.list_Mtl->clear();
	ui.list_Mesh->clear();
	ui.tree_Bone->clear();
	ui.list_Attach->clear();
	ui.list_Ribbon->clear();
	ui.list_Part->clear();
	ui.list_Anim->clear();

	for (int i = 0; i < m_scene->m_head.ntex; i++)
		(new QListWidgetItem(m_scene->m_textures[i].name, ui.list_Tex))->setData(Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.nmtl; i++)
		(new QListWidgetItem(QString("Mtl_") + QString::number(i), ui.list_Mtl))->setData(Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.ngeo; i++)
		(new QListWidgetItem(m_scene->m_geometries[i].name, ui.list_Mesh))->setData(Qt::UserRole, i);

	QTreeWidgetItem** treeItems = new QTreeWidgetItem*[m_scene->m_head.nbon];
	ZeroMemory(treeItems, sizeof(QTreeWidgetItem*) * m_scene->m_head.nbon);
	function<QTreeWidgetItem*(int)> getItem = [&](int id) -> QTreeWidgetItem* {
		if (treeItems[id] != nullptr)
			return treeItems[id];
		if (m_scene->m_bones[id].parentId == -1)
			return treeItems[id] = new QTreeWidgetItem(ui.tree_Bone, { m_scene->m_bones[id].name });
		else return treeItems[id] = new QTreeWidgetItem(getItem(m_scene->m_bones[id].parentId), { m_scene->m_bones[id].name });
	};

	for (int i = 0; i < m_scene->m_head.nbon; i++)
		getItem(i)->setData(0, Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.natt; i++)
		(new QListWidgetItem(m_scene->m_attachments[i].name, ui.list_Attach))->setData(Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.nrib; i++)
		(new QListWidgetItem(m_scene->m_ribbons[i].name, ui.list_Ribbon))->setData(Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.nprt; i++)
		(new QListWidgetItem(m_scene->m_particles[i].name, ui.list_Part))->setData(Qt::UserRole, i);
	for (int i = 0; i < m_scene->m_head.nact; i++)
		(new QListWidgetItem(m_scene->m_actions[i].name, ui.list_Anim))->setData(Qt::UserRole, i);

	updateInfoString();
	ui.fileDescEdit->setPlainText(m_scene->m_desc);
	ui.tree_Bone->expandAll();
	charWindow->setupScene(m_scene, QDir(savePath.path()));
}

void JumpXEditor::updateInfoString() {
	ui.fileInfoLabel->setText(QString("X文件版本 %1\n动画总帧数 %2\n\n%3 个贴图\n%4 个材质\n%5 个网格\n%6 个骨骼\n%7 个附件\n%8 个飘带\n%9 个粒子\n%10 个动作")
		.arg(m_scene->m_head.version).arg(m_scene->m_head.numKey).arg(m_scene->m_head.ntex).arg(m_scene->m_head.nmtl).arg(m_scene->m_head.ngeo)
		.arg(m_scene->m_head.nbon).arg(m_scene->m_head.natt).arg(m_scene->m_head.nrib).arg(m_scene->m_head.nprt).arg(m_scene->m_head.nact));

	ui.label_Frame->setText(QString("%1/%2").arg(charWindow->getCurrentFrame()).arg(m_scene->m_head.numKey - 1));
	bool blocked = ui.slider_Frame->blockSignals(true);
	ui.slider_Frame->setMaximum(m_scene->m_head.numKey - 1);
	ui.slider_Frame->setSliderPosition(charWindow->getCurrentFrame());
	ui.slider_Frame->blockSignals(blocked);
}

void JumpXEditor::clearScene() {
	charWindow->setupScene(nullptr, QDir(savePath.path()));
	ui.list_Tex->clear();
	ui.list_Mtl->clear();
	ui.list_Mesh->clear();
	ui.tree_Bone->clear();
	ui.list_Attach->clear();
	ui.list_Ribbon->clear();
	ui.list_Part->clear();
	ui.list_Anim->clear();
	ui.fileInfoLabel->setText("");
	ui.fileDescEdit->setPlainText("");
	ui.label_Frame->setText("0/0");
	ui.slider_Frame->setMaximum(0);
}

bool JumpXEditor::saveScene(QString path) {
	try {
		m_scene->saveToFile(QFile(path));
	} catch (QString msg) {
		m_scene->endProgress();
		QMessageBox::critical(this, "错误", msg);
		return false;
	}

	checkWarnings(m_scene);
	return true;
}

void JumpXEditor::checkWarnings(XScene *scene) {
	if (scene->haveWarning()) {
		QString msg = scene->nextWarning();
		while (scene->haveWarning())
			msg += "\n" + scene->nextWarning();
		QMessageBox::warning(this, "警告", msg);
	}
}

void JumpXEditor::onActionClose() {
	if (m_scene != nullptr) {
		switch (QMessageBox::warning(this, "关闭", "是否保存当前场景？", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {
			case QMessageBox::Yes:
				if (!saveScene(savePath.filePath())) return;
			case QMessageBox::No:
				clearScene();
				delete m_scene;
				m_scene = nullptr;
			default: return;
		}
	}
}

void JumpXEditor::onActionSave() {
	if (m_scene != nullptr) {
		saveScene(savePath.filePath());
	}
}

void JumpXEditor::onActionSaveAs() {
	if (m_scene != nullptr) {
		QString saveFile = QFileDialog::getSaveFileName(this, "选择保存位置", "", "JumpX (*.x)");
		if (!saveFile.isEmpty()) {
			if (saveScene(saveFile))
				savePath = saveFile;
		}
	}
}

void JumpXEditor::onActionQuit() {
	onActionClose();
	if (m_scene == nullptr)
		close();
}

void JumpXEditor::closeEvent(QCloseEvent *event) {
	onActionClose();
	if (m_scene != nullptr)
		event->ignore();
}

void JumpXEditor::onTranslate() {
	if (m_scene != nullptr) {
		QString value = QInputDialog::getText(this, "整体移动", "输入移动向量，以分号分割XYZ");
		QStringList list = value.split(';');
		if (list.size() == 3) {
			QVector3D vec(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
			for (XBone &bone : m_scene->m_bones)
				if (bone.posData != nullptr) {
					for (int i = 0; i < m_scene->m_head.numKey; i++)
						bone.posData[i] += vec;
				} else {
					bone.numPosKey = m_scene->m_head.numKey;
					bone.posData = new QVector3D[bone.numPosKey];
					for (int i = 0; i < bone.numPosKey; i++)
						bone.posData[i] = vec;
				}
		}
	}
}

void JumpXEditor::onRotate() {
	if (m_scene != nullptr) {
		QString value = QInputDialog::getText(this, "整体旋转", "输入旋转角度，以分号分割XYZ");
		QStringList list = value.split(';');
		if (list.size() == 3) {
			float angles[] = { list[0].toFloat(), list[1].toFloat(), list[2].toFloat() };
			QQuaternion rot = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, angles[2]);
			rot = rot * QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angles[1]);
			rot = rot * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, angles[0]);
			for (XBone &bone : m_scene->m_bones) {
				if (bone.posData != nullptr) {
					for (int i = 0; i < m_scene->m_head.numKey; i++)
						bone.posData[i] = rot.rotatedVector(bone.posData[i]);
				}
				if (bone.rotData != nullptr) {
					for (int i = 0; i < m_scene->m_head.numKey; i++)
						bone.rotData[i] = rot * bone.rotData[i];
				} else {
					bone.numRotKey = m_scene->m_head.numKey;
					bone.rotData = new QQuaternion[bone.numRotKey];
					for (int i = 0; i < bone.numRotKey; i++)
						bone.rotData[i] = rot;
				}
			}
		}
	}
}

void JumpXEditor::onScale() {
	if (m_scene != nullptr) {
		float value = QInputDialog::getDouble(this, "整体缩放", "输入放大倍数", 1.0, 0.01, 100, 10);
		if (value != 1.0f) {
			for (XBone &bone : m_scene->m_bones) {
				bone.matInv[12] *= value;
				bone.matInv[13] *= value;
				bone.matInv[14] *= value;
				bone.matrixInv = QMatrix4x4(bone.matInv).transposed();
				bone.matrix = bone.matrixInv.inverted();
				if (bone.posData != nullptr) {
					for (int i = 0; i < m_scene->m_head.numKey; i++)
						bone.posData[i] *= value;
				}
			}
			for (XGeometry &geo : m_scene->m_geometries) {
				for (int i = 0; i < geo.numVertex; i++)
					geo.vertexData[i] *= value;
				geo.center *= value;
			}
			for (XParticle &prt : m_scene->m_particles) {
				prt.pivot *= value;
				prt.speed *= value;
				prt.width *= value;
				prt.height *= value;
				prt.startSize *= value;
				prt.midSize *= value;
				prt.endSize *= value;
				prt.xAxis *= value;
				prt.yAxis *= value;
			}
			for (XRibbon &rib : m_scene->m_ribbons) {
				rib.startPos[0] *= value;
				rib.startPos[1] *= value;
				rib.startPos[2] *= value;
				rib.endPos[0] *= value;
				rib.endPos[1] *= value;
				rib.endPos[2] *= value;
			}
			setupScene();
		}
	}
}

void JumpXEditor::onCutAnim() {
	if (m_scene != nullptr) {
		QString value = QInputDialog::getText(this, "裁剪动画", "输入开始和结束帧，以分号分割");
		QStringList list = value.split(';');
		if (list.size() == 2) {
			int start = list[0].toInt(), end = list[1].toInt();
			if (start >= end) {
				QMessageBox::critical(this, "错误", "帧数不合法");
				return;
			}

			int old_fcnt = m_scene->m_head.numKey;
			m_scene->m_head.numKey = end - start;
			for (XMaterial &mtl : m_scene->m_materials) {
				if (mtl.colorKeys != nullptr) {
					mtl.colorKeys = CutArray<XMaterialAnimDataStruct>(mtl.colorKeys, old_fcnt, start, end);
					mtl.numColorKey = m_scene->m_head.numKey;
				}
			}
			for (XBone &bon : m_scene->m_bones) {
				if (bon.visibleData != nullptr) {
					bon.visibleData = CutArray<uint>(bon.visibleData, old_fcnt, start, end);
					bon.numVisibleKey = m_scene->m_head.numKey;
				}
				if (bon.posData != nullptr) {
					bon.posData = CutArray<QVector3D>(bon.posData, old_fcnt, start, end);
					bon.numPosKey = m_scene->m_head.numKey;
				}
				if (bon.rotData != nullptr) {
					bon.rotData = CutArray<QQuaternion>(bon.rotData, old_fcnt, start, end);
					bon.numRotKey = m_scene->m_head.numKey;
				}
				if (bon.scaleData != nullptr) {
					bon.scaleData = CutArray<QVector3D>(bon.scaleData, old_fcnt, start, end);
					bon.numScaleKey = m_scene->m_head.numKey;
				}
			}
			for (XAction &act : m_scene->m_actions) {
				act.startFrame = min(m_scene->m_head.numKey - 1, max(0, act.startFrame - start));
				act.endFrame = min(m_scene->m_head.numKey - 1, max(0, act.endFrame - start));
			}
		}
	}
}

void JumpXEditor::onChangeVersion() {
	if (m_scene != nullptr) {
		bool ok;
		int version = QInputDialog::getInt(this, "改变版本", "X文件版本", m_scene->m_head.version, 5, 8, 1, &ok);
		if (ok) {
			m_scene->m_head.version = version;
		}
	}
}

void JumpXEditor::onBoneDelete() {
	if (m_selected.bone != -1) {
		QString usingItems;
		for (int i = 0; i < m_scene->m_head.ngeo; i++) {
			if (m_scene->m_geometries[i].ancestorBone == m_selected.bone)
				usingItems += "\n" + ui.list_Mesh->item(i)->text();
			else if (m_scene->m_geometries[i].boneData != nullptr) {
				XGeometry &geo = m_scene->m_geometries[i];
				bool used = false;
				for (int j = 0; j < geo.numVertex; j++) {
					for (int k = 0; k < geo.boneData[j].numBone; k++)
						if (geo.boneData[j].bones[k] == m_selected.bone) {
							used = true;
							break;
						}
					if (used) break;
				}
				if (used) usingItems += "\n" + ui.list_Mesh->item(i)->text();
			}
		}
		for (int i = 0; i < m_scene->m_head.nprt; i++)
			if (m_scene->m_particles[i].boneId == m_selected.bone)
				usingItems += "\n" + ui.list_Part->item(i)->text();
		for (int i = 0; i < m_scene->m_head.nrib; i++)
			if (m_scene->m_ribbons[i].boneId == m_selected.bone)
				usingItems += "\n" + ui.list_Ribbon->item(i)->text();
		if (!usingItems.isEmpty()) {
			QMessageBox::critical(this, "错误", "此骨骼仍在被占用：" + usingItems);
			return;
		}

		for (int i = 0; i < m_scene->m_head.ngeo; i++) {
			XGeometry &geo = m_scene->m_geometries[i];
			if (geo.ancestorBone > m_selected.bone) geo.ancestorBone--;
			if (geo.boneData != nullptr) {
				for (int j = 0; j < geo.numVertex; j++) {
					for (int k = 0; k < geo.boneData[j].numBone; k++)
						if (geo.boneData[j].bones[k] > m_selected.bone)
							geo.boneData[j].bones[k]--;
				}
			}
		}
		for (int i = 0; i < m_scene->m_head.nprt; i++) {
			if (m_scene->m_particles[i].boneId > m_selected.bone)
				m_scene->m_particles[i].boneId--;
		}
		for (int i = 0; i < m_scene->m_head.nrib; i++) {
			if (m_scene->m_ribbons[i].boneId > m_selected.bone)
				m_scene->m_ribbons[i].boneId--;
		}
		for (int i = 0; i < m_scene->m_head.nbon; i++) {
			if (i == m_selected.bone) continue;
			if (m_scene->m_bones[i].parentId == m_selected.bone)
				m_scene->m_bones[i].parentId = m_scene->m_bones[m_selected.bone].parentId;
			if (m_scene->m_bones[i].parentId > m_selected.bone)
				m_scene->m_bones[i].parentId--;
		}

		m_scene->m_head.nbon--;
		m_scene->m_bones.erase(m_scene->m_bones.begin() + m_selected.bone);

		setupScene();
	}
}

void JumpXEditor::onBoneShowHide() {
	if (m_selected.bone != -1) {
		QString value = QInputDialog::getText(this, "设置显隐", "输入开始帧数，结束帧数，显示1/隐藏0，以分号分隔");
		QStringList list = value.split(';');
		if (list.size() == 3) {
			int start = list[0].toInt(), end = list[1].toInt(), show = list[2].toInt();
			start = min(m_scene->m_head.numKey, max(0, start));
			end = min(m_scene->m_head.numKey, max(start, end));
			show = show ? 1 : 0;
			XBone &bone = m_scene->m_bones[m_selected.bone];
			if (bone.visibleData == nullptr) {
				bone.numVisibleKey = m_scene->m_head.numKey;
				bone.visibleData = new uint[bone.numVisibleKey];
				for (int i = 0; i < bone.numVisibleKey; i++)
					bone.visibleData[i] = true;
			}

			for (int i = 0; i < bone.numVisibleKey; i++)
				if (i >= start && i < end) bone.visibleData[i] = show;
		}
	}
}

void JumpXEditor::onGeometryDelete() {
	if (m_selected.geometry != -1) {
		m_scene->m_head.ngeo--;
		for (int i = m_selected.geometry; i < m_scene->m_head.ngeo; i++) {
			QListWidgetItem* item = ui.list_Mesh->item(i + 1);
			item->setData(Qt::UserRole, i);
			m_scene->m_geometries[i + 1].objectId = i;
		}
		m_scene->m_geometries.erase(m_scene->m_geometries.begin() + m_selected.geometry);
		delete ui.list_Mesh->takeItem(m_selected.geometry);
		charWindow->setupScene(m_scene, QDir(savePath.path()));
	}
}

void JumpXEditor::onGeometryFlip() {
	if (m_selected.geometry != -1) {
		XGeometry &geo = m_scene->m_geometries[m_selected.geometry];
		if (geo.normalData != nullptr)
			for (int i = 0; i < geo.numVertex; i++)
				geo.normalData[i] = -geo.normalData[i];
		charWindow->setupScene(m_scene, QDir(savePath.path()));
	}
}

void JumpXEditor::onGeometryMoveup() {
	if (m_selected.geometry != -1 && m_selected.geometry > 0) {
		swap(m_scene->m_geometries[m_selected.geometry - 1], m_scene->m_geometries[m_selected.geometry]);
		m_scene->m_geometries[m_selected.geometry - 1].objectId = m_selected.geometry - 1;
		m_scene->m_geometries[m_selected.geometry].objectId = m_selected.geometry;
		setupScene();
	}
}

void JumpXEditor::onGeometryMovedown() {
	if (m_selected.geometry != -1 && m_selected.geometry < m_scene->m_head.ngeo - 1) {
		swap(m_scene->m_geometries[m_selected.geometry], m_scene->m_geometries[m_selected.geometry + 1]);
		m_scene->m_geometries[m_selected.geometry].objectId = m_selected.geometry;
		m_scene->m_geometries[m_selected.geometry + 1].objectId = m_selected.geometry + 1;
		setupScene();
	}
}

void JumpXEditor::onParticleDelete() {
	if (m_selected.particle != -1) {
		m_scene->m_head.nprt--;
		m_scene->m_particles.erase(m_scene->m_particles.begin() + m_selected.particle);
		for (int i = m_selected.particle; i < m_scene->m_head.nprt; i++) {
			QListWidgetItem* item = ui.list_Part->item(i + 1);
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Part->takeItem(m_selected.particle);
		charWindow->setupScene(m_scene, QDir(savePath.path()));
	}
}

void JumpXEditor::onParticleSave() {
	if (m_selected.particle != -1) {
		QString fileName = QFileDialog::getSaveFileName(this, "选择保存位置", "", "Json (*.json)");
		if (!fileName.isEmpty()) {
			QFile file = fileName;
			if (!file.open(QIODevice::WriteOnly)) {
				QMessageBox::critical(this, "错误", "打开文件失败");
				return;
			}

			QJsonObject obj;
			XParticle &prt = m_scene->m_particles[m_selected.particle];
			obj["modelSpace"] = bool(prt.flag & PARTICLE_PARTICLEINMODELSPACE);
			obj["xyQuad"] = bool(prt.flag & PARTICLE_XYQUAD);
			obj["lockEmitter"] = bool(prt.flag & PARTICLE_LOCKEMITTER);
			obj["emitterPlane"] = bool(prt.flag & PARTICLE_MOVEALONEEMITTERPLANE);
			obj["squirt"] = bool(prt.flag & PARTICLE_SQUIRT);
			obj["zWrite"] = bool(prt.flag & PARTICLE_ENABLEZWRITE);
			obj["faceSpeed"] = bool(prt.partFlag & 0x10000);
			obj["blend"] = bool(prt.blendMode & 0x40000);
			obj["randRotVec"] = bool(prt.randRotVec);
			obj["randRotVel"] = bool(prt.randRotVel);
			obj["timeBasedCell"] = bool(prt.useTimeBasedCell);
			obj["matchLife"] = bool(prt.matchLife);
			obj["enableLifeRandom"] = bool(prt.enableLifeRandom);
			obj["enableRandSize"] = bool(prt.enableRandSize);
			obj["enableBump"] = bool(prt.partEx.flag);
			obj["tailMode"] = bool(prt.tailMode);

			obj["speed"] = QJsonArray{ prt.speed, prt.speedVar };
			obj["coneAngle"] = prt.coneAngle;
			obj["lifeSpan"] = prt.lifeSpan;
			obj["emissionRate"] = prt.emissionRate;
			obj["plane"] = QJsonArray{ prt.width, prt.height };
			obj["tile"] = QJsonArray{ prt.row, prt.col };
			obj["middleTime"] = prt.middleTime;
			obj["size"] = QJsonArray{ prt.startSize, prt.midSize, prt.endSize };
			obj["rotVec"] = QJsonArray{ prt.rotVec[0], prt.rotVec[1], prt.rotVec[2] };
			obj["rotVel"] = QJsonArray{ prt.rotVel[0], prt.rotVel[1], prt.rotVel[2] };
			obj["rotVecVar"] = QJsonArray{ prt.rotVecVar[0], prt.rotVecVar[1], prt.rotVecVar[2], prt.rotVecVar[3], prt.rotVecVar[4], prt.rotVecVar[5] };
			obj["rotVelVar"] = QJsonArray{ prt.rotVelVar[0], prt.rotVelVar[1], prt.rotVelVar[2], prt.rotVelVar[3], prt.rotVelVar[4], prt.rotVelVar[5] };
			obj["gravity"] = QJsonArray{ prt.gravityX, prt.gravityY, prt.gravity };
			obj["lifeRandom"] = QJsonArray{ prt.lifeRandom[0], prt.lifeRandom[1] };
			obj["uvAnimFps"] = (int) prt.uvAnimFps;
			obj["numLoop"] = prt.numLoop;
			obj["randSize"] = QJsonArray{ prt.randSize[0], prt.randSize[1] };
			obj["bumpAmount"] = prt.partEx.bumpAmount;
			obj["script"] = prt.script;
			obj["tailLength"] = QJsonArray{ prt.tailStartLength, prt.tailLength };
			obj["tailSpeed"] = prt.tailSpeed;
			obj["color"] = QJsonArray{
				QJsonArray{ prt.startColor[0], prt.startColor[1], prt.startColor[2], prt.alpha[0] },
				QJsonArray{ prt.midColor[0], prt.midColor[1], prt.midColor[2], prt.alpha[1] },
				QJsonArray{ prt.endColor[0], prt.endColor[1], prt.endColor[2], prt.alpha[2] },
			};

			file.write(QJsonDocument(obj).toJson());
			file.close();
		}
	}
}

void JumpXEditor::onParticleLoad() {
	if (m_selected.particle != -1) {
		QString fileName = QFileDialog::getOpenFileName(this, "选择模板文件", "", "Json (*.json)");
		if (!fileName.isEmpty()) {
			QFile file = fileName;
			if (!file.open(QIODevice::ReadOnly)) {
				QMessageBox::critical(this, "错误", "打开文件失败");
				return;
			}

			QJsonParseError jsonError;
			QJsonObject obj = QJsonDocument::fromJson(file.readAll(), &jsonError).object();
			if (jsonError.error != QJsonParseError::NoError) {
				QMessageBox::critical(this, "错误", "读取Json失败：\n" + jsonError.errorString());
				file.close();
				return;
			}

			XParticle &prt = m_scene->m_particles[m_selected.particle];
			if (obj["modelSpace"].toBool() != bool(prt.flag & PARTICLE_PARTICLEINMODELSPACE)) {
				prt.normal[1] = -prt.normal[1]; prt.xAxis[1] = -prt.xAxis[1]; prt.yAxis[1] = -prt.yAxis[1]; }
			setFlag(prt.flag, PARTICLE_PARTICLEINMODELSPACE, obj["modelSpace"].toBool());
			setFlag(prt.flag, PARTICLE_XYQUAD, obj["xyQuad"].toBool());
			setFlag(prt.flag, PARTICLE_LOCKEMITTER, obj["lockEmitter"].toBool());
			setFlag(prt.flag, PARTICLE_MOVEALONEEMITTERPLANE, obj["emitterPlane"].toBool());
			setFlag(prt.flag, PARTICLE_SQUIRT, obj["squirt"].toBool());
			setFlag(prt.flag, PARTICLE_ENABLEZWRITE, obj["zWrite"].toBool());
			prt.partFlag = obj["faceSpeed"].toBool() ? 0x10000 : 0x8000;
			prt.blendMode = obj["blend"].toBool() ? 0x40000 : 0x20000;
			prt.randRotVec = obj["randRotVec"].toBool();
			prt.randRotVel = obj["randRotVel"].toBool();
			prt.useTimeBasedCell = obj["timeBasedCell"].toBool();
			prt.matchLife = obj["matchLife"].toBool();
			prt.enableLifeRandom = obj["enableLifeRandom"].toBool();
			prt.enableRandSize = obj["enableRandSize"].toBool();
			prt.partEx.flag = obj["enableBump"].toBool();
			prt.tailMode = obj["tailMode"].toBool();

			prt.speed = obj["speed"].toArray()[0].toDouble();
			prt.speedVar = obj["speed"].toArray()[1].toDouble();
			prt.coneAngle = obj["coneAngle"].toDouble();
			prt.lifeSpan = obj["lifeSpan"].toDouble();
			prt.emissionRate = obj["emissionRate"].toDouble();
			prt.width = obj["plane"].toArray()[0].toDouble();
			prt.xAxis = prt.xAxis.normalized() * prt.width;
			prt.height = obj["plane"].toArray()[1].toDouble();
			prt.yAxis = prt.yAxis.normalized() * prt.height;
			prt.row = obj["tile"].toArray()[0].toInt();
			prt.col = obj["tile"].toArray()[1].toInt();
			prt.middleTime = obj["middleTime"].toDouble();
			prt.startSize = obj["size"].toArray()[0].toDouble();
			prt.midSize = obj["size"].toArray()[1].toDouble();
			prt.endSize = obj["size"].toArray()[2].toDouble();
			prt.rotVec[0] = obj["rotVec"].toArray()[0].toDouble();
			prt.rotVec[1] = obj["rotVec"].toArray()[1].toDouble();
			prt.rotVec[2] = obj["rotVec"].toArray()[2].toDouble();
			prt.rotVel[0] = obj["rotVel"].toArray()[0].toDouble();
			prt.rotVel[1] = obj["rotVel"].toArray()[1].toDouble();
			prt.rotVel[2] = obj["rotVel"].toArray()[2].toDouble();
			prt.rotVecVar[0] = obj["rotVecVar"].toArray()[0].toDouble();
			prt.rotVecVar[1] = obj["rotVecVar"].toArray()[1].toDouble();
			prt.rotVecVar[2] = obj["rotVecVar"].toArray()[2].toDouble();
			prt.rotVecVar[3] = obj["rotVecVar"].toArray()[3].toDouble();
			prt.rotVecVar[4] = obj["rotVecVar"].toArray()[4].toDouble();
			prt.rotVecVar[5] = obj["rotVecVar"].toArray()[5].toDouble();
			prt.rotVelVar[0] = obj["rotVelVar"].toArray()[0].toDouble();
			prt.rotVelVar[1] = obj["rotVelVar"].toArray()[1].toDouble();
			prt.rotVelVar[2] = obj["rotVelVar"].toArray()[2].toDouble();
			prt.rotVelVar[3] = obj["rotVelVar"].toArray()[3].toDouble();
			prt.rotVelVar[4] = obj["rotVelVar"].toArray()[4].toDouble();
			prt.rotVelVar[5] = obj["rotVelVar"].toArray()[5].toDouble();
			prt.gravityX = obj["gravity"].toArray()[0].toDouble();
			prt.gravityY = obj["gravity"].toArray()[1].toDouble();
			prt.gravity = obj["gravity"].toArray()[2].toDouble();
			prt.lifeRandom[0] = obj["lifeRandom"].toArray()[0].toDouble();
			prt.lifeRandom[1] = obj["lifeRandom"].toArray()[1].toDouble();
			prt.uvAnimFps = obj["uvAnimFps"].toInt();
			prt.numLoop = obj["numLoop"].toInt();
			prt.randSize[0] = obj["randSize"].toArray()[0].toDouble();
			prt.randSize[1] = obj["randSize"].toArray()[1].toDouble();
			prt.partEx.bumpAmount = obj["bumpAmount"].toDouble();
			prt.script = obj["script"].toString();
			prt.tailStartLength = obj["tailLength"].toArray()[0].toDouble();
			prt.tailLength = obj["tailLength"].toArray()[1].toDouble();
			prt.tailSpeed = obj["tailSpeed"].toDouble();
			prt.startColor[0] = obj["color"].toArray()[0].toArray()[0].toDouble();
			prt.startColor[1] = obj["color"].toArray()[0].toArray()[1].toDouble();
			prt.startColor[2] = obj["color"].toArray()[0].toArray()[2].toDouble();
			prt.alpha[0] = obj["color"].toArray()[0].toArray()[3].toDouble();
			prt.midColor[0] = obj["color"].toArray()[1].toArray()[0].toDouble();
			prt.midColor[1] = obj["color"].toArray()[1].toArray()[1].toDouble();
			prt.midColor[2] = obj["color"].toArray()[1].toArray()[2].toDouble();
			prt.alpha[1] = obj["color"].toArray()[1].toArray()[3].toDouble();
			prt.endColor[0] = obj["color"].toArray()[2].toArray()[0].toDouble();
			prt.endColor[1] = obj["color"].toArray()[2].toArray()[1].toDouble();
			prt.endColor[2] = obj["color"].toArray()[2].toArray()[2].toDouble();
			prt.alpha[2] = obj["color"].toArray()[2].toArray()[3].toDouble();

			file.close();
			charWindow->setupScene(m_scene, QDir(savePath.path()));
			clearEditArea();
		}
	}
}

void JumpXEditor::onRibbonDelete() {
	if (m_selected.ribbon != -1) {
		m_scene->m_head.nrib--;
		m_scene->m_ribbons.erase(m_scene->m_ribbons.begin() + m_selected.ribbon);
		for (int i = m_selected.ribbon; i < m_scene->m_head.nrib; i++) {
			QListWidgetItem* item = ui.list_Ribbon->item(i + 1);
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Ribbon->takeItem(m_selected.ribbon);
		charWindow->setupScene(m_scene, QDir(savePath.path()));
	}
}

void JumpXEditor::onRibbonInverse() {
	if (m_selected.ribbon != -1) {
		XRibbon &ribbon = m_scene->m_ribbons[m_selected.ribbon];
		swap(ribbon.startPos, ribbon.endPos);
		charWindow->setupScene(m_scene, QDir(savePath.path()));
	}
}

void JumpXEditor::onActionDelete() {
	if (m_selected.action != -1) {
		m_scene->m_head.nact--;
		m_scene->m_actions.erase(m_scene->m_actions.begin() + m_selected.action);
		for (int i = m_selected.action; i < m_scene->m_head.nact; i++) {
			QListWidgetItem* item = ui.list_Anim->item(i + 1);
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Anim->takeItem(m_selected.action);
	}
}

void JumpXEditor::onActionCreate() {
	m_scene->m_actions.emplace_back();
	QListWidgetItem *item = new QListWidgetItem("", ui.list_Anim);
	item->setData(Qt::UserRole, m_scene->m_head.nact++);
	ui.list_Anim->setCurrentItem(item);
}

void JumpXEditor::onTextureDelete() {
	if (m_selected.texture != -1) {
		QString usingItems;
		for (int i = 0; i < m_scene->m_head.nmtl; i++)
			if (m_scene->m_materials[i].textureId == m_selected.texture ||
				m_scene->m_materials[i].exData.bumpTexId == m_selected.texture ||
				m_scene->m_materials[i].exData.specTexId == m_selected.texture ||
				m_scene->m_materials[i].exData.lightTexId == m_selected.texture ||
				m_scene->m_materials[i].cartoonData.specTextureID == m_selected.texture ||
				m_scene->m_materials[i].cartoonData.shadowTextureID == m_selected.texture ||
				m_scene->m_materials[i].dissolveData.dissolveTextureID == m_selected.texture)
				usingItems += "\n" + ui.list_Mtl->item(i)->text();
		for (int i = 0; i < m_scene->m_head.nprt; i++)
			if (m_scene->m_particles[i].textureId == m_selected.texture ||
				m_scene->m_particles[i].partEx.bumpTexId == m_selected.texture)
				usingItems += "\n" + ui.list_Part->item(i)->text();
		for (int i = 0; i < m_scene->m_head.nrib; i++)
			if (m_scene->m_ribbons[i].textureId == m_selected.texture)
				usingItems += "\n" + ui.list_Ribbon->item(i)->text();
		if (!usingItems.isEmpty()) {
			QMessageBox::critical(this, "错误", "此贴图仍在被占用：" + usingItems);
			return;
		}

		for (int i = 0; i < m_scene->m_head.nmtl; i++) {
			if (m_scene->m_materials[i].textureId > m_selected.texture)
				m_scene->m_materials[i].textureId--;
			if (m_scene->m_materials[i].exData.bumpTexId > m_selected.texture)
				m_scene->m_materials[i].exData.bumpTexId--;
			if (m_scene->m_materials[i].exData.specTexId > m_selected.texture)
				m_scene->m_materials[i].exData.specTexId--;
			if (m_scene->m_materials[i].exData.lightTexId > m_selected.texture)
				m_scene->m_materials[i].exData.lightTexId--;
			if (m_scene->m_materials[i].cartoonData.specTextureID > m_selected.texture)
				m_scene->m_materials[i].cartoonData.specTextureID--;
			if (m_scene->m_materials[i].cartoonData.shadowTextureID > m_selected.texture)
				m_scene->m_materials[i].cartoonData.shadowTextureID--;
			if (m_scene->m_materials[i].dissolveData.dissolveTextureID > m_selected.texture)
				m_scene->m_materials[i].dissolveData.dissolveTextureID--;
		}
		for (int i = 0; i < m_scene->m_head.nprt; i++) {
			if (m_scene->m_particles[i].textureId > m_selected.texture)
				m_scene->m_particles[i].textureId--;
			if (m_scene->m_particles[i].partEx.bumpTexId > m_selected.texture)
				m_scene->m_particles[i].partEx.bumpTexId--;
		}
		for (int i = 0; i < m_scene->m_head.nrib; i++)
			if (m_scene->m_ribbons[i].textureId > m_selected.texture)
				m_scene->m_ribbons[i].textureId--;

		m_scene->m_head.ntex--;
		m_scene->m_textures.erase(m_scene->m_textures.begin() + m_selected.texture);
		for (int i = m_selected.texture; i < m_scene->m_head.ntex; i++) {
			QListWidgetItem* item = ui.list_Tex->item(i + 1);
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Tex->takeItem(m_selected.texture);

		charWindow->updateTextures();
	}
}

void JumpXEditor::onTextureCreate() {
	m_scene->m_textures.emplace_back();
	charWindow->updateTextures();
	QListWidgetItem *item = new QListWidgetItem("", ui.list_Tex);
	item->setData(Qt::UserRole, m_scene->m_head.ntex++);
	ui.list_Tex->setCurrentItem(item);
}

void JumpXEditor::onTextureRefresh() {
	if (m_scene != nullptr)
		charWindow->updateTextures();
}

void JumpXEditor::onMaterialDelete() {
	if (m_selected.material != -1) {
		QString usingItems;
		for (XGeometry &geo : m_scene->m_geometries)
			if (geo.materialId == m_selected.material)
				usingItems += "\n" + geo.name;
		if (!usingItems.isEmpty()) {
			QMessageBox::critical(this, "错误", "此材质仍在被占用：" + usingItems);
			return;
		}

		for (XGeometry &geo : m_scene->m_geometries)
			if (geo.materialId > m_selected.material)
				geo.materialId--;

		m_scene->m_head.nmtl--;
		m_scene->m_materials.erase(m_scene->m_materials.begin() + m_selected.material);
		for (int i = m_selected.material; i < m_scene->m_head.nmtl; i++) {
			QListWidgetItem* item = ui.list_Mtl->item(i + 1);
			item->setText("Mtl_" + QString::number(i));
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Mtl->takeItem(m_selected.material);
	}
}

void JumpXEditor::onMaterialCreate() {
	m_scene->m_materials.emplace_back();
	int id = m_scene->m_head.nmtl++;
	QListWidgetItem *item = new QListWidgetItem("Mtl_" + QString::number(id), ui.list_Mtl);
	item->setData(Qt::UserRole, id);
	ui.list_Mtl->setCurrentItem(item);
}

void JumpXEditor::onMaterialColor() {
	if (m_selected.material != -1) {
		bool editAlpha = ui.check_Alpha->isChecked();
		QColor newColor = QColorDialog::getColor(Qt::white, this, "设置颜色",
			editAlpha ? QColorDialog::ShowAlphaChannel : QColorDialog::ColorDialogOptions());
		if (newColor.isValid()) {
			XMaterial &mtl = m_scene->m_materials[m_selected.material];
			if (mtl.colorKeys == nullptr) {
				mtl.numColorKey = m_scene->m_head.numKey;
				mtl.colorKeys = new XMaterialAnimDataStruct[mtl.numColorKey];
			}
			for (int j = 0; j < mtl.numColorKey; j++) {
				mtl.colorKeys[j].color[0] = newColor.red();
				mtl.colorKeys[j].color[1] = newColor.green();
				mtl.colorKeys[j].color[2] = newColor.blue();
				if (editAlpha)
					mtl.colorKeys[j].color[3] = newColor.alpha();
			}
			QWidget *widget = m_editArea->widget();
			if (widget->inherits("WidgetMaterial")) {
				WidgetMaterial *wMtl = (WidgetMaterial *) widget;
				if (&wMtl->mtl() == &mtl)
					wMtl->colorBar()->setColorData(mtl.colorKeys, mtl.numColorKey);
			}
		}
	}
}

void JumpXEditor::onAttachmentDelete() {
	if (m_selected.attachment != -1) {
		m_scene->m_head.natt--;
		m_scene->m_attachments.erase(m_scene->m_attachments.begin() + m_selected.attachment);
		for (int i = m_selected.attachment; i < m_scene->m_head.natt; i++) {
			QListWidgetItem *item = ui.list_Attach->item(i + 1);
			item->setData(Qt::UserRole, i);
		}
		delete ui.list_Attach->takeItem(m_selected.attachment);
	}
}
