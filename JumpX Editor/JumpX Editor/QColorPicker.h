#pragma once

class QColorPicker : public QLineEdit {
	Q_OBJECT
public:
	QColorPicker(QWidget *parent = nullptr) : QLineEdit(parent) {
		setReadOnly(true);
	}

	bool useAlpha() { return m_hasAlpha; }
	void setUseAlpha(bool alpha) { m_hasAlpha = alpha; }
	const QColor &getColor() { return m_color; }
	void setColor(const QColor &color) {
		m_color = color;
	}

signals:
	void colorChanged(const QColor &color);

private:
	virtual void mousePressEvent(QMouseEvent *event) override {
		QColor newColor = QColorDialog::getColor(m_color, parentWidget(), "Ñ¡ÔñÑÕÉ«",
			m_hasAlpha ? QColorDialog::ShowAlphaChannel : QColorDialog::ColorDialogOptions());
		if (newColor.isValid()) {
			m_color = newColor;
			emit colorChanged(newColor);
		}
	}

	virtual void paintEvent(QPaintEvent *event) override {
		QLineEdit::paintEvent(event);
		QPainter painter(this);
		QRect paintRect = contentsRect().adjusted(2, 2, -2, -2);
		for (int x = 0; x < paintRect.width(); x += 8) {
			for (int y = x % 16; y < paintRect.height() - 1; y += 16) {
				painter.fillRect(x + paintRect.x(), y + paintRect.y(),
					min(9, paintRect.width() - x), min(9, paintRect.height() - y), QColor(230, 230, 230));
			}
		}

		painter.fillRect(paintRect, m_hasAlpha ? m_color : QColor::fromRgb(m_color.red(), m_color.green(), m_color.blue()));
	}

	bool m_hasAlpha = true;
	QColor m_color = Qt::white;
};