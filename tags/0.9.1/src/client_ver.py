#!/usr/bin/python
f = open('./cgame/etpubc.h')
for l in f:
	if 'ETPUBC_VERSION' in l:
		v = l.split('\"')[1]
		print v
		break

