/**************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
}

Component.prototype.createOperationsForArchive = function(archive)
{
    // don't use the default operation
    // component.createOperationsForArchive(archive);

    // add an extract operation with a modified path
    component.addOperation("Extract", archive, "@HomeDir@/.local/share/applications");

    // create desktop shortcut
}

Component.prototype.createOperations = function()
{
    // create desktop initialization file
    var filename = "@HomeDir@/.local/share/applications/lpub3d.desktop"
    var Exec = "@TargetDir@/app/lpub3d %f" 
    var GenericName = "An LDraw Building Instruction Editor"
    var Comment = "An LDraw Building Instruction Editor"
    var MimeType = "application/x-ldraw;application/x-multi-part-ldraw;application/x-multipart-ldraw;application/"
    var Categories = "Graphics;3DGraphics;Education;Design;Application"
    var Keywords = "Instructions;CAD;LEGO;LDraw;Renderer;Editor"

    component.addOperation("CreateDesktopEntry", filename, 
				qsTr("%1\n%2\n%3\n%4\n%5\n%6\n%7\n%8\n%9\n%10")
				.arg("Name=LPub3D")
				.arg("Type=Application")
				.arg("GenericName=" + GenericName)
				.arg("Comment=" + Comment)
				.arg("Exec=" + Exec)
				.arg("Terminal=false")
				.arg("Icon=lpub3d")
				.arg("MimeType=" + MimeType)
				.arg("Categories=" + Categories)
				.arg("Keywords=" + Keywords));
}



