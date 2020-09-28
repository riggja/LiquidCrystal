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

GodmodeState *state = GODMODE();

class BitCollector : public DataStreamObserver {
private:
  bool fourBitMode;
  bool showData;
  vector<int> pinLog;

public:
  BitCollector(bool showData = false, bool fourBitMode = true)
      : DataStreamObserver(false, false) {
    this->fourBitMode = fourBitMode;
    this->showData = showData;
  }

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
        std::cout.width(7);
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

unittest(init) {
  state->reset();
  BitCollector pinValues(false);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  state->digitalPin[enable].removeObserver("lcd");
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
  vector<int> expected{48, 48, 48, 32, 32, 0, 0, 192, 0, 16, 0, 96};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(begin_16_02) {
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.begin(16, 2);
  state->digitalPin[enable].removeObserver("lcd");
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
  vector<int> expected{48, 48, 48, 32, 32, 128, 0, 192, 0, 16, 0, 96};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(createChar) {
  byte smiley[8] = {
      B00000, B10001, B00000, B00000, B10001, B01110, B00000,
  };

  // Test the function
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.createChar(0, smiley);
  state->digitalPin[enable].removeObserver("lcd");
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
  vector<int> expected{64,  0,   512, 512, 528, 528, 512, 512, 512,
                       512, 528, 528, 512, 736, 512, 512, 512, 512};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(clear) {
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.clear();
  state->digitalPin[enable].removeObserver("lcd");
  /*     rs rw  d7 to d0
      0 : 0  0  0000          \
     16 : 0  0      0001       clear
   */
  vector<int> expected{0, 16};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(print_hello) {
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.print("Hello");
  state->digitalPin[enable].removeObserver("lcd");
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
  vector<int> expected{576, 640, 608, 592, 608, 704, 608, 704, 608, 752};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(scrollDisplayLeft) {
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.scrollDisplayLeft();
  state->digitalPin[enable].removeObserver("lcd");
  /*     rs rw  d7 to d0
     16 : 0  0  0001      \
    128 : 0  0      1000   00011000 = shift display left
   */
  vector<int> expected{16, 128};
  assertTrue(pinValues.isEqualTo(expected));
}

unittest(scrollDisplayRight) {
  state->reset();
  BitCollector pinValues(false);
  LiquidCrystal_Test lcd(rs, enable, d4, d5, d6, d7);
  lcd.begin(16, 2);
  state->digitalPin[enable].addObserver("lcd", &pinValues);
  lcd.scrollDisplayRight();
  state->digitalPin[enable].removeObserver("lcd");
  /*     rs rw  d7 to d0
     16 : 0  0  0001      first half of command
    192 : 0  0      1100  full command: 00011100 = shift display right
   */
  vector<int> expected{16, 192};
  assertTrue(pinValues.isEqualTo(expected));
}
