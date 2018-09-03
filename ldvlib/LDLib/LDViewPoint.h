#ifndef __LDVIEWPOINT_H__
#define __LDVIEWPOINT_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCVector.h>
#include <TRE/TREGL.h>
#include <LDLib/LDrawModelViewer.h>

class LDLCamera;

class LDViewPoint: public TCObject
{
public:
	LDViewPoint(void);
	void setCamera(const LDLCamera &value);
	const LDLCamera &getCamera(void) const { return *m_camera; }
	void setRotation(const TCVector &value) { m_rotation = value; }
	const TCVector &getRotation(void) const { return m_rotation; }
	void setCameraRotation(const TCVector &value) { m_cameraRotation = value; }
	const TCVector &getCameraRotation(void) const { return m_cameraRotation; }
	void setPan(const TCVector &value) { m_pan = value; }
	const TCVector &getPan(void) const { return m_pan; }
	void setRotationMatrix(const float *value);
	const float *getRotationMatrix(void) const { return m_rotationMatrix; }
	void setRotationSpeed(TCFloat value) { m_rotationSpeed = value; }
	TCFloat getRotationSpeed(void) const { return m_rotationSpeed; }
	void setAutoCenter(bool value) { m_autoCenter = value; }
	bool getAutoCenter(void) const { return m_autoCenter; }
	void setBackgroundColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
	void getBackgroundColor(GLclampf &r, GLclampf &g, GLclampf &b, GLclampf &a)
		const;
	void setStereoMode(LDVStereoMode value) { m_stereoMode = value; }
	LDVStereoMode getStereoMode(void) const { return m_stereoMode; }
	void setClipAmount(TCFloat value) { m_clipAmount = value; }
	float getClipAmount(void) const { return m_clipAmount; }
	void setDefaultDistance(TCFloat value) { m_defaultDistance = value; }
	TCFloat getDefaultDistance(void) const { return m_defaultDistance; }
protected:
	virtual ~LDViewPoint(void);
	virtual void dealloc(void);

	LDLCamera *m_camera;
	TCVector m_rotation;
	TCVector m_cameraRotation;
	TCVector m_pan;
	TCFloat m_rotationMatrix[16];
	TCFloat m_rotationSpeed;
	bool m_autoCenter;
	GLclampf m_backgroundR;
	GLclampf m_backgroundG;
	GLclampf m_backgroundB;
	GLclampf m_backgroundA;
	LDVStereoMode m_stereoMode;
	TCFloat m_clipAmount;
	TCFloat m_defaultDistance;
};

#endif // __LDVIEWPOINT_H__
