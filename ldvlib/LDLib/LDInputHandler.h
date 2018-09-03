#ifndef __LDINPUTHANDLER_H__
#define __LDINPUTHANDLER_H__
#include <TCFoundation/TCAlertSender.h>

class LDrawModelViewer;
class TCAlert;
class TCVector;

class LDInputHandler : public TCAlertSender
{
public:
	enum ModifierKey
	{
		MKShift = 0x0001,
		MKControl = 0x0002,
		MKAppleControl = 0x0004,
	};
	enum MouseButton
	{
		MBUnknown,
		MBLeft,
		MBFirst = MBLeft,	// This intentionally skips MBUnknown
		MBRight,
		MBMiddle,
		MBLast = MBMiddle
	};
	enum ViewMode
	{
		VMExamine,
		VMFlyThrough,
		VMWalk,
	};
	// Note: the special key values happen to match Windows, but that's only
	// because it made it easy to be sure there were no repeats.  Even the
	// Windows code that relies on these should not assume that they will
	// match.
	enum KeyCode
	{
		KCUnknown,
		KCReturn = 13,
		KCShift = 16,
		KCControl,
		KCAlt,
		KCSpace = ' ',
		KCPageUp,
		KCPageDown,
		KCEnd,
		KCHome,
		KCEscape = 27,
		KCLeft = 38,
		KCUp,
		KCRight,
		KCDown,
		KCInsert = 45,
		KCDelete,
		KCA = 'A',
		KCB,
		KCC,
		KCD,
		KCE,
		KCF,
		KCG,
		KCH,
		KCI,
		KCJ,
		KCK,
		KCL,
		KCM,
		KCN,
		KCO,
		KCP,
		KCQ,
		KCR,
		KCS,
		KCT,
		KCU,
		KCV,
		KCW,
		KCX,
		KCY,
		KCZ,
	};

	LDInputHandler(LDrawModelViewer *modelViewer);

	void setViewMode(ViewMode value);
	ViewMode getViewMode(void) const { return m_viewMode; }
	bool mouseDown(TCULong modifierKeys, MouseButton button, int x, int y);
	bool mouseUp(TCULong modifierKeys, MouseButton button, int x, int y);
	bool mouseMove(TCULong modifierKeys, int x, int y);
	bool mouseWheel(TCULong modifierKeys, TCFloat amount);
	bool mouseCaptureChanged(void);
	bool keyDown(TCULong modifierKeys, KeyCode keyCode);
	bool keyUp(TCULong modifierKeys, KeyCode keyCode);
	void setMouseUpPending(bool value);
	void cancelMouseDrag(void);
	virtual TCObject *getAlertSender(void);
	void stopRotation(void);

	static const char *captureAlertClass(void) { return "LDCaptureMouse"; }
	static const char *releaseAlertClass(void) { return "LDReleaseMouse"; }
	static const char *peekMouseUpAlertClass(void) { return "LDPeekMouseUp"; }
protected:
	typedef enum
	{
		MMNone,
		MMNormal,
		MMZoom,
		MMPan,
		MMLight,
	} MouseMode;

	~LDInputHandler(void);
	void dealloc(void);
	bool leftDown(TCULong modifierKeys, int xPos, int yPos);
	void updateSpinRateXY(int xPos, int yPos);
	void updateXY(TCULong modifierKeys, int xPos, int yPos);
	void updateHeadXY(TCULong modifierKeys, int xPos, int yPos);
	void updatePanXY(int xPos, int yPos);
	void updateLightXY(int xPos, int yPos);
	void updateZoomY(int yPos);
	void frameDone(TCAlert *alert);
	void updateCameraMotion(TCVector &cameraMotion, TCFloat motionAmount,
		TCFloat strafeAmount);
	void updateCameraRotation(TCFloat rotationAmount, TCFloat rollAmount);
	void updateRotation(TCFloat rotationSpeed);
	void recordRotationStop(void);
	void clearRotationStop(void);
	bool checkSpin(void);
	//double getTimeRef(void);

	LDrawModelViewer *m_modelViewer;
	ViewMode m_viewMode;
	MouseMode m_mouseMode;
	bool m_buttonsDown[3];
	bool m_appleRightClick;
	int m_numButtons;
	int m_clickX;
	int m_clickY;
	int m_lastX;
	int m_lastY;
	int m_lastFrameX;
	int m_lastFrameY;
	TCFloat m_rotationSpeed;
	bool m_mouseUpPending;
	bool m_mouseUpHandled;
	//double m_lastMoveTime;

	float m_lastXRotate;
	float m_lastYRotate;
#ifdef WIN32
	DWORD m_stopTicks;
	bool m_haveStopTicks;
#endif // WIN32
#ifdef COCOA
	void *m_stopTime;
#endif // COCOA
	static TCFloat sm_keyRotationSpeed;
};

#endif // __LDINPUTHANDLER_H__
