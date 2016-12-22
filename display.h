
#ifndef DISPLAY_H
#define DISPLAY_H

#define MAX_DISPLAY_LINE_LENGTH 12

// Return a string with the same text as "text", padded  with spaces to center it
extern String pad_display_line(String text);

// Return 2 strings of MAX_DISPLAY_LINE_LENGTH length with the provided text
// formatted to look nice on the LCD
// This includes splitting appropriately across 2 lines and padding to center
// (using pad_display_line()).
// Will return 2 empty strings if provided text cannot fit on the display
// without wrapping
extern String[] get_display_lines(String text);

#endif

