#ifndef __LDLCAMERA_H__
#define __LDLCAMERA_H__

#include <LDLoader/LDLFacing.h>

class LDLCamera
{
public:
	LDLCamera(void);
	LDLCamera(const LDLCamera& other);
	~LDLCamera(void);

	TCVector getPosition(void) const { return position; }
	void setPosition(const TCVector &pos) { position = pos; }
	LDLFacing getFacing(void) const { return facing; }
	void setFacing(const LDLFacing &newFacing) { facing = newFacing; }
	const char* getName(void) const { return name; }
	void setName(char* n);
	LDLCamera& operator=(const LDLCamera& right) {return duplicate(right);}
	void rotate(const TCVector &rotation);
	void move(const TCVector &distance);
protected:
	LDLCamera& duplicate(const LDLCamera& copyFrom);

	TCVector position;
	LDLFacing facing;
	char* name;
};

#endif // __LDLCAMERA_H__
