#include "stdafx.h"
#include "WidgetParticle.h"

WidgetParticle::WidgetParticle(XParticle &prt, QListWidgetItem *item, QList<QString> &textures, QList<QString> &bones, QWidget *parent) : m_prt(prt), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	m_intVal.setBottom(1);

	ui.modelSpace->setChecked(m_prt.flag & PARTICLE_PARTICLEINMODELSPACE);
	connect(ui.modelSpace, &QCheckBox::stateChanged, [this](int state) {
		if (bool(state) != bool(m_prt.flag & PARTICLE_PARTICLEINMODELSPACE)) {
			m_prt.normal[1] = -m_prt.normal[1]; m_prt.xAxis[1] = -m_prt.xAxis[1]; m_prt.yAxis[1] = -m_prt.yAxis[1]; }
		setFlag(m_prt.flag, PARTICLE_PARTICLEINMODELSPACE, state); });
	ui.xyQuad->setChecked(m_prt.flag & PARTICLE_XYQUAD);
	connect(ui.xyQuad, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.flag, PARTICLE_XYQUAD, state); });
	ui.lockEmitter->setChecked(m_prt.flag & PARTICLE_LOCKEMITTER);
	connect(ui.lockEmitter, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.flag, PARTICLE_LOCKEMITTER, state); });
	ui.emitterPlane->setChecked(m_prt.flag & PARTICLE_MOVEALONEEMITTERPLANE);
	connect(ui.emitterPlane, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.flag, PARTICLE_MOVEALONEEMITTERPLANE, state); });
	ui.faceSpeed->setChecked(m_prt.partFlag & 0x10000);
	connect(ui.faceSpeed, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.partFlag, 0x10000, state); setFlag(m_prt.partFlag, 0x8000, !state); });
	ui.randRotVec->setChecked(m_prt.randRotVec);
	connect(ui.randRotVec, &QCheckBox::stateChanged, [this](int state) { m_prt.randRotVec = state;
		ui.rotVecVar11->setEnabled(state); ui.rotVecVar21->setEnabled(state);
		ui.rotVecVar12->setEnabled(state); ui.rotVecVar22->setEnabled(state);
		ui.rotVecVar31->setEnabled(state);
		ui.rotVecVar32->setEnabled(state); });
	if (!m_prt.randRotVec) {
		ui.rotVecVar11->setEnabled(false); ui.rotVecVar21->setEnabled(false); ui.rotVecVar31->setEnabled(false);
		ui.rotVecVar12->setEnabled(false); ui.rotVecVar22->setEnabled(false); ui.rotVecVar32->setEnabled(false); }
	ui.randRotVel->setChecked(m_prt.randRotVel);
	connect(ui.randRotVel, &QCheckBox::stateChanged, [this](int state) { m_prt.randRotVel = state;
		ui.rotVelVar11->setEnabled(state); ui.rotVelVar21->setEnabled(state);
		ui.rotVelVar12->setEnabled(state); ui.rotVelVar22->setEnabled(state);
		ui.rotVelVar31->setEnabled(state);
		ui.rotVelVar32->setEnabled(state); });
	if (!m_prt.randRotVel) {
		ui.rotVelVar11->setEnabled(false); ui.rotVelVar21->setEnabled(false); ui.rotVelVar31->setEnabled(false);
		ui.rotVelVar12->setEnabled(false); ui.rotVelVar22->setEnabled(false); ui.rotVelVar32->setEnabled(false); }
	ui.timeBasedCell->setChecked(m_prt.useTimeBasedCell);
	connect(ui.timeBasedCell, &QCheckBox::stateChanged, [this](int state) { m_prt.useTimeBasedCell = state; });
	ui.matchLife->setChecked(m_prt.matchLife);
	connect(ui.matchLife, &QCheckBox::stateChanged, [this](int state) { m_prt.matchLife = state; });
	ui.squirt->setChecked(m_prt.flag & PARTICLE_SQUIRT);
	connect(ui.squirt, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.flag, PARTICLE_SQUIRT, state); });
	ui.zWrite->setChecked(m_prt.flag & PARTICLE_ENABLEZWRITE);
	connect(ui.zWrite, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.flag, PARTICLE_ENABLEZWRITE, state); });
	ui.blend->setChecked(m_prt.blendMode & 0x40000);
	connect(ui.blend, &QCheckBox::stateChanged, [this](int state) { setFlag(m_prt.blendMode, 0x40000, state); setFlag(m_prt.blendMode, 0x20000, !state); });
	ui.enableLifeRandom->setChecked(m_prt.enableLifeRandom);
	connect(ui.enableLifeRandom, &QCheckBox::stateChanged, [this](int state) { m_prt.enableLifeRandom = state;
		ui.lifeRandom1->setEnabled(state); ui.lifeRandom2->setEnabled(state); });
	if (!m_prt.enableLifeRandom) {
		ui.lifeRandom1->setEnabled(false); ui.lifeRandom2->setEnabled(false); }
	ui.enableRandSize->setChecked(m_prt.enableRandSize);
	connect(ui.enableRandSize, &QCheckBox::stateChanged, [this](int state) { m_prt.enableRandSize = state;
		ui.randSize1->setEnabled(state); ui.randSize2->setEnabled(state); });
	if (!m_prt.enableRandSize) {
		ui.randSize1->setEnabled(false); ui.randSize2->setEnabled(false); }
	ui.enableBump->setChecked(m_prt.partEx.flag);
	connect(ui.enableBump, &QCheckBox::stateChanged, [this](int state) { m_prt.partEx.flag = state;
		ui.bumpTexId->setEnabled(state); ui.bumpAmount->setEnabled(state); });
	if (!m_prt.partEx.flag) {
		ui.bumpTexId->setEnabled(false); ui.bumpAmount->setEnabled(false); }
	ui.tailMode->setChecked(m_prt.tailMode);
	connect(ui.tailMode, &QCheckBox::stateChanged, [this](int state) { m_prt.tailMode = state;
		ui.tailStartLength->setEnabled(state); ui.tailLength->setEnabled(state); ui.tailSpeed->setEnabled(state); });
	if (!m_prt.tailMode) {
		ui.tailStartLength->setEnabled(false); ui.tailLength->setEnabled(false); ui.tailSpeed->setEnabled(false); }

	ui.boneId->addItems(bones); clamp(m_prt.boneId, bones.size()); ui.boneId->setCurrentIndex(m_prt.boneId);
	connect(ui.boneId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_prt.boneId = index; });
	ui.textureId->addItems(textures); clamp(m_prt.textureId, textures.size()); ui.textureId->setCurrentIndex(m_prt.textureId);
	connect(ui.textureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_prt.textureId = index; });
	ui.bumpTexId->addItems(textures); clamp(m_prt.partEx.bumpTexId, textures.size()); ui.bumpTexId->setCurrentIndex(m_prt.partEx.bumpTexId);
	connect(ui.bumpTexId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_prt.partEx.bumpTexId = index; });

	ui.name->setText(m_prt.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_prt.name = ui.name->text(); m_item->setText(m_prt.name); });
	ui.speed->setText(QString::number(m_prt.speed)); ui.speed->setValidator(&m_doubleVal);
	connect(ui.speed, &QLineEdit::editingFinished, [this]() { m_prt.speed = ui.speed->text().toFloat(); });
	ui.speedVar->setText(QString::number(m_prt.speedVar)); ui.speedVar->setValidator(&m_doubleVal);
	connect(ui.speedVar, &QLineEdit::editingFinished, [this]() { m_prt.speedVar = ui.speedVar->text().toFloat(); });
	ui.coneAngle->setText(QString::number(m_prt.coneAngle)); ui.coneAngle->setValidator(&m_doubleVal);
	connect(ui.coneAngle, &QLineEdit::editingFinished, [this]() { m_prt.coneAngle = ui.coneAngle->text().toFloat(); });
	ui.lifeSpan->setText(QString::number(m_prt.lifeSpan)); ui.lifeSpan->setValidator(&m_doubleVal);
	connect(ui.lifeSpan, &QLineEdit::editingFinished, [this]() { m_prt.lifeSpan = ui.lifeSpan->text().toFloat(); });
	ui.emissionRate->setText(QString::number(m_prt.emissionRate)); ui.emissionRate->setValidator(&m_doubleVal);
	connect(ui.emissionRate, &QLineEdit::editingFinished, [this]() { m_prt.emissionRate = ui.emissionRate->text().toFloat(); });
	ui.width->setText(QString::number(m_prt.width)); ui.width->setValidator(&m_doubleVal);
	connect(ui.width, &QLineEdit::editingFinished, [this]() { m_prt.width = ui.width->text().toFloat(); m_prt.xAxis = m_prt.xAxis.normalized() * max(0.01f, ui.width->text().toFloat()); });
	ui.height->setText(QString::number(m_prt.height)); ui.height->setValidator(&m_doubleVal);
	connect(ui.height, &QLineEdit::editingFinished, [this]() { m_prt.height = ui.height->text().toFloat(); m_prt.yAxis = m_prt.yAxis.normalized() * max(0.01f, ui.height->text().toFloat()); });
	ui.row->setText(QString::number(m_prt.row)); ui.row->setValidator(&m_intVal);
	connect(ui.row, &QLineEdit::editingFinished, [this]() { m_prt.row = ui.row->text().toInt(); });
	ui.col->setText(QString::number(m_prt.col)); ui.col->setValidator(&m_intVal);
	connect(ui.col, &QLineEdit::editingFinished, [this]() { m_prt.col = ui.col->text().toInt(); });
	ui.middleTime->setText(QString::number(m_prt.middleTime)); ui.middleTime->setValidator(&m_doubleVal);
	connect(ui.middleTime, &QLineEdit::editingFinished, [this]() { m_prt.middleTime = ui.middleTime->text().toFloat(); });
	ui.startSize->setText(QString::number(m_prt.startSize)); ui.startSize->setValidator(&m_doubleVal);
	connect(ui.startSize, &QLineEdit::editingFinished, [this]() { m_prt.startSize = ui.startSize->text().toFloat(); });
	ui.midSize->setText(QString::number(m_prt.midSize)); ui.midSize->setValidator(&m_doubleVal);
	connect(ui.midSize, &QLineEdit::editingFinished, [this]() { m_prt.midSize = ui.midSize->text().toFloat(); });
	ui.endSize->setText(QString::number(m_prt.endSize)); ui.endSize->setValidator(&m_doubleVal);
	connect(ui.endSize, &QLineEdit::editingFinished, [this]() { m_prt.endSize = ui.endSize->text().toFloat(); });
	ui.rotVec3->setText(QString::number(m_prt.rotVec[2])); ui.rotVec3->setValidator(&m_doubleVal);
	connect(ui.rotVec3, &QLineEdit::editingFinished, [this]() { m_prt.rotVec[2] = ui.rotVec3->text().toFloat(); });
	ui.rotVel3->setText(QString::number(m_prt.rotVel[2])); ui.rotVel3->setValidator(&m_doubleVal);
	connect(ui.rotVel3, &QLineEdit::editingFinished, [this]() { m_prt.rotVel[2] = ui.rotVel3->text().toFloat(); });
	ui.rotVecVar31->setText(QString::number(m_prt.rotVecVar[4])); ui.rotVecVar31->setValidator(&m_doubleVal);
	connect(ui.rotVecVar31, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[4] = ui.rotVecVar31->text().toFloat(); });
	ui.rotVecVar32->setText(QString::number(m_prt.rotVecVar[5])); ui.rotVecVar32->setValidator(&m_doubleVal);
	connect(ui.rotVecVar32, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[5] = ui.rotVecVar32->text().toFloat(); });
	ui.rotVelVar31->setText(QString::number(m_prt.rotVelVar[4])); ui.rotVelVar31->setValidator(&m_doubleVal);
	connect(ui.rotVelVar31, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[4] = ui.rotVelVar31->text().toFloat(); });
	ui.rotVelVar32->setText(QString::number(m_prt.rotVelVar[5])); ui.rotVelVar32->setValidator(&m_doubleVal);
	connect(ui.rotVelVar32, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[5] = ui.rotVelVar32->text().toFloat(); });
	ui.script->setPlainText(m_prt.script);
	connect(ui.script, &QPlainTextEdit::textChanged, [this]() { m_prt.script = ui.script->toPlainText().replace("\n", "\r\n"); });
	ui.gravityX->setText(QString::number(m_prt.gravityX)); ui.gravityX->setValidator(&m_doubleVal);
	connect(ui.gravityX, &QLineEdit::editingFinished, [this]() { m_prt.gravityX = ui.gravityX->text().toFloat(); });
	ui.gravityY->setText(QString::number(m_prt.gravityY)); ui.gravityY->setValidator(&m_doubleVal);
	connect(ui.gravityY, &QLineEdit::editingFinished, [this]() { m_prt.gravityY = ui.gravityY->text().toFloat(); });
	ui.gravityZ->setText(QString::number(m_prt.gravity)); ui.gravityZ->setValidator(&m_doubleVal);
	connect(ui.gravityZ, &QLineEdit::editingFinished, [this]() { m_prt.gravity = ui.gravityZ->text().toFloat(); });
	ui.lifeRandom1->setText(QString::number(m_prt.lifeRandom[0])); ui.lifeRandom1->setValidator(&m_doubleVal);
	connect(ui.lifeRandom1, &QLineEdit::editingFinished, [this]() { m_prt.lifeRandom[0] = ui.lifeRandom1->text().toFloat(); });
	ui.lifeRandom2->setText(QString::number(m_prt.lifeRandom[1])); ui.lifeRandom2->setValidator(&m_doubleVal);
	connect(ui.lifeRandom2, &QLineEdit::editingFinished, [this]() { m_prt.lifeRandom[1] = ui.lifeRandom2->text().toFloat(); });
	ui.uvAnimFps->setText(QString::number(m_prt.uvAnimFps)); ui.uvAnimFps->setValidator(&m_intVal);
	connect(ui.uvAnimFps, &QLineEdit::editingFinished, [this]() { m_prt.uvAnimFps = ui.uvAnimFps->text().toInt(); });
	ui.numLoop->setText(QString::number(m_prt.numLoop)); ui.numLoop->setValidator(&m_intVal);
	connect(ui.numLoop, &QLineEdit::editingFinished, [this]() { m_prt.numLoop = ui.numLoop->text().toInt(); });
	ui.randSize1->setText(QString::number(m_prt.randSize[0])); ui.randSize1->setValidator(&m_doubleVal);
	connect(ui.randSize1, &QLineEdit::editingFinished, [this]() { m_prt.randSize[0] = ui.randSize1->text().toFloat(); });
	ui.randSize2->setText(QString::number(m_prt.randSize[1])); ui.randSize2->setValidator(&m_doubleVal);
	connect(ui.randSize2, &QLineEdit::editingFinished, [this]() { m_prt.randSize[1] = ui.randSize2->text().toFloat(); });
	ui.rotVec1->setText(QString::number(m_prt.rotVec[0])); ui.rotVec1->setValidator(&m_doubleVal);
	connect(ui.rotVec1, &QLineEdit::editingFinished, [this]() { m_prt.rotVec[0] = ui.rotVec1->text().toFloat(); });
	ui.rotVec2->setText(QString::number(m_prt.rotVec[1])); ui.rotVec2->setValidator(&m_doubleVal);
	connect(ui.rotVec2, &QLineEdit::editingFinished, [this]() { m_prt.rotVec[1] = ui.rotVec2->text().toFloat(); });
	ui.rotVel1->setText(QString::number(m_prt.rotVel[0])); ui.rotVel1->setValidator(&m_doubleVal);
	connect(ui.rotVel1, &QLineEdit::editingFinished, [this]() { m_prt.rotVel[0] = ui.rotVel1->text().toFloat(); });
	ui.rotVel2->setText(QString::number(m_prt.rotVel[1])); ui.rotVel2->setValidator(&m_doubleVal);
	connect(ui.rotVel2, &QLineEdit::editingFinished, [this]() { m_prt.rotVel[1] = ui.rotVel2->text().toFloat(); });
	ui.rotVecVar11->setText(QString::number(m_prt.rotVecVar[0])); ui.rotVecVar11->setValidator(&m_doubleVal);
	connect(ui.rotVecVar11, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[0] = ui.rotVecVar11->text().toFloat(); });
	ui.rotVecVar12->setText(QString::number(m_prt.rotVecVar[1])); ui.rotVecVar12->setValidator(&m_doubleVal);
	connect(ui.rotVecVar12, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[1] = ui.rotVecVar12->text().toFloat(); });
	ui.rotVecVar21->setText(QString::number(m_prt.rotVecVar[2])); ui.rotVecVar21->setValidator(&m_doubleVal);
	connect(ui.rotVecVar21, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[2] = ui.rotVecVar21->text().toFloat(); });
	ui.rotVecVar22->setText(QString::number(m_prt.rotVecVar[3])); ui.rotVecVar22->setValidator(&m_doubleVal);
	connect(ui.rotVecVar22, &QLineEdit::editingFinished, [this]() { m_prt.rotVecVar[3] = ui.rotVecVar22->text().toFloat(); });
	ui.rotVelVar11->setText(QString::number(m_prt.rotVelVar[0])); ui.rotVelVar11->setValidator(&m_doubleVal);
	connect(ui.rotVelVar11, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[0] = ui.rotVelVar11->text().toFloat(); });
	ui.rotVelVar12->setText(QString::number(m_prt.rotVelVar[1])); ui.rotVelVar12->setValidator(&m_doubleVal);
	connect(ui.rotVelVar12, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[1] = ui.rotVelVar12->text().toFloat(); });
	ui.rotVelVar21->setText(QString::number(m_prt.rotVelVar[2])); ui.rotVelVar21->setValidator(&m_doubleVal);
	connect(ui.rotVelVar21, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[2] = ui.rotVelVar21->text().toFloat(); });
	ui.rotVelVar22->setText(QString::number(m_prt.rotVelVar[3])); ui.rotVelVar22->setValidator(&m_doubleVal);
	connect(ui.rotVelVar22, &QLineEdit::editingFinished, [this]() { m_prt.rotVelVar[3] = ui.rotVelVar22->text().toFloat(); });
	ui.bumpAmount->setText(QString::number(m_prt.partEx.bumpAmount)); ui.bumpAmount->setValidator(&m_doubleVal);
	connect(ui.bumpAmount, &QLineEdit::editingFinished, [this]() { m_prt.partEx.bumpAmount = ui.bumpAmount->text().toFloat(); });
	ui.tailStartLength->setText(QString::number(m_prt.tailStartLength)); ui.tailStartLength->setValidator(&m_doubleVal);
	connect(ui.tailStartLength, &QLineEdit::editingFinished, [this]() { m_prt.tailStartLength = ui.tailStartLength->text().toFloat(); });
	ui.tailLength->setText(QString::number(m_prt.tailLength)); ui.tailLength->setValidator(&m_doubleVal);
	connect(ui.tailLength, &QLineEdit::editingFinished, [this]() { m_prt.tailLength = ui.tailLength->text().toFloat(); });
	ui.tailSpeed->setText(QString::number(m_prt.tailSpeed)); ui.tailSpeed->setValidator(&m_doubleVal);
	connect(ui.tailSpeed, &QLineEdit::editingFinished, [this]() { m_prt.tailSpeed = ui.tailSpeed->text().toFloat(); });

	QColor startColor = getColor(m_prt.startColor), midColor = getColor(m_prt.midColor), endColor = getColor(m_prt.endColor);
	startColor.setAlpha(m_prt.alpha[0]); midColor.setAlpha(m_prt.alpha[1]); endColor.setAlpha(m_prt.alpha[2]);
	ui.startColor->setColor(startColor); ui.startColor->setUseAlpha(true);
	connect(ui.startColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_prt.startColor, color); m_prt.alpha[0] = color.alpha(); });
	ui.midColor->setColor(midColor); ui.midColor->setUseAlpha(true);
	connect(ui.midColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_prt.midColor, color); m_prt.alpha[1] = color.alpha(); });
	ui.endColor->setColor(endColor); ui.endColor->setUseAlpha(true);
	connect(ui.endColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_prt.endColor, color); m_prt.alpha[2] = color.alpha(); });
}
