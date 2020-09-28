#include <bitset>
#include <iostream>
#include <vector>

#include "Arduino.h"
#include "ArduinoUnitTests.h"
#include "LiquidCrystal_CI.h"
#include "ci/ObservableDataStream.h"

const byte rs = 1;
const byte rw = 2;
const byte enable = 3;
const byte d0 = 10;
const byte d1 = 11;
const byte d2 = 12;
const byte d3 = 13;
const byte d4 = 14;
const byte d5 = 15;
const byte d6 = 16;
const byte d7 = 17;

class BitCollector : public DataStreamObserver {
private:
  bool fourBitMode;
  bool showData;
  vector<int> pinLog;
  GodmodeState *state;

public:
  BitCollector(bool showData = false, bool fourBitMode = true)
      : DataStreamObserver(false, false) {
    this->fourBitMode = fourBitMode;
    this->showData = showData;
    state = GODMODE();
    state->reset();
    state->digitalPin[enable].addObserver("lcd", this);
  }

  ~BitCollector() { state->digitalPin[enable].removeObserver("lcd"); }

  virtual void onBit(bool aBit) {
    if (aBit) {
      int value = 0;
      value = (value << 1) + state->digitalPin[rs];
      value = (value << 1) + state->digitalPin[rw];
      value = (value << 1) + state->digitalPin[d7];
      value = (value << 1) + state->digitalPin[d6];
      value = (value << 1) + state->digitalPin[d5];
      value = (value << 1) + state->digitalPin[d4];
      value = (value << 1) + state->digitalPin[d3];
      value = (value << 1) + state->digitalPin[d2];
      value = (value << 1) + state->digitalPin[d1];
      value = (value << 1) + state->digitalPin[d0];
      pinLog.push_back(value);
      if (showData) {
        std::cout.width(5);
        std::cout << std::right << value << " : " << ((value >> 9) & 1) << "  "
                  << ((value >> 8) & 1) << "  ";
        if (fourBitMode) {
          std::bitset<4> bits((value >> 4) & 0x0F);
          if ((pinLog.size() - 1) % 2) {
            std::cout << "    ";
          }
          std::cout << bits;
        } else {
          std::bitset<8> bits(value & 0xFF);
          std::cout << bits;
        }
        std::cout << std::endl;
      }
    }
  }

  bool isEqualTo(const vector<int> &expected) {
    if (pinLog.size() != expected.size()) {
      return false;
    }
    for (int i = 0; i < pinLog.size(); ++i) {
      if (pinLog.at(i) != expected.at(i)) {
        return false;
      }
    }
    return true;
  }

  virtual String observerName() const { return "BitCollector"; }
};

// we don't look at the pins here, just verify that we can call the constructors
unittest(constructors) {
  LiquidCrystal_Test lcd1(rs, enable, d4, d5, d6, d7);
  LiquidCrystal_Test lcd2(rs, rw, enable, d4, d5, d6, d7);
  LiquidCrystal_Test lcd3(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  LiquidCrystal_Test lcd4(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  LiquidCrystal_Test *lcd5 = new LiquidCrystal_Test(rs, enable, d4, d5, d6, d7);
  LiquidCrystal_Test *lcd6 =
      new LiquidCrystal_Test(rs, rw, enable, d4, d5, d6, d7);
  LiquidCrystal_Test *lcd7 =
      new LiquidCrystal_Test(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  LiquidCrystal_Test *lcd8 =
      new LiquidCrystal_Test(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
  assertNotNull(lcd5);
  assertNotNull(lcd6);
  assertNotNull(lcd7);
  assertNotNull(lcd8);
  delete lcd8;
  delete lcd7;
  delete lcd6;
  delete lcd5;
}

/*     rs rw  d7 to d0
   48 : 0  0  00110000      set to 8-bit mode (takes three tries)
   48 : 0  0  00110000      set to 8-bit mode
   48 : 0  0  00110000      set to 8-bit mode
   32 : 0  0  00100000      set to 4-bit mode, 1 line, 8-bit font
   32 : 0  0  0010          \
    0 : 0  0      0000       set to 4-bit mode, 1 line, 8-bit font
    0 : 0  0  0000          \
  192 : 0  0      1100       display on, cursor off, blink off
    0 : 0  0  0000          \
  016 : 0  0      0001       clear display
    0 : 0  0  0000          \
   96 : 0  0      0110       increment cursor position, no display shift
*/
unittest(init) {
  vector<int> expected{48, 48, 48, 32, 32, 0, 0, 192, 0, 16, 0, 96};
  BitCollector pinValues(false); // test the next line (a constructor)
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
   48 : 0  0  00110000      set to 8-bit mode (takes three tries)
   48 : 0  0  00110000      set to 8-bit mode
   48 : 0  0  00110000      set to 8-bit mode
   32 : 0  0  00100000      set to 4-bit mode, 1 line, 8-bit font
   32 : 0  0  0010          \
  128 : 0  0      1000       set to 4-bit mode, 2 lines, 8-bit font
    0 : 0  0  0000          \
  192 : 0  0      1100       display on, cursor off, blink off
    0 : 0  0  0000          \
  016 : 0  0      0001       clear display
    0 : 0  0  0000          \
   96 : 0  0      0110       increment cursor position, no display shift
*/
unittest(begin) {
  vector<int> expected{48, 48, 48, 32, 32, 128, 0, 192, 0, 16, 0, 96};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  BitCollector pinValues(false); // test the next line
  lcd.begin(16, 2);
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
      0 : 0  0  0000
    208 : 0  0      1101  00001101 = display on, cursor blink on
*/
unittest(blink) {
  vector<int> expected{0, 192};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.noBlink();
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
      0 : 0  0  0000
    192 : 0  0      1100  00001100 = display on, cursor blink off
*/
unittest(noBlink) {
  vector<int> expected{0, 192};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.noBlink();
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
   64 : 0  0  0100
    0 : 0  0      0000
  512 : 1  0  0000
  512 : 1  0      0000
  528 : 1  0  0001
  528 : 1  0      0001
  512 : 1  0  0000
  512 : 1  0      0000
  512 : 1  0  0000
  512 : 1  0      0000
  528 : 1  0  0001
  528 : 1  0      0001
  512 : 1  0  0000
  736 : 1  0      1110
  512 : 1  0  0000
  512 : 1  0      0000
  512 : 1  0  0000
  512 : 1  0      0000
*/
unittest(createChar) {
  vector<int> expected{64,  0,   512, 512, 528, 528, 512, 512, 512,
                       512, 528, 528, 512, 736, 512, 512, 512, 512};
  byte smiley[8] = {B00000, B10001, B00000, B00000, B10001, B01110, B00000};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  BitCollector pinValues(false); // test the next line
  lcd.createChar(0, smiley);
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
    0 : 0  0  0000          \
   16 : 0  0      0001       clear
*/
unittest(clear) {
  vector<int> expected{0, 16};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.clear();
  assertTrue(pinValues.isEqualTo(expected));
}

/*      rs rw  d7 to d0
    576 : 1  0  0100      \
    640 : 1  0      1000  0x48 H
    608 : 1  0  0110      \
    592 : 1  0      0101  0x65 e
    608 : 1  0  0110      \
    704 : 1  0      1100  0x6C l
    608 : 1  0  0110      \
    704 : 1  0      1100  0x6C l
    608 : 1  0  0110      \
    752 : 1  0      1111  0x6F o
*/
unittest(print_hello) {
  vector<int> expected{576, 640, 608, 592, 608, 704, 608, 704, 608, 752};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.print("Hello");
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
   16 : 0  0  0001      \
  128 : 0  0      1000   00011000 = shift display left
*/
unittest(scrollDisplayLeft) {
  vector<int> expected{16, 128};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.scrollDisplayLeft();
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
   16 : 0  0  0001      first half of command
  192 : 0  0      1100  full command: 00011100 = shift display right
*/
unittest(scrollDisplayRight) {
  vector<int> expected{16, 192};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.scrollDisplayRight();
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(failIfWrongSize) {
  vector<int> expected{0};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.clear();
  assertFalse(pinValues.isEqualTo(expected));
}

unittest(failIfWrongValues) {
  vector<int> expected{0, 255};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.clear();
  assertFalse(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
    0 : 0  0  0000
   32 : 0  0      0010
*/
unittest(home) {
  vector<int> expected{0, 32};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.home();
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
    0 : 0  0  0000
   96 : 0  0      0110  => left to right
*/
unittest(leftToRight) {
  vector<int> expected{0, 96};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.leftToRight();
  assertTrue(pinValues.isEqualTo(expected));
}

/*     rs rw  d7 to d0
    0 : 0  0  0000
   64 : 0  0      0100  => right to left
*/
unittest(rightToLeft) {
  vector<int> expected{0, 64};
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  BitCollector pinValues(false); // test the next line
  lcd.rightToLeft();
  assertTrue(pinValues.isEqualTo(expected));
}
