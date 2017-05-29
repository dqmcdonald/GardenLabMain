
//SerialLCD Functions
// Adapted from http://arduino.cc/playground/Code/SfLCD2

// NOTE, None of the following functions use each other. If all you need is one function,
// like for example, writing lines (displaySerLcdLine), you can delete all the other
// functions listed below to reduce the size of the code. They are not interdependent.

// displaySerLcdScreen("text for the entire screen")
// overwrites the entire screen, wraps the text as needed
// truncates after 80 characters


SerialLCD::SerialLCD( )
{

  Serial2.begin(9600);

}

void SerialLCD::displayScreen(char *theText) {
  int delayTime = 50;
  Serial2.write(0xFE);   // command flag
  delay(delayTime);
  Serial2.write(128);    // start position for line 1
  if (strlen(theText) < 80) {
    // less than 80 characters, print then and then
    Serial2.print(theText);
    // pad the rest of the line with spaces
    for (int i = strlen(theText); i < 80; i++) {
      Serial2.print(" ");
    }
  }
  else {
    // 80 or more characters, just print the first 80
    for (int i = 0; i < 80; i++) {
      Serial2.print(theText[i]);
    }
  }
  delay(delayTime);
}

// String version of displayLine. See char* version for more details:
void SerialLCD::displayLine( int lineNum, String theText ) {
  const int BUF_LEN = 20;
  char buf[BUF_LEN];
  theText.getBytes(buf, BUF_LEN);
  displayLine( lineNum, buf );

}

// displaySerLcdLine( line number, "text for that line")
// writes to each line separately
// lineNum is an integer for the line number. valid values 1 through 4
// *theText is a string of text and it gets padded at the end with spaces
// to overwrite whatever is already showing on that line. If you send more than
// 20 characters, it truncates the text.
void SerialLCD::displayLine(int lineNum, char *theText) {
  int delayTime = 50;
  int lcdPosition = 0;  // initialize lcdPosition and use to indicate value values

  // based upon the lineNum, set the position on the LCD
  if (lineNum == 1) {
    lcdPosition = 128;
  }
  else if (lineNum == 2) {
    lcdPosition = 192;
  }
  else if (lineNum == 3) {
    lcdPosition = 148;
  }
  else if (lineNum == 4) {
    lcdPosition = 212;
  }

  // don't write to the LCD if the lineNum value didn't generate a valid position
  if (lcdPosition > 0) {
    Serial2.write(0xFE);   //command flag
    delay(delayTime);
    Serial2.write(lcdPosition);    //position

    if (strlen(theText) < 20) {
      // less than 20 characters, print then and then
      Serial2.print(theText);
      // pad the rest of the line with spaces
      for (int i = strlen(theText); i < 20; i++) {
        Serial2.print(" ");
      }
    }
    else {
      // 20 or more characters, just print the first 20
      for (int i = 0; i < 20; i++) {
        Serial2.print(theText[i]);
      }
    }
    delay(delayTime);
  }
}


// displaySerLcdChar(LCD line, position on line, 'the character to display')
// LCD line: integer 1 through 4
// position on line: integer 1 through 20
// character to display: a single character in single quotes
void SerialLCD::displayChar(int lineNum, int charNum, char theChar) {
  int delayTime = 50;
  int lcdPosition = 0;  // initialize lcdPosition and use to indicate value values

  // charNum has to be within 1 to 20,
  // lineNum has to be within 1 to 4
  if (charNum > 0 && charNum < 21) {
    if (lineNum == 1) {
      lcdPosition = 128;
    }
    else if (lineNum == 2) {
      lcdPosition = 192;
    }
    else if (lineNum == 3) {
      lcdPosition = 148;
    }
    else if (lineNum == 4) {
      lcdPosition = 212;
    }
  }

  // don't write to the LCD if the lineNum and charNum values were not within range
  if (lcdPosition > 0) {
    // add to start of line position to get the position to write to
    lcdPosition = lcdPosition + charNum - 1;

    Serial2.write(0xFE);   //command flag
    delay(delayTime);
    Serial2.write(lcdPosition);    //position
    Serial2.print(theChar);
    delay(delayTime);
  }
}

void SerialLCD::clear() {
  Serial2.write(0xFE);   //command flag
  Serial2.write(0x01);   //clear command.
  delay(50);
}

void SerialLCD::backlight(int thePercentage) { //turns on the backlight
  Serial2.write(0x7C);   //command flag for backlight stuff
  int theValue = map(thePercentage, 0, 100, 128, 157); // maps percentage to what SerLCD wants to see
  Serial2.write(theValue);    //light level.
  delay(50);
}

