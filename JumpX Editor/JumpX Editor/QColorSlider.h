#pragma once

class QColorSlider : public QLineEdit {
	Q_OBJECT
public:
	QColorSlider(QWidget *parent = nullptr) : QLineEdit(parent) {
		setReadOnly(true);
	}

	void setColorData(XMaterialAnimDataStruct *data, int numKey) {
		if (data) {
			int height = (numKey + 273) / (274 / 2);
			m_image = QImage(274, height, QImage::Format_RGBA8888);
			m_image.fill(QColor(0, 0, 0, 0));
			int width = numKey / height;
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < 274; j++) {
					XCOLOR color = data[width * i + j * width / 274].color;
					m_image.setPixelColor(j, i, QColor(color.r, color.g, color.b, color.a));
				}
			}
			setMinimumHeight(m_image.height() * 16 + 4);
		}
		m_scaled = QImage();
	}

private:
	virtual void mousePressEvent(QMouseEvent *event) override {
		QColorDialog::getColor(m_image.pixelColor(int(event->localPos().x() - 2), int(event->localPos().y() - 2) / 16),
			parentWidget(), "²ÄÖÊÑÕÉ«", QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);
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

		if (m_scaled.size() != paintRect.size())
			m_scaled = m_image.scaled(paintRect.size());
		painter.drawImage(2, 2, m_scaled);
	}

	QImage m_image, m_scaled;
};