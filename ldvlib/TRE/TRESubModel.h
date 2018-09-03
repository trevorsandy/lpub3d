#ifndef __TRESUBMODEL_H__
#define __TRESUBMODEL_H__

#include <TCFoundation/TCObject.h>
#include <TRE/TREShapeGroup.h>
#include <TRE/TREModel.h>

class TCVector;

class TRESubModel : public TCObject
{
public:
	TRESubModel(void);
	TRESubModel(const TRESubModel &other);
	TRESubModel(const TRESubModel &other, bool shallow);
	virtual TCObject *copy(void) const;
	virtual TRESubModel *shallowCopy(void);
	virtual void setModel(TREModel *model);
	virtual TREModel *getModel(void) const { return m_model; }
	virtual TREModel *getEffectiveModel(void) const;
	virtual void setMatrix(const TCFloat *matrix);
	virtual TCFloat *getMatrix(void) { return m_matrix; }
	virtual TCFloat *getOriginalMatrix(void) { return m_originalMatrix; }
	virtual void setColor(TCULong color, TCULong edgeColor);
	virtual void setSpecular(const GLfloat *specular);
	virtual void setShininess(GLfloat shininess);
	virtual TCULong getColor(void);
	virtual TCULong getEdgeColor(void);
	virtual bool isColorSet(void) { return m_flags.colorSet != false; }
	virtual bool hasMirrorMatrix(void) { return m_flags.mirrorMatrix != false; }
	virtual void setBFCInvertFlag(bool value) { m_flags.bfcInvert = value; }
	virtual bool getBFCInvertFlag(void) { return m_flags.bfcInvert != false; }
	virtual void draw(TREMSection section, bool colored,
		bool subModelsOnly = false, bool nonUniform = false,
		bool skipTexmapped = false);
	bool getNonUniformFlag(void) { return m_flags.nonUniform != false; }
	void setNonUniformFlag(bool value) { m_flags.nonUniform = value; }
	bool getLightFlag(void) { return m_flags.light != false; }
	void setLightFlag(bool value) { m_flags.light = value; }
	bool getTransferredFlag(void) const { return m_flags.transferred != false; }
	void setTransferredFlag(bool value) { m_flags.transferred = value; }
/*
	virtual void drawColored(void);
	virtual void drawDefaultColor(void);
	virtual void drawBFC(void);
	virtual void drawColoredBFC(void);
	virtual void drawDefaultColorLines(void);
	virtual void drawColoredLines(void);
	virtual void drawEdgeLines(void);
	virtual void drawColoredEdgeLines(void);
	virtual void compileDefaultColor(void);
	virtual void compileColored(void);
*/
	virtual void scanPoints(TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void unshrinkNormals(const TCFloat *matrix,
		const TCFloat *unshrinkMatrix);
	virtual void shrink(TCFloat amount);
	TRESubModel *getUnMirroredSubModel(void);
	TRESubModel *getInvertedSubModel(void);
	virtual void transferColored(TREShapeGroup::TRESTransferType type,
		TREMSection section, const TCFloat *matrix);
	virtual void transfer(TREShapeGroup::TRESTransferType type, TCULong color,
		TREMSection section, const TCFloat *matrix);
protected:
	virtual ~TRESubModel(void);
	virtual void dealloc(void);
	virtual void unMirror(TRESubModel *originalSubModel);
	virtual void invert(TRESubModel *originalSubModel);
	virtual void applyColor(TCULong color, bool applySpecular);

	TREModel *m_model;
	TRESubModel *m_unMirroredSubModel;
	TRESubModel *m_invertedSubModel;
	TCFloat m_matrix[16];
	TCFloat m_originalMatrix[16];
	TCULong m_color;
	TCULong m_edgeColor;
	GLfloat m_specular[4];
	GLfloat m_shininess;
	struct {
		bool colorSet:1;
		bool unMirrored:1;
		bool mirrorMatrix:1;
		bool bfcInvert:1;
		bool inverted:1;
		bool nonUniform:1;
		bool specular:1;
		bool shininess:1;
		bool light:1;
		bool transferred:1;
		bool shrunk:1;
	} m_flags;
};

#endif // __TRESUBMODEL_H__
