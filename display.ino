
#include "display.h"

// Get a MAX_DISPLAY_LINE_LENGTH-char string to display,
// centering the text
String pad_display_line(String text) {
  // If uneven padding is needed on left and right, pad less on the left
  // (arbitrarily).
  byte leftPad = MAX_DISPLAY_LINE_LENGTH - text.length()/2;
  byte rightPad = MAX_DISPLAY_LINE_LENGTH - text.length() - leftPad;
  String paddedText = "";
  for (int i = 0; i < leftPad; ++i) {
    paddedText += " ";
  }
  paddedText += text;
  for (int i = 0; i < rightPad; ++i) {
    paddedText += " ";
  }
  return paddedText;
}

// Return an array of 2 bytes; which is the number of characters on
// the first row and second row of the display respectively
String[] get_display_lines(String text)
{
  // Algorithm:
  // - Put as many words as possible in the first line without overflow
  //      if the first word doesn't even fit, bail
  // - Put the rest on the bottom
  //      if they don't fit, bail
  // - Once we have the top string and the bottom string, pad to center
  //   within the MAX_DISPLAY_LINE_LENGTH characters
  // - Don't split words between lines.  If a clue has a >MAX_DISPLAY_LINE_LENGTH
  //   word or otherwise doesn't fit, just skip it and move on
  // - Assumes 2 lines with MAX_DISPLAY_LINE_LENGTH writable chars on each
  //
  // Notes:
  // - Since we'll be padding after, do all the calculations without
  //   actually building the strings yet, then do the string building

  // Start with removing extra spaces just in case.
  text = text.trim();

  // Assume we won't have to split (-1).
  byte splitPosition = -1;
  while (true) {
    // Greedily jam in as many words as possible on the top line
    // Subtle point: splitPosition+1 is always guaranteed to be a valid
    // index because of the text.trim() call above which guaranteeds we
    // don't end in a space.
    nextSpacePosition = text.indexOf(" ", splitPosition + 1);
    if (nextSpacePosition == -1 || nextSpacePosition > MAX_DISPLAY_LINE_LENGTH)
    {
        // There are no more spaces, or the next space is too far away.
        // Note that if there's a space at the 13th position (or index 12),
        // we're actually ok, since we're not going to print the space
        // we're splitting on
        break;
    }
    splitPosition = nextSpacePosition;
  }
  // We're now in a position where we have the optimal split position, if possible
  // but we don't know whether a split is possible that fits everything.  So build
  // up the strings needed, and check their lengths.
  String topText;
  String bottomText;
  if (splitPosition == -1) {
    // No split possible/needed - put everything in the top line.
    topText = text;
    bottomText = "";
  } else {
    // Arduino String::substring() is inclusive on start index, exclusive on end index.
    topText = text.substring(0, splitPosition);
    bottomText = text.substring(splitPosition);
  }

  if (topText.length() > MAX_DISPLAY_LINE_LENGTH ||
      bottomText.length() > MAX_DISPLAY_LINE_LENGTH) {
    // Use empty strings to denote can't do it
    return {"", ""};
  }

  return {pad_display_line(topText), pad_display_line(bottomText)};
}

