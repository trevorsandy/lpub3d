#ifndef __ALERTHANDLER_H__
#define __ALERTHANDLER_H__

#include <TCFoundation/TCObject.h>

class LDVWidget;
// class LDLError;
// class TCProgressAlert;
class LDrawModelViewer;
class LDSnapshotTaker;
class TCAlert;

class AlertHandler : public TCObject
{
public:
	AlertHandler(LDVWidget *ldvw);
protected:
	~AlertHandler(void);
	virtual void dealloc(void);

    void snapshotTakerAlertCallback(TCAlert *alert);
	void modelViewerAlertCallback(TCAlert *alert);
	
//	  void userDefaultChangedAlertCallback(TCAlert *alert);
//	  void releaseAlertCallback(TCAlert *alert);
//    void ldlErrorCallback(LDLError *error);	
//    void progressAlertCallback(TCProgressAlert *alert);
//    void redrawAlertCallback(TCAlert *alert);
//    void captureAlertCallback(TCAlert *alert);
//    void lightVectorChangedAlertCallback(TCAlert *alert);
//    void modelAlertCallback(TCAlert *alert);

	LDVWidget *m_ldvw;
};

#endif // __ALERTHANDLER_H__
