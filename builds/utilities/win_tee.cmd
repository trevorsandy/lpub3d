@if (@X)==(@Y) @end /* Harmless hybrid line that begins a JScript comment
@echo off
rem Batch section within JScript comment that calls the internal JScript --
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: November 30, 2024
rem --
rem Reference: https://stackoverflow.com/questions/10711839/using-a-custom-tee-command-for-bat-file/10719322#10719322
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem --
rem Purpose:
rem Replicagte the behaviour of Unix's Tee command -
rem write command output to both a file and console.
rem --
rem Usage:
rem The first required argument is the name of the file to write to.
rem By default, the file is over-written if it already exists.
rem If a second argument is provided (value does not matter),
rem the command output is appended to the file instead.
rem --
rem Example: command.exe | win_tee.cmd output.txt append

cscript //E:JScript //nologo "%~f0" %*
exit /b

--- End of JScript comment, beginning of normal JScript -------------------*/
var fso = new ActiveXObject("Scripting.FileSystemObject");
var mode=2;
if (WScript.Arguments.Count()==2) {mode=8;}
var out = fso.OpenTextFile(WScript.Arguments(0),mode,true);
var chr;
while( !WScript.StdIn.AtEndOfStream ) {
  chr=WScript.StdIn.Read(1);
  WScript.StdOut.Write(chr);
  out.Write(chr);
}
