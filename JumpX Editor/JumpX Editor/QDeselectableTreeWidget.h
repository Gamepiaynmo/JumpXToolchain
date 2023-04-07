#pragma once

class QDeselectableTreeWidget : public QTreeWidget {
	Q_OBJECT
public:
	QDeselectableTreeWidget(QWidget *parent) : QTreeWidget(parent) {}

private:
	void clearSel() {
		clearSelection();
		const QModelIndex index;
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
	}

	virtual void mousePressEvent(QMouseEvent *event) {
		QModelIndex item = indexAt(event->pos());
		if (item.isValid() && (!this->selectedIndexes().contains(item) || event->button() != Qt::LeftButton)) {
			QTreeWidget::mousePressEvent(event);
		} else clearSel();
	}
};