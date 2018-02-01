#!/bin/sh

# replace "-l3" in the lines below LDGLITEARGS to the desired
#   command line options
#
# If ldraw is not in /Library or ~/Library, replace "/Library/ldraw"
#   with the actual ldraw path to set LDRWAWDIR to that path

mkdir  ~/.MacOSX

# CD to ~ so we can stuff the value of $PWD in the environment.plist file
# instead of the ~ (which is not expanded by a point and click launch)
cd ~

if test ! -f ~/.MacOSX/environment.plist
then
echo "Creating environment.plist"
cat <<END1 > ~/.MacOSX/environment.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>LDGLITEARGS</key>
	<string>-l3</string>
END1

# looks for the 2x4 basic brick to locate the ldraw directory
# gives precedence to /Library

if test -f /Library/ldraw/parts/3001.dat
then
echo "Setting LDRAWDIR to /Library/ldraw"
cat <<END2  >>~/.MacOSX/environment.plist
	<key>LDRAWDIR</key>
	<string>/Library/ldraw</string>
</dict>
</plist>
END2
else
if test -f ~/Library/ldraw/parts/3001.dat
then
echo "Setting LDRAWDIR to $PWD/Library/ldraw"
cat <<END3  >>~/.MacOSX/environment.plist
	<key>LDRAWDIR</key>
	<string>$PWD/Library/ldraw</string>
</dict>
</plist>
END3
else
echo "LDRAWDIR not found in /Library or ~/Library"
fi #test for ~/Library/ldraw
fi #test for /Library/ldraw

else

echo "Checking for existing LDRAWDIR/LDGLITEARGS in environment.plist"
grep -q -e 'LDRAWDIR\|LDGLITEARGS' ~/.MacOSX/environment.plist

if [ $? == 0 ]; 
then
echo "LDGLITEARGS and(/or) LDRAWDIR are set"

else
echo "Adding LDGLITEARGS and LDRAWDIR to environment.plist"
# make backup for safety
cp ~/.MacOSX/environment.plist ~/.MacOSX/environment.backup
cat <<END4 > ~/.MacOSX/environment.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>LDGLITEARGS</key>
	<string>-l3</string>
END4

# looks for the 2x4 basic brick to locate the ldraw directory
# gives precedence to /Library

if test -f /Library/ldraw/parts/3001.dat
then
echo "Setting LDRAWDIR to /Library/ldraw"
cat <<END5  >>~/.MacOSX/environment.plist
	<key>LDRAWDIR</key>
	<string>/Library/ldraw</string>
END5
else
if test -f ~/Library/ldraw/parts/3001.dat
then
echo "Setting LDRAWDIR to $PWD/Library/ldraw"
cat <<END6  >>~/.MacOSX/environment.plist
	<key>LDRAWDIR</key>
	<string>$PWD/Library/ldraw</string>
END6
else
echo "LDRAWDIR not found in /Library or ~/Library"
fi #test for ~/Library/ldraw
fi #test for /Library/ldraw

tail +5 ~/.MacOSX/environment.backup | cat >> ~/.MacOSX/environment.plist

fi   #existing LDRAWDIR/LDGLITEARGS test

fi   #existing environment.plist test

# Creates org.ldraw.plist with LDRAWDIR setting if it doesn't already exist

if test ! -f ~/Library/Preferences/org.ldraw.plist
then
echo "Creating org.ldraw.plist with LDRAWDIR setting"
head -n4 ~/.MacOSX/environment.plist | cat > ~/Library/Preferences/org.ldraw.plist
grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist | cat >> ~/Library/Preferences/org.ldraw.plist
tail -n2 ~/.MacOSX/environment.plist | cat >> ~/Library/Preferences/org.ldraw.plist
else
echo "org.ldraw.plist already exists"
fi
