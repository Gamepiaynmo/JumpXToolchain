#pragma once

#include <QSplashScreen>

class SplashScreen : public QSplashScreen
{
	Q_OBJECT

public:
	SplashScreen(const QPixmap &pixmap);
	~SplashScreen();

	void paintEvent(QPaintEvent *e) override;
};
