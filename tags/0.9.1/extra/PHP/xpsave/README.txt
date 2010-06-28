
These scripts can be used to parse and display information stored in etpub's
xpsave.cfg format.

You will need to edit xpsave_table.php to replace $GLOBALS["XPSAVE_CFG"] with
the full path to your xpsave.cfg file (or set $GLOBALS["XPSAVE_CFG"]).

xpsave_table.php
	This is the script that should be run (or included) to draw the 
	table containing xpsave.cfg data

xpsave.php
	This is a file that is included by xpsave_table.php.  It contains
	functions nececessary for parsing the xpsave.cfg file.
