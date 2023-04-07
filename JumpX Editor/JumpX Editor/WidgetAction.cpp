#include "stdafx.h"
#include "WidgetAction.h"

WidgetAction::WidgetAction(XAction &act, QListWidgetItem *item, QWidget *parent) : m_act(act), m_item(item), QWidget(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());
	m_intVal.setBottom(-1);

	ui.name->setText(m_act.name);
	connect(ui.name, &QLineEdit::editingFinished, [this]() { m_act.name = ui.name->text(); m_item->setText(m_act.name); });

	ui.startFrame->setText(QString::number(m_act.startFrame)); ui.startFrame->setValidator(&m_intVal);
	connect(ui.startFrame, &QLineEdit::editingFinished, [this]() { m_act.startFrame = ui.startFrame->text().toInt(); });
	ui.endFrame->setText(QString::number(m_act.endFrame)); ui.endFrame->setValidator(&m_intVal);
	connect(ui.endFrame, &QLineEdit::editingFinished, [this]() { m_act.endFrame = ui.endFrame->text().toInt(); });
	ui.hitPoint->setText(QString::number(m_act.hitPoint)); ui.hitPoint->setValidator(&m_intVal);
	connect(ui.hitPoint, &QLineEdit::editingFinished, [this]() { m_act.hitPoint = ui.hitPoint->text().toInt(); });
	//ui.ribStartFrame->setText(QString::number(m_act.ribStartFrame)); ui.ribStartFrame->setValidator(&m_intVal);
	//connect(ui.ribStartFrame, &QLineEdit::editingFinished, [this]() { m_act.ribStartFrame = ui.ribStartFrame->text().toInt(); });
	//ui.ribEndFrame->setText(QString::number(m_act.ribEndFrame)); ui.ribEndFrame->setValidator(&m_intVal);
	//connect(ui.ribEndFrame, &QLineEdit::editingFinished, [this]() { m_act.ribEndFrame = ui.ribEndFrame->text().toInt(); });
}
