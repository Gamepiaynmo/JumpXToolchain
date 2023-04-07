#pragma once

class QDeselectableListWidget : public QListWidget {
	Q_OBJECT
public:
	QDeselectableListWidget(QWidget *parent) : QListWidget(parent) {}

private:
	void clearSel() {
		clearSelection();
		const QModelIndex index;
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
	}

	virtual void mousePressEvent(QMouseEvent *event) {
		QModelIndex item = indexAt(event->pos());
		if (item.isValid() && (!this->selectedIndexes().contains(item) || event->button() != Qt::LeftButton)) {
			QListWidget::mousePressEvent(event);
		} else clearSel();
	}
};