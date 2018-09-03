#include "LDInputHandler.h"
#include "LDrawModelViewer.h"
#include <TCFoundation/TCAlertManager.h>
#include <TCFoundation/TCAlert.h>
#include <TCFoundation/TCMacros.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

#ifdef COCOA
#include <Foundation/Foundation.h>
#define STOP_TIME ((NSDate *&)m_stopTime)
#endif // COCOA

TCFloat LDInputHandler::sm_keyRotationSpeed = 0.5f;

LDInputHandler::LDInputHandler(LDrawModelViewer *m_modelViewer):
	m_modelViewer(m_modelViewer),
	m_viewMode(VMExamine),
	m_mouseMode(MMNone),
	m_appleRightClick(false),
	m_numButtons(sizeof(m_buttonsDown) / sizeof(m_buttonsDown[0]))
#ifdef WIN32
	, m_haveStopTicks(false)
#endif // WIN32
#ifdef COCOA
	, m_stopTime(NULL)
#endif // COCOA
{
	memset(&m_buttonsDown, 0, sizeof(m_buttonsDown));
	TCAlertManager::registerHandler(LDrawModelViewer::frameDoneAlertClass(),
		this, (TCAlertCallback)&LDInputHandler::frameDone);
}

LDInputHandler::~LDInputHandler(void)
{
}

void LDInputHandler::dealloc(void)
{
#ifdef COCOA
	[STOP_TIME release];
#endif // COCOA
	TCAlertManager::unregisterHandler(this);
	TCObject::dealloc();
}

void LDInputHandler::cancelMouseDrag(void)
{
	m_mouseMode = MMNone;
	for (int i = 0; i < m_numButtons; i++)
	{
		m_buttonsDown[i] = false;
	}
}

void LDInputHandler::setViewMode(LDInputHandler::ViewMode value)
{
	stopRotation();
	m_viewMode = value;
	if (m_modelViewer)
	{
		m_modelViewer->setViewMode((LDrawModelViewer::ViewMode)value);
		if (value == VMExamine)
		{
			m_modelViewer->setConstrainZoom(true);
		}
		else
		{
			m_modelViewer->setConstrainZoom(false);
		}
		m_modelViewer->requestRedraw();
	}
}

void LDInputHandler::stopRotation(void)
{
	recordRotationStop();
	m_rotationSpeed = 0.0f;
	m_modelViewer->setXRotate(0.0f);
	m_modelViewer->setYRotate(0.0f);
	m_modelViewer->setRotationSpeed(m_rotationSpeed);
}

void LDInputHandler::updateSpinRateXY(int xPos, int yPos)
{
	int deltaX = xPos - m_lastX;
	int deltaY = yPos - m_lastY;
	TCFloat magnitude = (TCFloat)sqrt((TCFloat)(deltaX * deltaX +
		deltaY * deltaY));

	m_rotationSpeed = magnitude / 10.0f;
	if (fEq(m_rotationSpeed, 0.0f))
	{
		stopRotation();
	}
	else
	{
		m_modelViewer->setXRotate((TCFloat)deltaY);
		m_modelViewer->setYRotate((TCFloat)deltaX);
	}
	m_modelViewer->setRotationSpeed(m_rotationSpeed);
	m_modelViewer->requestRedraw();
}

void LDInputHandler::updateHeadXY(TCULong modifierKeys, int xPos, int yPos)
{
	TCFloat magnitude = (TCFloat)(xPos - m_clickX);
	TCFloat denom = 5000.0f;
	TCFloat fov = m_modelViewer->getFov();

	denom /= (TCFloat)tan(deg2rad(fov));
	if (modifierKeys & MKShift)
	{
		debugPrintf("Faster.\n");
		denom /= 2.0f;
	}
	m_modelViewer->setCameraXRotate(magnitude / denom);
	magnitude = (TCFloat)(yPos - m_clickY);
	m_modelViewer->setCameraYRotate(magnitude / -denom);
	m_modelViewer->requestRedraw();
}

void LDInputHandler::updatePanXY(int xPos, int yPos)
{
	if (m_viewMode == VMExamine)
	{
		int deltaX = xPos - m_lastX;
		int deltaY = yPos - m_lastY;

		m_modelViewer->panXY(deltaX, deltaY);
		m_modelViewer->requestRedraw();
	}
}

void LDInputHandler::updateZoomY(int yPos)
{
	TCFloat magnitude = (TCFloat)(yPos - m_clickY);

	m_modelViewer->setZoomSpeed(magnitude / 2.0f);
	m_modelViewer->requestRedraw();
}

bool LDInputHandler::leftDown(TCULong modifierKeys, int xPos, int yPos)
{
	if (modifierKeys & MKShift)
	{
		if (!m_modelViewer->getUseLighting() ||
			m_modelViewer->getContrastingLightDirModel() == NULL)
		{
			// Allowing this will likely just lead to confusion.
			return false;
		}
		m_mouseMode = MMLight;
		m_modelViewer->setShowLightDir(true);
		m_modelViewer->requestRedraw();
	}
	else if (modifierKeys & MKControl)
	{
		m_mouseMode = MMPan;
	}
	else
	{
		m_mouseMode = MMNormal;
		updateSpinRateXY(xPos, yPos);
	}
	clearRotationStop();
	return true;
}

bool LDInputHandler::mouseDown(
	TCULong modifierKeys,
	MouseButton button,
	int x,
	int y)
{
	if (button < MBFirst || button > MBLast)
	{
		return false;
	}
	if (modifierKeys & MKAppleControl && button == MBLeft)
	{
		m_appleRightClick = true;
		button = MBRight;
	}
	// Don't process chorded mouse clicks.
	for (int i = 0; i < m_numButtons; i++)
	{
		if (i != button && m_buttonsDown[i])
		{
			return false;
		}
	}
	m_buttonsDown[button] = true;
	m_lastX = m_lastFrameX = m_clickX = x;
	m_lastY = m_lastFrameY = m_clickY = y;
	switch (button)
	{
	case MBLeft:
		if (!leftDown(modifierKeys, x, y))
		{
			return false;
		}
		break;
	case MBRight:
		m_modelViewer->setClipZoom((modifierKeys & MKControl) != 0);
		m_mouseMode = MMZoom;
		break;
	case MBMiddle:
		m_mouseMode = MMPan;
		break;
	default:
		return false;
	}
	TCAlertManager::sendAlert(captureAlertClass(), this);
	return true;
}

bool LDInputHandler::mouseCaptureChanged(void)
{
	if (m_buttonsDown[MBLeft])
	{
		if (m_viewMode == VMFlyThrough || m_viewMode == VMWalk)
		{
			m_modelViewer->setCameraXRotate(0.0f);
			m_modelViewer->setCameraYRotate(0.0f);
		}
		else
		{
			m_modelViewer->setXRotate(0.0f);
			m_modelViewer->setYRotate(0.0f);
			m_modelViewer->setRotationSpeed(0.0f);
		}
		m_modelViewer->setShowLightDir(false);
	}
	if (m_buttonsDown[MBRight])
	{
		m_modelViewer->setZoomSpeed(0.0f);
	}
	m_mouseMode = MMNone;
	for (int i = 0; i < m_numButtons; i++)
	{
		m_buttonsDown[i] = false;
	}
	return true;
}

bool LDInputHandler::mouseUp(
	TCULong modifierKeys,
	MouseButton button,
	int x,
	int y)
{
	if (button < MBFirst || button > MBLast)
	{
		return false;
	}
	if (button == MBLeft && m_appleRightClick)
	{
		m_appleRightClick = false;
		button = MBRight;
	}
	if (!m_buttonsDown[button])
	{
		return false;
	}
	if (x != m_lastX || y != m_lastY)
	{
		mouseMove(modifierKeys, x, y);
	}
	switch (button)
	{
	case MBLeft:
		if (m_viewMode == VMFlyThrough || m_viewMode == VMWalk)
		{
			m_modelViewer->setCameraXRotate(0.0f);
			m_modelViewer->setCameraYRotate(0.0f);
		}
		else if (checkSpin())
		{
			m_rotationSpeed =
				(TCFloat)sqrt((TCFloat)(m_lastXRotate * m_lastXRotate +
				m_lastYRotate * m_lastYRotate)) / 10.0f;
			m_modelViewer->setXRotate(m_lastXRotate);
			m_modelViewer->setYRotate(m_lastYRotate);
			m_modelViewer->setRotationSpeed(m_rotationSpeed);
		}
		m_modelViewer->setShowLightDir(false);
		if (m_modelViewer->getExamineMode() == LDrawModelViewer::EMLatLong)
		{
			// In latitude/longitude mode, we don't want it to spin along both
			// axes at once, because once the latitude gets to 90 or -90, it
			// just stops.
			if (fabs(m_modelViewer->getXRotate()) >
				fabs(m_modelViewer->getYRotate()))
			{
				m_modelViewer->setYRotate(0.0f);
			}
			else if (fabs(m_modelViewer->getYRotate()) >
				fabs(m_modelViewer->getXRotate()))
			{
				m_modelViewer->setXRotate(0.0f);
			}
			else if (fabs(m_modelViewer->getYRotate()) ==
				fabs(m_modelViewer->getXRotate()) &&
				m_modelViewer->getXRotate() != 0.0f)
			{
				m_modelViewer->setXRotate(0.0f);
			}
		}
		break;
	case MBRight:
		m_modelViewer->setZoomSpeed(0.0f);
	default:
		break;
	}
	TCAlertManager::sendAlert(releaseAlertClass(), this);
	m_modelViewer->requestRedraw();
	m_buttonsDown[button] = false;
	m_mouseMode = MMNone;
	return true;
}

void LDInputHandler::updateLightXY(int xPos, int yPos)
{
	m_modelViewer->mouseMoveLight(xPos - m_lastX, yPos - m_lastY);
	m_modelViewer->requestRedraw();
}

void LDInputHandler::updateXY(TCULong modifierKeys, int xPos, int yPos)
{
	if (m_viewMode == VMExamine)
	{
		//m_lastMoveTime = getTimeRef();
		updateSpinRateXY(xPos, yPos);
	}
	else
	{
		updateHeadXY(modifierKeys, xPos, yPos);
	}
}

bool LDInputHandler::mouseMove(TCULong modifierKeys, int x, int y)
{
	bool retValue = true;

	switch (m_mouseMode)
	{
	case MMNormal:
		updateXY(modifierKeys, x, y);
		break;
	case MMZoom:
		updateZoomY(y);
		break;
	case MMPan:
		updatePanXY(x, y);
		break;
	case MMLight:
		updateLightXY(x, y);
		break;
	default:
		retValue = false;
		break;
	}
	m_lastX = x;
	m_lastY = y;
	return retValue;
}

bool LDInputHandler::mouseWheel(TCULong modifierKeys, TCFloat amount)
{
	// Don't process while while any mouse buttons are pressed.
	for (int i = 0; i < m_numButtons; i++)
	{
		if (m_buttonsDown[i])
		{
			return false;
		}
	}
	m_modelViewer->setClipZoom((modifierKeys & MKControl) != 0);
	m_modelViewer->zoom((TCFloat)amount * -0.5f);
	m_modelViewer->requestRedraw();
	return true;
}

void LDInputHandler::setMouseUpPending(bool value)
{
	m_mouseUpPending = value;
	m_mouseUpHandled = true;
}

void LDInputHandler::frameDone(TCAlert *alert)
{
	if (m_mouseMode == MMNormal && m_viewMode == VMExamine &&
		alert->getSender() == m_modelViewer)
	{
		// We're spinning, and a frame just completed drawing, so
		// check to see if we should stop the spinning.
		m_mouseUpHandled = false;
		TCAlertManager::sendAlert(peekMouseUpAlertClass(), this);
		if (!m_mouseUpHandled ||
			(m_mouseUpHandled && m_mouseUpPending))
		{
			updateSpinRateXY(m_lastX, m_lastY);
		}
		m_lastFrameX = m_lastX;
		m_lastFrameY = m_lastY;
	}
}

void LDInputHandler::updateCameraMotion(
	TCVector &cameraMotion,
	TCFloat motionAmount,
	TCFloat strafeAmount)
{
	for (int i = 0; i < 2; i++)
	{
		if (cameraMotion[i] > 0.0f)
		{
			cameraMotion[i] = strafeAmount;
		}
		else if (cameraMotion[i] < 0.0f)
		{
			cameraMotion[i] = -strafeAmount;
		}
	}
	if (cameraMotion[2] > 0.0f)
	{
		cameraMotion[2] = motionAmount;
	}
	else if (cameraMotion[2] < 0.0f)
	{
		cameraMotion[2] = -motionAmount;
	}
}

void LDInputHandler::updateCameraRotation(
	TCFloat rotationAmount,
	TCFloat rollAmount)
{
	TCFloat value = m_modelViewer->getCameraXRotate();

	if (value > 0)
	{
		m_modelViewer->setCameraXRotate(rotationAmount);
	}
	else if (value < 0)
	{
		m_modelViewer->setCameraXRotate(-rotationAmount);
	}
	value = m_modelViewer->getCameraYRotate();
	if (value > 0)
	{
		m_modelViewer->setCameraYRotate(rotationAmount);
	}
	else if (value < 0)
	{
		m_modelViewer->setCameraYRotate(-rotationAmount);
	}
	value = m_modelViewer->getCameraZRotate();
	if (value > 0)
	{
		m_modelViewer->setCameraZRotate(rollAmount);
	}
	else if (value < 0)
	{
		m_modelViewer->setCameraZRotate(-rollAmount);
	}
}

bool LDInputHandler::keyDown(TCULong modifierKeys, KeyCode keyCode)
{
	if (m_viewMode == VMExamine)
	{
		TCFloat rotationSpeed = sm_keyRotationSpeed;

		if (modifierKeys & MKShift)
		{
			rotationSpeed *= 2.0f;
		}
		switch (keyCode)
		{
		case KCUp:
			m_modelViewer->setXRotate(-1.0f);
			m_rotationSpeed = rotationSpeed;
			break;
		case KCDown:
			m_modelViewer->setXRotate(1.0f);
			m_rotationSpeed = rotationSpeed;
			break;
		case KCLeft:
			m_modelViewer->setYRotate(-1.0f);
			m_rotationSpeed = rotationSpeed;
			break;
		case KCRight:
			m_modelViewer->setYRotate(1.0f);
			m_rotationSpeed = rotationSpeed;
			break;
		case KCShift:
			if (fEq(m_modelViewer->getRotationSpeed(), 0.0f))
			{
				return true;
			}
			m_rotationSpeed = rotationSpeed;
			break;
		case KCSpace:
			stopRotation();
			break;
		default:
			return false;
		}
		m_modelViewer->setRotationSpeed(m_rotationSpeed);
		m_modelViewer->requestRedraw();
		return true;
	}
	else if (m_viewMode == VMFlyThrough || m_viewMode == VMWalk)
	{
		TCVector cameraMotion = m_modelViewer->getCameraMotion();
		TCFloat fov = m_modelViewer->getFov();
		TCFloat motionAmount = m_modelViewer->getDefaultDistance() / 400.0f;
		TCFloat rotationAmount = 0.01f * (TCFloat)tan(deg2rad(fov));
		TCFloat rollAmount = 0.01f;
		TCFloat strafeAmount = 1.0f * (TCFloat)sqrt(fov / 45.0f);

		if (modifierKeys & MKShift)
		{
			motionAmount *= 2.0f;
			strafeAmount *= 2.0f;
			rotationAmount *= 2.0f;
			rollAmount *= 2.0f;
		}
		switch (keyCode)
		{
		case KCW:
			cameraMotion[2] = -motionAmount;
			break;
		case KCS:
			cameraMotion[2] = motionAmount;
			break;
		case KCA:
			cameraMotion[0] = -strafeAmount;
			break;
		case KCD:
			cameraMotion[0] = strafeAmount;
			break;
		case KCR:
			cameraMotion[1] = strafeAmount;
			break;
		case KCF:
			cameraMotion[1] = -strafeAmount;
			break;
		case KCE:
			m_modelViewer->setCameraZRotate(rollAmount);
			break;
		case KCQ:
			m_modelViewer->setCameraZRotate(-rollAmount);
			break;
		case KCUp:
			m_modelViewer->setCameraYRotate(rotationAmount);
			break;
		case KCDown:
			m_modelViewer->setCameraYRotate(-rotationAmount);
			break;
		case KCLeft:
			m_modelViewer->setCameraXRotate(-rotationAmount);
			break;
		case KCRight:
			m_modelViewer->setCameraXRotate(rotationAmount);
			break;
		case KCShift:
			updateCameraMotion(cameraMotion, motionAmount, strafeAmount);
			updateCameraRotation(rotationAmount, rollAmount);
			break;
		default:
			return false;
			break;
		}
		m_modelViewer->setCameraMotion(cameraMotion);
		m_modelViewer->requestRedraw();
		//forceRedraw(2);
		return true;
	}
	return false;
}

bool LDInputHandler::keyUp(TCULong /*modifierKeys*/, KeyCode keyCode)
{
	if (m_viewMode == VMExamine)
	{
		switch (keyCode)
		{
		case KCUp:
		case KCDown:
			m_modelViewer->setXRotate(0.0f);
			break;
		case KCLeft:
		case KCRight:
			m_modelViewer->setYRotate(0.0f);
			break;
		case KCShift:
			m_rotationSpeed = sm_keyRotationSpeed;
			break;
		default:
			return false;
		}
		if (m_modelViewer->getXRotate() == 0.0f &&
			m_modelViewer->getYRotate() == 0.0f)
		{
			m_rotationSpeed = 0.0f;
		}
		m_modelViewer->setRotationSpeed(m_rotationSpeed);
		m_modelViewer->requestRedraw();
		return true;
	}
	else if (m_viewMode == VMFlyThrough || m_viewMode == VMWalk)
	{
		TCVector cameraMotion = m_modelViewer->getCameraMotion();

		switch (keyCode)
		{
		case KCW:
			cameraMotion[2] = 0.0f;
			break;
		case KCS:
			cameraMotion[2] = 0.0f;
			break;
		case KCA:
			cameraMotion[0] = 0.0f;
			break;
		case KCD:
			cameraMotion[0] = 0.0f;
			break;
		case KCR:
			cameraMotion[1] = 0.0f;
			break;
		case KCF:
			cameraMotion[1] = 0.0f;
			break;
		case KCE:
			m_modelViewer->setCameraZRotate(0.0f);
			break;
		case KCQ:
			m_modelViewer->setCameraZRotate(0.0f);
			break;
		case KCUp:
			m_modelViewer->setCameraYRotate(0.0f);
			break;
		case KCDown:
			m_modelViewer->setCameraYRotate(0.0f);
			break;
		case KCLeft:
			m_modelViewer->setCameraXRotate(0.0f);
			break;
		case KCRight:
			m_modelViewer->setCameraXRotate(0.0f);
			break;
		case KCShift:
			{
				TCFloat fov = m_modelViewer->getFov();
				TCFloat motionAmount = m_modelViewer->getDefaultDistance() /
					400.0f;
				TCFloat rotationAmount = 0.01f * (TCFloat)tan(deg2rad(fov));
				TCFloat strafeAmount = 1.0f * (TCFloat)sqrt(fov / 45.0f);

				updateCameraMotion(cameraMotion, motionAmount, strafeAmount);
				updateCameraRotation(rotationAmount, 0.01f);
			}
			break;
		default:
			return 1;
			break;
		}
		m_modelViewer->setCameraMotion(cameraMotion);
		m_modelViewer->requestRedraw();
		//forceRedraw(2);
		return 0;
	}
	return false;
}

TCObject *LDInputHandler::getAlertSender(void)
{
	return m_modelViewer;
}

void LDInputHandler::recordRotationStop(void)
{
	if (!fEq(m_modelViewer->getXRotate(), 0.0f) ||
		!fEq(m_modelViewer->getYRotate(), 0.0f))
	{
		m_lastXRotate = m_modelViewer->getXRotate();
		m_lastYRotate = m_modelViewer->getYRotate();
#ifdef WIN32
		m_stopTicks = GetTickCount();
		m_haveStopTicks = true;
#endif // WIN32
#ifdef COCOA
		[STOP_TIME release];
		STOP_TIME = [[NSDate alloc] init];
#endif // COCOA
	}
}

void LDInputHandler::clearRotationStop(void)
{
#ifdef WIN32
	m_haveStopTicks = false;
#endif // WIN32
#ifdef COCOA
	[STOP_TIME release];
	STOP_TIME = nil;
#endif // COCOA
}

bool LDInputHandler::checkSpin(void)
{
	bool retValue = false;
#ifdef WIN32
	if (m_haveStopTicks)
	{
		retValue = GetTickCount() - m_stopTicks < 50;
		m_haveStopTicks = false;
	}
#endif // WIN32
#ifdef COCOA
	retValue = STOP_TIME != nil && [STOP_TIME timeIntervalSinceNow] > -0.1f;
	[STOP_TIME release];
	STOP_TIME = nil;
#endif // COCOA
	return retValue;
}
