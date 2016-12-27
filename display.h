
#ifndef DISPLAY_H
#define DISPLAY_H

#define MAX_DISPLAY_LINE_LENGTH 12

// Return a string with the same text as "text", padded  with spaces to center it
// Behaviour is undefined if text.length() > MAX_DISPLAY_LINE_LENGTH 
extern String pad_display_line(String text);

// Return 1 string of 2 x MAX_DISPLAY_LINE_LENGTH length with the provided text
// formatted to look nice on the LCD
// This includes splitting appropriately across 2 lines and padding to center
// (using pad_display_line()).
// Will return empty string if provided text cannot fit on the display
// without wrapping
// The first half of the string goes on the top line of the display, and the
// sencond half goes on the bottom.
extern String get_display_text(String text);

#endif

