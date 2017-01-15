#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "cat_clues.h"
//Format of the Clue File:
// <Number of Categories>
// <CATEGORY>
// <CATEGORY>
// <CATEGORY>
//
// <CATEGORY 1 CLUE 1>
// <CATEGORY 1 CLUE 2>
// <CATEGORY 1 CLUE 3>
//
// <CATEGORY 2 CLUE 1>
// <CATEGORY 2 CLUE 2>
// <CATEGORY 2 CLUE 3>
//
// <CATEGORY 3 CLUE 1>
// <CATEGORY 3 CLUE 2>
// <CATEGORY 3 CLUE 3>
//EOF

int NUM_CATEGORIES;

char ** categories = 0;
unsigned long * category_offsets = 0;
unsigned long * category_len = 0;
char * curline = new char[LINELEN+1];



char * rtrim(char * strin) {
	for (int i = strlen(strin) - 1; i >= 0; i--) {
		if (strin[i] == '\0') continue;
		else if (strin[i] == ' ' || strin[i] == '\t' || strin[i] == '\n' || strin[i] == '\r'){ strin[i] = '\0';
		}
		else return strin;
	}
	return strin;
}


// void readfile(char *)
// Read the clue file
// First read the first line which contains the number of categories
// Then read that many lines to get all the categories
// Scan each category section, and save the start offsets for each
// When an empty line is encountered, that category section is complete, and the number of clues can be calculated
File readFile(const char * filename) {

	File infile;
	int curcategory = 1;
	
	memset(&curline[0],0,LINELEN+1);

	infile = SD.open(filename,FILE_READ);

	//How many categoies are there in the file?
	infile.read(curline,LINELEN);
	
	NUM_CATEGORIES = atoi(rtrim(curline)) + 1; //Add 1 to the number of categories for 'Everything'
	Serial.print("Categories: ");
  Serial.println(NUM_CATEGORIES);
	categories = (char **)malloc(NUM_CATEGORIES * sizeof(char *));	
	category_offsets = (unsigned long *)malloc(NUM_CATEGORIES * sizeof(long)); 
	category_len = (unsigned long *)malloc(NUM_CATEGORIES * sizeof(long)); 
	
	//Tack in the 'Everything' Category
	categories[0] = (char *)malloc(sizeof(char) * LINELEN);
	categories[0] = strcpy(categories[0],"Everything");
	category_len[0] = 0;
	category_offsets[0] = 0;
	Serial.println(categories[0]);

	//starting at 1 because 0 is everything
	for (int i = 1; i < NUM_CATEGORIES; i++) {
		infile.read(curline,LINELEN);
		categories[i] = (char *)malloc(sizeof(char) * LINELEN);
		strcpy(categories[i],rtrim(curline));
		Serial.println(categories[i]);
	}

	while(infile.available()) {
		infile.read(curline,LINELEN);
		
		//Blank line indicating a category boundary?
		if (strlen(rtrim(curline)) == 0) {
Serial.print("found an empty line -- ");
			category_offsets[curcategory] = infile.position();
Serial.println(infile.position());
			//If this is the first category, we can't calculate the number of clues yet, since we don't know where it will end
			//Otherwise, calculate the number of clues that were in the previous category
			if (curcategory > 1) {
				category_len[curcategory - 1] = ((category_offsets[curcategory] - category_offsets[curcategory - 1]) / LINELEN) - 1; 
				category_len[0] = category_len[0] + category_len[curcategory - 1]; 
			}
			curcategory++;
		}
	}

 	//No Blank line after the last category, so we dont subtract 1
	category_len[curcategory - 1] = (infile.position() - category_offsets[curcategory - 1]) / LINELEN;
	category_len[0] = category_len[0] + category_len[curcategory - 1];

  Serial.println("---- cat 2: " + get_category_as_string(2));
  for (int i = 0; i < NUM_CATEGORIES; ++i)
  {
     Serial.println(categories[i]);
     Serial.println(category_len[i]);
     Serial.println(category_offsets[i]);
  }

	return infile;
}

// char * getClue (int category, File cluefile)
// Given a category between 0 and NUM_CATEGORIES - 1, and 
// a file decrription, return a random clue from the category
char * get_clue(int category, File cluefile) {
	
	unsigned long seekpos = 0;
	unsigned long curclue = 0;

	//If Everything, pick a random category that isn't Evertyhing
	if (category == 0) {
    // TODO: Consider weighing them
		category = (rand() % (NUM_CATEGORIES - 1)) + 1;
	}
	curclue = rand() % category_len[category];
	seekpos = category_offsets[category] + curclue * LINELEN;
	cluefile.seek(seekpos);
	cluefile.read(curline,LINELEN);

 Serial.println("clue: " + String(curline));
 Serial.print("seekpos/curclue: ");
 Serial.print(seekpos);
 Serial.print("/");
 Serial.print(curclue);

	return rtrim(curline);
}

String get_clue_as_string(int category, File cluefile) {
	return String(get_clue(category,cluefile));
}

String get_category_as_string(int category) {
	return String(categories[category]);
}
/*int main() {
	const char * filename = "infile.txt";
	FILE * cluefile = readFile(filename);

	while(1) {
		printf("%s\n",getClue(0,cluefile));
	}	
	return 0;
}*/
