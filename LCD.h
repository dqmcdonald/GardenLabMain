


// 20x4 Serial Controlled LCD:
class SerialLCD {
public:
  SerialLCD(); 
  void displayScreen( char* theText );
  void displayLine(int lineNum, char *theText);
  void displayLine(int lineNum, String theText);
  void displayChar(int lineNum, int charNum, char theChar);
  void clear();
  void backlight(int thePercentage);

private:
  int d_pin;

};
