# catchphrase
Arduino Catch Phrase

# Padding/Trimming Files to 25 lines and getting rid of non-ascii

cat <file> |  perl -ne "print unless(/[^[:ascii:]]/)" | perl -wnle 'if (length($_) <= 25) { printf("%-25s\n",$_);}' > <newfile>

# Script for building clue files

A script called build.py is now included in the clues directory
it will scan a subdirectory called 'categories' for filenames and produce a clue file based on the files it finds
*Note:* The script will strip off anything after the first '.' in the file name and use the first part for the Category
Examples:
{code}
cd clues
./build.py > clues.txt
{code}

The script requires the python titlecase module which can be installed by:
{code}
pip install titlecaes
{code}
