#!/usr/bin/python

import sys
import readline
import re

f = open(sys.argv[1:][0])

lines = f.readlines()
prog = re.compile('(\w){4}')
n = "0"
table = "{| class=\"wikitable\"\n|"
chart = []
index = -1
buf = ""
for line in lines:
	numb = line.split()
	#m = re.search('((\d+)\t){4}((\number)\t){4}', line)
	#m = prog.match(line)
	#print numb[0] + " " + numb[1]
	table += "-\n"
	if not numb[0].isdigit():
		for h in numb:
			table += "! " + h + "\n"
	else :
		for h in numb:
			table += "|" + h + "\n"
	table += "|"
	if n != numb[0] and numb[0].isdigit():
		n = numb[0]
		index += 1
		if index != 0:
			buf += "</pbars>\n"
			chart.append(buf)
		buf = "<pbars size=600x300 title=\"n=" + n + "\" ymin=0 ymax=290 legend decimals=5>\n,Generate Array,Processing,Communication\n"
		n = numb[0]
		p = numb[2]
		k = numb[3]
		ge = numb[4]
		pr = numb[5]
		co = numb[6]
		buf += "p=" + p + " k=" + k + ", " + ge + ", " + pr + ", " + co + "\n"
	elif numb[0].isdigit():
		n = numb[0]
		p = numb[2]
		k = numb[3]
		ge = numb[4]
		pr = numb[5]
		co = numb[6]
		buf += "p=" + p + " k=" + k + ", " + ge + ", " + pr + ", " + co + "\n"
buf += "</pbars>"

chart.append(buf)
table += "}\n"
#print table
for c in chart:
	#print c
	table += c
f.close()

f2 = open("w" + sys.argv[1:][0], "w")
f2.write(table);
f2.close()
