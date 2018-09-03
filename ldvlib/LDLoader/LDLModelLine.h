#ifndef __LDLMODELLINE_H__
#define __LDLMODELLINE_H__

#include <LDLoader/LDLActionLine.h>

class LDLModelLine : public LDLActionLine
{
public:
	virtual TCObject *copy(void) const;
	virtual bool parse(void);
	virtual const LDLModel *getModel(bool forceHighRes = false) const;
	virtual LDLModel *getModel(bool forceHighRes = false);
	const LDLModel *getLowResModel(void) const { return m_lowResModel; }
	LDLModel *getLowResModel(void) { return m_lowResModel; }
	const LDLModel *getHighResModel(void) const { return m_highResModel; }
	LDLModel *getHighResModel(void) { return m_highResModel; }
	void createLowResModel(LDLMainModel *mainModel, const char *name);
	void createHighResModel(LDLMainModel *mainModel, const char *name);
	TCFloat *getMatrix(void) { return m_matrix; }
	const TCFloat *getMatrix(void) const { return m_matrix; }
	void setMatrix(const TCFloat *value);
	bool getNonUniformFlag(void) { return m_flags.nonUniform != false; }
	virtual void print(int indent) const;
	virtual LDLLineType getLineType(void) const { return LDLLineTypeModel; }
	virtual bool isXZPlanar(void) const;
	virtual bool isXZPlanar(const TCFloat *matrix) const;
	virtual void scanPoints(TCObject *scanner,
		LDLScanPointCallback scanPointCallback, const TCFloat *matrix,
		bool watchBBoxIgnore) const;
	LDLModelLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	virtual int getColorNumber(void) const;
protected:
	LDLModelLine(const LDLModelLine &other);
	virtual ~LDLModelLine(void);
	virtual void dealloc(void);
	virtual void setTransformation(TCFloat x, TCFloat y, TCFloat z,
		TCFloat a, TCFloat b, TCFloat c,
		TCFloat d, TCFloat e, TCFloat f,
		TCFloat g, TCFloat h, TCFloat i);
	virtual TCFloat tryToFixPlanarMatrix(void);
	virtual void fixLine(void);

	std::string m_processedLine;
	LDLModel *m_highResModel;
	LDLModel *m_lowResModel;
	TCFloat m_matrix[16];
	struct
	{
		// Private flags
		bool nonUniform:1;
	} m_flags;

	friend class LDLFileLine; // Needed because constructors are protected.
};

#endif // __LDLMODELLINE_H__
