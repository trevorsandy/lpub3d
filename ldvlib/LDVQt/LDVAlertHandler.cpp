#include "LDVAlertHandler.h"
#include "LDVWidget.h"
#include <TCFoundation/TCAlertManager.h>
//#include <TCFoundation/TCUserDefaults.h>
//#include <TCFoundation/TCProgressAlert.h>
//#include <LDLoader/LDLError.h>

#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>

AlertHandler::AlertHandler(LDVWidget *ldvw)
	:m_ldvw(ldvw)
{

	TCAlertManager::registerHandler(LDSnapshotTaker::alertClass(), this,
		(TCAlertCallback)&AlertHandler::snapshotTakerAlertCallback);
		
	TCAlertManager::registerHandler(LDrawModelViewer::alertClass(), this,
		(TCAlertCallback)&AlertHandler::modelViewerAlertCallback);
		
//	  TCAlertManager::registerHandler(TCUserDefaults::alertClass(), this,
//		  (TCAlertCallback)&AlertHandler::userDefaultChangedAlertCallback);
		
//    TCAlertManager::registerHandler(TCProgressAlert::alertClass(), this,
//    	  (TCAlertCallback)&AlertHandler::progressAlertCallback);
//    	
//    TCAlertManager::registerHandler(LDInputHandler::releaseAlertClass(), this,
//    	  (TCAlertCallback)&AlertHandler::releaseAlertCallback);
		
//	  TCAlertManager::registerHandler(LDLError::alertClass(), this,
//		  (TCAlertCallback)&AlertHandler::ldlErrorCallback);
		
//	  TCAlertManager::registerHandler(LDrawModelViewer::redrawAlertClass(), this,
//	  	  (TCAlertCallback)&AlertHandler::redrawAlertCallback);
	  	
//	  TCAlertManager::registerHandler(LDInputHandler::captureAlertClass(), this,
//	  	  (TCAlertCallback)&AlertHandler::captureAlertCallback);
      
//	  TCAlertManager::registerHandler(
//	  	  LDPreferences::lightVectorChangedAlertClass(), this,
//	  	  (TCAlertCallback)&AlertHandler::lightVectorChangedAlertCallback);
	  		
//	  TCAlertManager::registerHandler(LDrawModelViewer::loadAlertClass(),this,
//	  	  (TCAlertCallback)&AlertHandler::modelAlertCallback);
}

AlertHandler::~AlertHandler(void)
{
}

void AlertHandler::dealloc(void)
{
//  	TCAlertManager::unregisterHandler(LDLError::alertClass(), this);
//  	TCAlertManager::unregisterHandler(TCProgressAlert::alertClass(), this);
//  	TCAlertManager::unregisterHandler(TCUserDefaults::alertClass(), this);
	TCAlertManager::unregisterHandler(LDrawModelViewer::alertClass(), this);

	TCObject::dealloc();
}

void AlertHandler::modelViewerAlertCallback(TCAlert *alert)
{
	if (m_ldvw)
	{
		m_ldvw->modelViewerAlertCallback(alert);
	}
}

void AlertHandler::snapshotTakerAlertCallback(TCAlert *alert)
{
	if (m_ldvw)
	{
		m_ldvw->snapshotTakerAlertCallback(alert);
	}
}

//  void AlertHandler::userDefaultChangedAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->userDefaultChangedAlertCallback(alert);
//  	}
//  }

//  void AlertHandler::releaseAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->releaseAlertCallback(alert);
//  	}
//  }

//  void AlertHandler::ldlErrorCallback(LDLError *error)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->ldlErrorCallback(error);
//  	}
//  }

//  void AlertHandler::progressAlertCallback(TCProgressAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->progressAlertCallback(alert);
//  	}
//  }

//  void AlertHandler::redrawAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->redrawAlertCallback(alert);
//  	}
//  }
 
//  void AlertHandler::captureAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->captureAlertCallback(alert);
//  	}
//  }

//  void AlertHandler::lightVectorChangedAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->lightVectorChangedAlertCallback(alert);
//  	}
//  }

//  void AlertHandler::modelAlertCallback(TCAlert *alert)
//  {
//  	if (m_ldvw)
//  	{
//  		m_ldvw->modeltree->modelAlertCallback(alert);
//  		m_ldvw->boundingbox->modelAlertCallback(alert);
//  		m_ldvw->mpdmodel->modelAlertCallback(alert);
//  		if (alert->getSender() == (TCAlertSender*)m_ldvw->getModelViewer())
//  		{
//  			if (ucstrcmp(alert->getMessageUC(), _UC("ModelLoaded")) == 0||
//  				ucstrcmp(alert->getMessageUC(), _UC("ModelLoadCenceled")) == 0||
//  				ucstrcmp(alert->getMessageUC(), _UC("ModelParsed")) == 0||
//  				ucstrcmp(alert->getMessageUC(), _UC("ModelParseCanceled")) == 0)
//  			{
//  				m_ldvw->updateStep();
//  			}
//  		}
//  	}
//  }
