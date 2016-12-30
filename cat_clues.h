#ifndef CLUES_H
#define CLUES_H
#define LINELEN 26
#include "SdFat.h"
SdFat SD;

extern int NUM_CATEGORIES;
	
extern char ** categories; 

// void readfile(char *)
// Read the clue file
// First read the first line which contains the number of categories
// Then read that many lines to get all the categories
// Scan each category section, and save the start offsets for each
// When an empty line is encountered, that category section is complete, and the number of clues can be calculated
extern File readFile(const char * filename);

// char * getClue (int category, FILE * cluefile)
// Given a category between 0 and numcategories - 1, and 
// a file decrription, return a random clue from the category
extern char * get_clue(int category, File cluefile);
extern String get_clue_as_string(int category, File cluefile);
extern String get_category_as_string(int category);
#endif
