#ifndef __LDLPRIMITIVECHECK_H__
#define __LDLPRIMITIVECHECK_H__

#include <TCFoundation/TCAlertSender.h>

#include <string.h>

class LDLModel;

class LDLPrimitiveCheck : public TCAlertSender
{
public:
	LDLPrimitiveCheck(void);
	void setPrimitiveSubstitutionFlag(bool value)
	{
		m_flags.primitiveSubstitution = value;
	}
	bool getPrimitiveSubstitutionFlag(void)
	{
		return m_flags.primitiveSubstitution != false;
	}
	void setCurveQuality(int value) { m_curveQuality = value; }
	int getCurveQuality(void) { return m_curveQuality; }
	bool getNoLightGeomFlag(void) { return m_flags.noLightGeom != false; }
	void setNoLightGeomFlag(bool value) { m_flags.noLightGeom = value; }
	virtual TCObject *getAlertSender(void) { return m_alertSender; }
	virtual void setAlertSender(TCObject *value) { m_alertSender = value; }
	virtual void setTexmapsFlag(bool value ) { m_flags.texmaps = value; }
	virtual bool getTexmapsFlag(void) const { return m_flags.texmaps != false; }
protected:
	virtual ~LDLPrimitiveCheck(void);
	virtual void dealloc(void);
	virtual bool performPrimitiveSubstitution(LDLModel *ldlModel,
		bool bfc);
	virtual TCFloat getTorusFraction(int size);
	virtual bool substituteStud(int /*numSegments*/) { return false; }
	virtual bool substituteStud(void) { return false; }
	virtual bool substituteStu2(void) { return false; }
	virtual bool substituteStu22(bool /*isA*/, bool /*bfc*/) { return false; }
	virtual bool substituteStu23(bool /*isA*/, bool /*bfc*/) { return false; }
	virtual bool substituteStu24(bool /*isA*/, bool /*bfc*/) { return false; }
	virtual bool substituteTorusIO(bool /*inner*/, TCFloat /*fraction*/,
		int /*size*/, bool /*bfc*/, bool /*isMixed*/, bool /*is48*/ = false)
		{ return false; }
	virtual bool substituteTorusQ(TCFloat /*fraction*/, int /*size*/,
		bool /*bfc*/, bool /*isMixed*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteEighthSphere(bool /*bfc*/,
		bool /*is48*/ = false) { return false; }
	virtual bool substituteEighthSphereCorner(bool /*bfc*/,
		bool /*is48*/ = false) { return false; }
	virtual bool substituteCylinder(TCFloat /*fraction*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteSlopedCylinder(TCFloat /*fraction*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteSlopedCylinder2(TCFloat /*fraction*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteChrd(TCFloat /*fraction*/, bool /*bfc*/,
		bool /*is48*/ = false) { return false; }
	virtual bool substituteDisc(TCFloat /*fraction*/, bool /*bfc*/,
		bool /*is48*/ = false) { return false; }
	virtual bool substituteNotDisc(TCFloat /*fraction*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteTangent(TCFloat /*fraction*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteCircularEdge(TCFloat /*fraction*/,
		bool /*is48*/ = false) { return false; }
	virtual bool substituteCone(TCFloat /*fraction*/, int /*size*/,
		bool /*bfc*/, bool /*is48*/ = false) { return false; }
	virtual bool substituteRing(TCFloat /*fraction*/, int /*size*/,
		bool /*bfc*/, bool /*is48*/ = false, bool /*isOld*/ = false)
		{ return false; }
	virtual TCFloat startingFraction(const char *filename);
	virtual size_t getStartingFractionLength(const char *filename);
	virtual bool startsWithFraction(const char *filename);
	virtual bool startsWithFraction2(const char *filename);
	virtual bool isPrimitive(const char *filename, const char *suffix,
		bool *is48 = NULL);
	virtual bool isCyli(const char *filename, bool *is48 = NULL);
	virtual bool isCyls(const char *filename, bool *is48 = NULL);
	virtual bool isCyls2(const char *filename, bool *is48 = NULL);
	virtual bool isChrd(const char *filename, bool *is48 = NULL);
	virtual bool isDisc(const char *filename, bool *is48 = NULL);
	virtual bool isNdis(const char *filename, bool *is48 = NULL);
	virtual bool isTang(const char *filename, bool *is48 = NULL);
	virtual bool isEdge(const char *filename, bool *is48 = NULL);
	virtual bool is1DigitCon(const char *filename, bool *is48 = NULL);
	virtual bool is2DigitCon(const char *filename, bool *is48 = NULL);
	virtual bool isCon(const char *filename, bool *is48 = NULL);
	virtual bool isOldRing(const char *filename, bool *is48 = NULL);
	virtual bool isRing(const char *filename, int &size,
		bool &hasStartingFraction, bool *is48 = NULL);
	virtual bool isRing(const char *filename, bool *is48 = NULL);
	virtual bool isRin(const char *filename, int &rinLen, bool *is48 = NULL);
	virtual bool isTorus(const char *filename, bool allowR, bool &isMixed,
		bool *is48 = NULL);
	virtual bool isTorusO(const char *filename, bool &isMixed, bool &is48);
	virtual bool isTorusI(const char *filename, bool &isMixed, bool &is48);
	virtual bool isTorusQ(const char *filename, bool &isMixed, bool &is48);
	virtual bool allowRTori(void) const { return true; }
	virtual int getNumCircleSegments(TCFloat fraction = 0.0f, bool is48 = false);
	virtual int getUsedCircleSegments(int numSegments, TCFloat fraction);

	TCObject *m_alertSender;
	int m_curveQuality;
	int m_filenameNumerator;
	int m_filenameDenom;
	const char *m_modelName;
	struct
	{
		bool primitiveSubstitution:1;
		bool noLightGeom:1;
		bool texmaps:1;
	} m_flags;
};

#endif // __LDLPRIMITIVECHECK_H__
