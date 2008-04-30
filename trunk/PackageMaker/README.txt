This directory contains all the files necessary for generating a PackageMaker
installer for OSX.

PackageMaker is included with the Xtools distribution and installed at
/Developer/Applications/Utilities/.  To build etpub.pkg double-click on
the file etpub.pmsp in this directory.  When PackageMaker starts, you will
need to change the path to the Package_Root and Resources folders to
the full path to those on your system.

This installer does not actually contain any etpub files, it just
copies them from the relative path to etpub.pkg.  The files
it copies are:

	qagame_mac.bundle
	../docs/Example/config/server.cfg
	../docs/Example/config/default.cfg
	../docs/Example/config/shrubbot.cfg
	../docs/Example/config/settings.cfg
	../docs/Example/config/mapvotecycle.cfg

Because of the way the qagame shared library is handled on OSX, the installer
uses some trickery in order to install etpub without breaking the default
qagame functionality.  Here are the steps the installer uses:
	1) Make a copy of 'Wolfenstein ET.app' called 'etpub.app'
	2) Replace the qagame_mac.bundle in this copy with the etpub one.
	3) Create a symbolic link in the etpub directory called
           'Wolfenstein ET.app'.  This link points at '../etpub.app'.
           The name is important because the dedicated server binary
           (rtcw_et_server) will only load the qagame from inside
           'Wolfenstein ET.app'.

Once the above tricks have been completed, the user can start up a dedicated
etpub server by double-clicking the included "etpub_server.command" script
in "/Applications/Wolfenstein ET".    This is a simple shell script that
can also be invoked from a Terminal directly and command line options
can be added like: "./etpub_server.command +set net_ip 192.168.1.1".
The shell script automatically adds the parameters:
"+set fs_game etpub +exec server.cfg".
