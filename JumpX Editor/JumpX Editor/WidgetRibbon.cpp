#include "stdafx.h"
#include "WidgetRibbon.h"

WidgetRibbon::WidgetRibbon(XRibbon &rib, QListWidgetItem *item, QList<QString> &textures, QList<QString> &bones, QWidget *parent) : m_rib(rib), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	m_intVal.setBottom(1);

	ui.textureId->addItems(textures); clamp(m_rib.textureId, textures.size()); ui.textureId->setCurrentIndex(m_rib.textureId);
	connect(ui.textureId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_rib.textureId = index; });
	ui.boneId->addItems(bones); clamp(m_rib.boneId, bones.size()); ui.boneId->setCurrentIndex(m_rib.boneId);
	connect(ui.boneId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_rib.boneId = index; });

	ui.name->setText(m_rib.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_rib.name = ui.name->text(); m_item->setText(m_rib.name); });

	ui.edgeLifeSec->setText(QString::number(m_rib.edgeLifeSec)); ui.edgeLifeSec->setValidator(&m_intVal);
	connect(ui.edgeLifeSec, &QLineEdit::editingFinished, [this]() { m_rib.edgeLifeSec = ui.edgeLifeSec->text().toInt(); });
	ui.edgePerSec->setText(QString::number(m_rib.edgePerSec)); ui.edgePerSec->setValidator(&m_intVal);
	connect(ui.edgePerSec, &QLineEdit::editingFinished, [this]() { m_rib.edgePerSec = ui.edgePerSec->text().toInt(); });
	ui.maxEdgeCount->setText(QString::number(m_rib.maxEdgeCount)); ui.maxEdgeCount->setValidator(&m_intVal);
	connect(ui.maxEdgeCount, &QLineEdit::editingFinished, [this]() { m_rib.maxEdgeCount = ui.maxEdgeCount->text().toInt(); });

	ui.blendMode->setChecked(!m_rib.blendMode);
	connect(ui.blendMode, &QCheckBox::stateChanged, [this](int state) { m_rib.blendMode = !state; });

	QColor color(m_rib.ribEx.color[2], m_rib.ribEx.color[1], m_rib.ribEx.color[0]);
	color.setAlpha(m_rib.ribEx.alpha);
	ui.color->setColor(color); ui.color->setUseAlpha(true);
	connect(ui.color, &QColorPicker::colorChanged, [this](const QColor &color) { m_rib.ribEx.color[0] = color.blue();
		m_rib.ribEx.color[1] = color.green(); m_rib.ribEx.color[2] = color.red(); m_rib.ribEx.alpha = color.alpha(); });
}
