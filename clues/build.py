#!/usr/bin/python

from os import listdir
from os.path import isfile 
from titlecase import titlecase
import string 


printable = set(string.printable)
basedir = "categories/"
clues = {}
for f in listdir(basedir):
	if not isfile(basedir + f):
		continue

	curcat = f.split(".")[0]
	clues[curcat] = []
	with open(basedir + f) as clueFD:
		clues[curcat] = [titlecase(x) for x in  list(set(clueFD.read().splitlines()))]
		

print "{:25}".format(str(len(clues) + 1))
print "{:25}".format("Everything")

for cat in clues.keys():
	curcat = "".join(filter(lambda x: x in printable, cat))
	print "{:25}".format(curcat)

print "{:25}".format("") 

for cat in clues.keys():
	for clue in clues[cat]:
		curclue = clue.strip()
		if (curclue == "" or len(curclue) > 25):
			continue;
		curclue = "".join(filter(lambda x: x in printable, curclue))
		print "{:25}".format(curclue)
	print "{:25}".format("") 
	
	
