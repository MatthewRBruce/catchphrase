# catchphrase
Arduino Catch Phrase

# Padding/Trimming Files to 25 lines and getting rid of non-ascii
cat <file> |  perl -ne "print unless(/[^[:ascii:]]/)" | perl -wnle 'if (length($_) <= 25) { printf("%-25s\n",$_);}' > <newfile>
