#include "stdafx.h"
#include "WidgetGeometry.h"

WidgetGeometry::WidgetGeometry(XGeometry &geo, QListWidgetItem *item, QList<QString> &materials, QList<QString> &bones, QWidget *parent) : m_geo(geo), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());

	ui.name->setText(m_geo.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_geo.name = ui.name->text(); m_item->setText(m_geo.name); });
	ui.materialId->addItems(materials); clamp(m_geo.materialId, materials.size()); ui.materialId->setCurrentIndex(m_geo.materialId);
	connect(ui.materialId, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_geo.materialId = index; });
	ui.ancestorBone->addItems(bones); clamp(m_geo.ancestorBone, bones.size()); ui.ancestorBone->setCurrentIndex(m_geo.ancestorBone);
	connect(ui.ancestorBone, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) { m_geo.ancestorBone = index; });

	switch (m_geo.type) {
		case XGEO_NORMAL_MESH: ui.type->setCurrentIndex(0); break;
		case XGEO_BILLBOARD: ui.type->setCurrentIndex(1); break;
		case XGEO_FLOOR: ui.type->setCurrentIndex(2); break;
		case XGEO_ALPHA_MESH: ui.type->setCurrentIndex(3); break;
		case XGEO_BOUND_MESH: ui.type->setCurrentIndex(4); break;
		default: ui.type->setCurrentIndex(0); break;
	}
	connect(ui.type, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		switch (index) {
			case 0: m_geo.type = XGEO_NORMAL_MESH; break;
			case 1: m_geo.type = XGEO_BILLBOARD; break;
			case 2: m_geo.type = XGEO_FLOOR; break;
			case 3: m_geo.type = XGEO_ALPHA_MESH; break;
			case 4: m_geo.type = XGEO_BOUND_MESH; break;
			default: m_geo.type = XGEO_NORMAL_MESH; break;
		}
		ui.alwaysStand->setEnabled(index == 1);
		ui.verticalGround->setEnabled(index == 1);
		});
	if (m_geo.type != XGEO_BILLBOARD) {
		ui.alwaysStand->setEnabled(false);
		ui.verticalGround->setEnabled(false);
	}

	ui.alwaysStand->setChecked(m_geo.flag &XGEO_FLAG_ALWAYSESTAND);
	connect(ui.alwaysStand, &QCheckBox::stateChanged, [this](int state) { setFlag(m_geo.flag, XGEO_FLAG_ALWAYSESTAND, state); });
	ui.verticalGround->setChecked(m_geo.flag &XGEO_FLAG_VERTICALGROUND);
	connect(ui.verticalGround, &QCheckBox::stateChanged, [this](int state) { setFlag(m_geo.flag, XGEO_FLAG_VERTICALGROUND, state); });

	ui.numVertex->setText(QString::number(m_geo.numVertex));
	ui.numFace->setText(QString::number(m_geo.numFace));
	ui.script->setPlainText(m_geo.script);
	connect(ui.script, &QPlainTextEdit::textChanged, [this]() { m_geo.script = ui.script->toPlainText().replace("\n", "\r\n"); });
}
