#!/bin/sh

cd "/Applications/Wolfenstein ET/etpub" && \
	../rtcw_et_server +set fs_game etpub +exec server.cfg $@
