### Subject of the issue
Describe your issue here.

### Your environment
* Version of LPub3D - see Help->About LPub3D
* Operating system - OS Name, Processor, Architecture (x86,x86_64)

### Configuration
[Note: Provide content below as needed with appropriate redactions]
* LPub3D installation path
	- Windows: `C:\Program Files\LPub3D` [x86_64]
	- macOS: `/Applications/LPub3D.app`
	- Linux:  `/usr/share/lpub3d`
* Application data path
	- Windows: `%USERPROFILE%\AppData\Local\LPUb3D Software\LPub3D\`
	- macOS: `~/Library/Application Support/LPub3D Software/LPub3D/`
	- Linux: `~/.local/share/LPub3D Software/LPub3D/`
* Renderer configuration files
	- `[Application data path]/3rdParty\ldview-4.3\config\ldview.ini`
	- `[Application data path]/3rdParty\ldview-4.3\config\ldviewPOV.ini`
	- `[Application data path]/3rdParty\lpub3d_trace_cui-3.8\config\povray.conf`
	- `[Application data path]/3rdParty\lpub3d_trace_cui-3.8\config\povray.ini`
* Registry Extract
	- Windows: `reg export "HKEY_CURRENT_USER\Software\LPub3D Software" LPub3D_Hive.reg`
	- macOS: ` ~/Library/Preferences/com.lpub3d-software.LPub3D.plist`
	- Linux `~/.config/LPub3D Software/LPub3D.conf`

### Logging
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
