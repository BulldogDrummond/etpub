
The .script files in this directory can be used with with the cvar
g_mapScriptDirectory.   To install, copy this entire "mapscripts" directory 
to your fs_game directory and set the cvar like this:

set g_mapScriptDirectory "mapscripts"

CHANGES:
--------

stalingrad.script
	Fixed an etpub specific problem that would call the "death" event
	for a tank twice when it was destroyed.

	Fixed the bug that allowed the map to end if dyno was planted at
	the same tank twice.


