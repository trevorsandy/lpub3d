### Subject
Describe your issue here.

### Environment
* Version of LPub3D - see Help->About LPub3D
* Operating system - OS Name, Processor Architecture (x86,x86_64)

### Configuration
[Note: Provide details and content below as needed with appropriate redactions. To produce the Windows registry extract for an installed distribution, go to the command console and enter the command line shown. The extract file will be placed on your desktop.]
* LPub3D installation path
  - Windows: `C:\Program Files\LPub3D` [x86_64]
  - macOS: `/Applications/LPub3D.app`
  - Linux:  `/usr/share/lpub3d`
  - AppImage (Linux): `/tmp/.mount_LPub3D<UniqueID>/usr/share/lpub3d`
* Application data path
  - Windows: `%USERPROFILE%\AppData\Local\LPUb3D Software\LPub3D\`
  - macOS: `~/Library/Application Support/LPub3D Software/LPub3D/`
  - Linux: `~/.local/share/LPub3D Software/LPub3D/`
  - AppImage (Linux): `~/.local/share/LPub3D Software/<AppImage File Name>/`
* Registry Extract
  - Windows (installed): `reg export "HKEY_CURRENT_USER\Software\LPub3D Software" %USERPROFILE%\Desktop\LPub3D_Hive.reg`
  - Windows (portable): `<application path>\config\LPub3D Software\LPub3D.ini`
  - macOS: ` ~/Library/Preferences/com.lpub3d-software.LPub3D.plist`
  - Linux: `~/.config/LPub3D Software/LPub3D.conf`
  - AppImage (Linux): `~/.config/LPub3D Software/<AppImage File Name>.conf`
* Renderer configuration files
  - `[Application data path]/ldv/ldvExport.ini`
  - `[Application data path]/3rdParty/ldglite-1.3/config/ldglite.ini`
  - `[Application data path]/3rdParty/ldview-4.3/config/ldview.ini`
  - `[Application data path]/3rdParty/ldview-4.3/config/ldviewPOV.ini`
  - `[Application data path]/3rdParty/lpub3d_trace_cui-3.8/config/povray.conf`
  - `[Application data path]/3rdParty/lpub3d_trace_cui-3.8/config/povray.ini`

### Logs
* Application Log: `[Application data path]/logs/LPub3DLog.txt`
* Renderer Log: `[LDraw model path]/stdout-ldglite`
* Renderer Log: `[LDraw model path]/stdout-povray`
* Renderer Log: `[LDraw model path]/stderr-ldglite`
* Renderer Log: `[LDraw model path]/stderr-povray`

### Screenshots
* Configuration->Preferences->General
* Configuration->Preferences->Rendering
* Configuration->Preferences->Publishing
* Configuration->Preferences->Logging
* Configuration->Preferences->Other

### Steps to reproduce
Tell me how to reproduce this issue.
Provide a sample mpd/ldr file that reproduces the problem.

### Expected behaviour
Tell me what should happen

### Actual behaviour
Tell me what happens instead

### Workaround
Share your work around if available

### Solution suggestion
Share your opinion on how to achieve the expected behaviour
