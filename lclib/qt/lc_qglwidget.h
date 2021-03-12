#pragma once

#include <QGLWidget>

class lcQGLWidget : public QGLWidget
{
public:
	lcQGLWidget(QWidget* Parent, lcGLWidget* Owner);
	~lcQGLWidget();

	QSize sizeHint() const override;

	lcGLWidget* mWidget;

	float GetDeviceScale() const
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
		return windowHandle()->devicePixelRatio();
#else
		return 1.0f;
#endif
	}

/*** LPub3D Mod - preview widget for LPub3D ***/
	void SetPreferredSize(const QSize &PreferredSize)
	{
		mPreferredSize = PreferredSize;
	}
	void SetPreviewPosition(const QRect& ParentRect, const QPoint &ViewPos = QPoint(), bool UseViewPos = false);
/*** LPub3D Mod end ***/

protected:
	void resizeGL(int Width, int Height) override;
	void paintGL() override;
	void focusInEvent(QFocusEvent* FocusEvent) override;
	void focusOutEvent(QFocusEvent* FocusEvent) override;
	void keyPressEvent(QKeyEvent* KeyEvent) override;
	void keyReleaseEvent(QKeyEvent* KeyEvent) override;
	void mousePressEvent(QMouseEvent* MouseEvent) override;
	void mouseReleaseEvent(QMouseEvent* MouseEvent) override;
	void mouseDoubleClickEvent(QMouseEvent* MouseEvent) override;
	void mouseMoveEvent(QMouseEvent* MouseEvent) override;
	void wheelEvent(QWheelEvent* WheelEvent) override;
	void dragEnterEvent(QDragEnterEvent* DragEnterEvent) override;
	void dragLeaveEvent(QDragLeaveEvent* DragLeaveEvent) override;
	void dragMoveEvent(QDragMoveEvent* DragMoveEvent) override;
	void dropEvent(QDropEvent* DropEvent) override;

	QSize mPreferredSize;
	int mWheelAccumulator;
};
