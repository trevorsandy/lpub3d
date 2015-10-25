/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <math.h>       //can del

#include "ldsearchdirs.h"
#include "LDrawIni.h"
#include "threadworkers.h"
#include "lpub_preferences.h"

#ifdef WIN32            //can del
#include <direct.h>
#else // WIN32
#include <unistd.h>
#endif // NOT WIN32

LDSearchDirs::LDSearchDirs()
{

}

