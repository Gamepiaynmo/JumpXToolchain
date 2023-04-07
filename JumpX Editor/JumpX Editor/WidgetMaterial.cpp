#include "stdafx.h"
#include "WidgetMaterial.h"

WidgetMaterial::WidgetMaterial(XMaterial &mtl, QListWidgetItem *item, QList<QString> &textures, QWidget *parent) : m_mtl(mtl), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	m_intVal.setBottom(1);

	ui.alphaBlend->setChecked(m_mtl.flag & RENDER_ALPHABLEND);
	connect(ui.alphaBlend, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.flag, RENDER_ALPHABLEND, state); });
	ui.alphaTest->setChecked(m_mtl.flag & RENDER_ALPHATEST);
	connect(ui.alphaTest, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.flag, RENDER_ALPHATEST, state); });
	ui.twoSided->setChecked(m_mtl.flag & RENDER_TWOSIDED);
	connect(ui.twoSided, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.flag, RENDER_TWOSIDED, state); });

	uint blend = 0;
	for (int i = 0; i < mtl.numColorKey; i++)
		blend |= mtl.colorKeys[i].blend;
	ui.addMode->setChecked(blend & RENDER_ADD);
	connect(ui.addMode, &QCheckBox::stateChanged, [this](int state) {
		for (int i = 0; i < m_mtl.numColorKey; i++) setFlag(m_mtl.colorKeys[i].blend, RENDER_ADD, state); });
	ui.unshaded->setChecked(blend & RENDER_UNSHADED);
	connect(ui.unshaded, &QCheckBox::stateChanged, [this](int state) {
		for (int i = 0; i < m_mtl.numColorKey; i++) setFlag(m_mtl.colorKeys[i].blend, RENDER_UNSHADED, state); });
	ui.zWriteEnable->setChecked(blend & RENDER_ZWRITEENABLE);
	connect(ui.zWriteEnable, &QCheckBox::stateChanged, [this](int state) {
		for (int i = 0; i < m_mtl.numColorKey; i++) setFlag(m_mtl.colorKeys[i].blend, RENDER_ZWRITEENABLE, state); });
	ui.uvClamp->setChecked(blend & RENDER_UVCLAMP);
	connect(ui.uvClamp, &QCheckBox::stateChanged, [this](int state) {
		for (int i = 0; i < m_mtl.numColorKey; i++) setFlag(m_mtl.colorKeys[i].blend, RENDER_UVCLAMP, state); });
	ui.enableBump->setChecked(m_mtl.exData.flag & EFFECT_BUMP);
	connect(ui.enableBump, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.exData.flag, EFFECT_BUMP, state);
		ui.bumpTexId->setEnabled(state); ui.bumpAmount->setEnabled(state); });
	if (!(m_mtl.exData.flag & EFFECT_BUMP)) {
		ui.bumpTexId->setEnabled(false); ui.bumpAmount->setEnabled(false); }
	ui.enableSpec->setChecked(m_mtl.exData.flag & EFFECT_SPECULAR);
	connect(ui.enableSpec, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.exData.flag, EFFECT_SPECULAR, state);
		ui.specTexId->setEnabled(state); });
	if (!(m_mtl.exData.flag & EFFECT_SPECULAR)) {
		ui.specTexId->setEnabled(false); }
	ui.enableLight->setChecked(m_mtl.exData.flag & EFFECT_LIGHT);
	connect(ui.enableLight, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.exData.flag, EFFECT_LIGHT, state);
		ui.lightTexId->setEnabled(state); });
	if (!(m_mtl.exData.flag & EFFECT_LIGHT)) {
		ui.lightTexId->setEnabled(false); }
	ui.enableCartoon->setChecked(m_mtl.exData.flag & EFFECT_CARTOON);
	connect(ui.enableCartoon, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.exData.flag, EFFECT_CARTOON, state);
		ui.ambientColor->setEnabled(state); ui.shadowColor->setEnabled(state); ui.ambientIntensity->setEnabled(state); ui.shadowThreshold->setEnabled(state);
		ui.specularSmoothness->setEnabled(state); ui.specTextureId->setEnabled(state); ui.shadowTextureId->setEnabled(state); });
	if (!(m_mtl.exData.flag & EFFECT_CARTOON)) {
		ui.ambientColor->setEnabled(false); ui.shadowColor->setEnabled(false); ui.ambientIntensity->setEnabled(false); ui.shadowThreshold->setEnabled(false);
		ui.specularSmoothness->setEnabled(false); ui.specTextureId->setEnabled(false); ui.shadowTextureId->setEnabled(false); }
	ui.enableDissolve->setChecked(m_mtl.exData.flag & EFFECT_DISSOLVE);
	connect(ui.enableDissolve, &QCheckBox::stateChanged, [this](int state) { setFlag(m_mtl.exData.flag, EFFECT_DISSOLVE, state);
		ui.dissolveTextureId->setEnabled(state); ui.dissolveColor->setEnabled(state); ui.dissolveEdgeColor->setEnabled(state);
		if (m_mtl.dissolveKeys == nullptr) { m_mtl.dissolveKeys = new float[m_mtl.numColorKey]; memset(m_mtl.dissolveKeys, 0, sizeof(float) * m_mtl.numColorKey); }
		});
	if (!(m_mtl.exData.flag & EFFECT_DISSOLVE)) {
		ui.dissolveTextureId->setEnabled(false); ui.dissolveColor->setEnabled(false); ui.dissolveEdgeColor->setEnabled(false); }

	ui.textureId->addItems(textures); clamp(m_mtl.textureId, textures.size()); ui.textureId->setCurrentIndex(m_mtl.textureId);
	connect(ui.textureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.textureId = index; });
	ui.bumpTexId->addItems(textures); clamp(m_mtl.exData.bumpTexId, textures.size()); ui.bumpTexId->setCurrentIndex(m_mtl.exData.bumpTexId);
	connect(ui.bumpTexId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.exData.bumpTexId = index; });
	ui.specTexId->addItems(textures); clamp(m_mtl.exData.specTexId, textures.size()); ui.specTexId->setCurrentIndex(m_mtl.exData.specTexId);
	connect(ui.specTexId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.exData.specTexId = index; });
	ui.lightTexId->addItems(textures); clamp(m_mtl.exData.lightTexId, textures.size()); ui.lightTexId->setCurrentIndex(m_mtl.exData.lightTexId);
	connect(ui.lightTexId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.exData.lightTexId = index; });
	ui.specTextureId->addItems(textures); clamp(m_mtl.cartoonData.specTextureID, textures.size()); ui.specTextureId->setCurrentIndex(m_mtl.cartoonData.specTextureID);
	connect(ui.specTextureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.cartoonData.specTextureID = index; });
	ui.shadowTextureId->addItems(textures); clamp(m_mtl.cartoonData.shadowTextureID, textures.size()); ui.shadowTextureId->setCurrentIndex(m_mtl.cartoonData.shadowTextureID);
	connect(ui.shadowTextureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.cartoonData.shadowTextureID = index; });
	ui.dissolveTextureId->addItems(textures); clamp(m_mtl.dissolveData.dissolveTextureID, textures.size()); ui.dissolveTextureId->setCurrentIndex(m_mtl.dissolveData.dissolveTextureID);
	connect(ui.dissolveTextureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_mtl.dissolveData.dissolveTextureID = index; });

	ui.uSpeed->setText(QString::number(m_mtl.uvSpeed[0])); ui.uSpeed->setValidator(&m_doubleVal);
	connect(ui.uSpeed, &QLineEdit::editingFinished, [this]() { m_mtl.uvSpeed[0] = ui.uSpeed->text().toFloat(); });
	ui.vSpeed->setText(QString::number(m_mtl.uvSpeed[1])); ui.vSpeed->setValidator(&m_doubleVal);
	connect(ui.vSpeed, &QLineEdit::editingFinished, [this]() { m_mtl.uvSpeed[1] = ui.vSpeed->text().toFloat(); });
	ui.uTile->setText(QString::number(m_mtl.uTile)); ui.uTile->setValidator(&m_intVal);
	connect(ui.uTile, &QLineEdit::editingFinished, [this]() { m_mtl.uTile = ui.uTile->text().toInt(); });
	ui.vTile->setText(QString::number(m_mtl.vTile)); ui.vTile->setValidator(&m_intVal);
	connect(ui.vTile, &QLineEdit::editingFinished, [this]() { m_mtl.vTile = ui.vTile->text().toInt(); });
	ui.bumpAmount->setText(QString::number(m_mtl.exData.bumpAmount)); ui.bumpAmount->setValidator(&m_doubleVal);
	connect(ui.bumpAmount, &QLineEdit::editingFinished, [this]() { m_mtl.exData.bumpAmount = ui.bumpAmount->text().toFloat(); });
	ui.ambientIntensity->setText(QString::number(m_mtl.cartoonData.ambientIntensity)); ui.ambientIntensity->setValidator(&m_doubleVal);
	connect(ui.ambientIntensity, &QLineEdit::editingFinished, [this]() { m_mtl.cartoonData.ambientIntensity = ui.ambientIntensity->text().toFloat(); });
	ui.shadowThreshold->setText(QString::number(m_mtl.cartoonData.shadowThreshold)); ui.shadowThreshold->setValidator(&m_doubleVal);
	connect(ui.shadowThreshold, &QLineEdit::editingFinished, [this]() { m_mtl.cartoonData.shadowThreshold = ui.shadowThreshold->text().toFloat(); });
	ui.specularSmoothness->setText(QString::number(m_mtl.cartoonData.specularSmoothness)); ui.specularSmoothness->setValidator(&m_doubleVal);
	connect(ui.specularSmoothness, &QLineEdit::editingFinished, [this]() { m_mtl.cartoonData.specularSmoothness = ui.specularSmoothness->text().toFloat(); });

	ui.ambientColor->setColor(getColor(m_mtl.cartoonData.ambientColor, 3)); ui.ambientColor->setUseAlpha(false);
	connect(ui.ambientColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_mtl.cartoonData.ambientColor, 3, color); });
	ui.shadowColor->setColor(getColor(m_mtl.cartoonData.shadowColor, 3)); ui.shadowColor->setUseAlpha(false);
	connect(ui.shadowColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_mtl.cartoonData.shadowColor, 3, color); });
	ui.dissolveColor->setColor(getColor(m_mtl.dissolveData.dissolveColor)); ui.dissolveColor->setUseAlpha(true);
	connect(ui.dissolveColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_mtl.dissolveData.dissolveColor, color); });
	ui.dissolveEdgeColor->setColor(getColor(m_mtl.dissolveData.dissolveEdgeColor)); ui.dissolveEdgeColor->setUseAlpha(true);
	connect(ui.dissolveEdgeColor, &QColorPicker::colorChanged, [this](const QColor &color) { setColor(m_mtl.dissolveData.dissolveEdgeColor, color); });

	ui.colorBar->setColorData(m_mtl.colorKeys, m_mtl.numColorKey);
}