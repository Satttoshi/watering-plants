// Multiple Roomplant Watering System

#include <LiquidCrystal.h>

// Declare pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
const int button1 = 2; // + button
const int button2 = 3; // - button
const int button3 = 13; // select/confirm button
const int latchPin = 5; // Applies the current shift register bit set on HIGH (top pinout 0000X000)
const int clockPin = 6; // Shifts bits through shift register (top pinout 00000X00)
const int dataPin = 4; // Writes current bit into shift register (top pinout 00X00000)

// Init shift register variables
#define bitOrder MSBFIRST // Declares bit writing direction of data Pin into shift register
byte setBits = B11111111; // Default set of bits in shift register (1 = relay channel off)

// Init program variables
int targetMoisture = 999; // desired moisture
int selPump = 1; // Select selPump
int selector = 0; // Select option state
const int pumpQty = 3; // Amount of installed pumps

// Init pump configuration variables

int pump1dry = 0; // Dryness
int pump2dry = 0;
int pump3dry = 0;
int pump1tar = 999; // Target
int pump2tar = 999;
int pump3tar = 999;
int pump1dur = 5; // Duration
int pump2dur = 5;
int pump3dur = 5;
unsigned long pump1cd = 1; // Cooldown
unsigned long pump2cd = 1;
unsigned long pump3cd = 1;
unsigned long pump1dst = 0; // Delay Start Time
unsigned long pump2dst = 0;
unsigned long pump3dst = 0;
unsigned long pump1currentcd = 1;
unsigned long pump2currentcd = 1;
unsigned long pump3currentcd = 1;
bool pump1rdy = true; // Delay Completed
bool pump2rdy = true;
bool pump3rdy = true;

// Init Async function variables
unsigned long pollingStartTime = 0;
bool pollingCompleted = true;

// Init conditions
bool decision = false;
bool runOnce1 = false;
bool runOnce2 = false;
bool runOnce3 = false;

///////////////////////
// V A R I A B L E S //
///////////////////////
///////////////////////
// F U N C T I O N S //
///////////////////////

// Display abstraction to print on display int values (with default length 3)
void display(int col, int row, int number) {
  int length = floor(log10(abs(number))) + 1; // Get length (like string-length) from a number
  if (length == 0) {
    length = 1;
  }
  for (int i = 0; i < 3 - length; i++) {
    lcd.setCursor(col + i, row);
    lcd.print(" ");
  }
  lcd.setCursor(col + 3 - length, row);
  lcd.print(number);
}
// Display abstraction to print on display int values (with setable length)
void display(int col, int row, int number, int maxLength) {
  int length = floor(log10(abs(number))) + 1; // Get length (like string-length) from a number
  if (length == 0) {
    length = 1;
  }
  for (int i = 0; i < maxLength - length; i++) {
    lcd.setCursor(col + i, row);
    lcd.print(" ");
  }
  lcd.setCursor(col + maxLength - length, row);
  lcd.print(number);
}
// Display abstraction to print on display string values
void display(int col, int row, String text) {
  lcd.setCursor(col, row);
  lcd.print(text);
}
// Turn on Relay (int which relay 1-8)
void triggerRelay(int relay) {
  digitalWrite(latchPin, LOW);
  if (relay == 1) {
    setBits = B01111111;
  } else if (relay == 2) {
    setBits = B10111111;
  } else if (relay == 3) {
    setBits = B11011111;
  } else if (relay == 4) {
    setBits = B11101111;
  } else if (relay == 5) {
    setBits = B11110111;
  } else if (relay == 6) {
    setBits = B11111011;
  } else if (relay == 7) {
    setBits = B11111101;
  } else if (relay == 8) {
    setBits = B11111110;
  } else {
    setBits = B11111111;
  }
  // Function to write 8 bits into shift register and apply them to output
  shiftOut(dataPin, clockPin, bitOrder, setBits); 
  digitalWrite(latchPin, HIGH);
}


// Turn on relay, only do this once, check with boolean if digitalWrite did run, else ignore
bool pumpIsOn = false; // Outscoped function variable
void pumpOn(int relay, int duration) {
  if (!pumpIsOn) {
    triggerRelay(relay);
    pumpIsOn = true;
    display(0, 0, "Pump is running!"); // Print Pump is running aka. Relay is on
    delay(duration * 1000);
  }
}
void pumpOff() {
  if (pumpIsOn) {
    triggerRelay(0);
    pumpIsOn = false;
    display(0, 0, "                "); // Clear first row
    delay(100);
  }
}

///////////////////////
// F U N C T I O N S //
///////////////////////
///////////////////////
//     S E T U P     //
///////////////////////
void setup() {
  lcd.begin(16, 2); // Init LCD

  // Init Pins
  pinMode(A0, INPUT); // moisture sensor 1
  pinMode(A1, INPUT); // target potentiometer
  pinMode(A2, INPUT); // moisture sensor 2
  pinMode(A3, INPUT); // moisture sensor 3
  pinMode(A4, INPUT); // moisture sensor 4
  pinMode(A5, INPUT); // moisture sensor 5
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  triggerRelay(0);

  display(0, 0, "yoshiko watering");
  display(0, 1, "fuer pflanzen!");
  delay(2500);
  lcd.clear();


  // Init default text on Display
  display(0, 1, "Sel:");
  display(4, 1, selPump, 2);


  pump1dst = millis();
  pump2dst = millis();
  pump3dst = millis();

  pollingStartTime = millis();

}
///////////////////////
//     S E T U P     //
///////////////////////
///////////////////////
//      L O O P      //
///////////////////////


void loop() {


  // IF PUMP SELECT IS ON 1
  if (selPump == 1) {
    
    display(0, 0, pump1dry);
    display(8, 0, pump1dur);
    display(12, 0, pump1cd, 4);

    if (selector == 1 || selector == 2) {
      display(4, 0, targetMoisture);
      if (pollingCompleted) {
        targetMoisture = map(analogRead(A1), 0, 1023, 0, 999); // Read Poti for Target
      }
    } else display(4, 0, pump1tar);

    if (selector == 2) {
      display(0, 1, "confirm? ");
      display(10, 1, "no");
      display(13, 1, "yes");
      if (!runOnce1) {
        display(9, 1, ">");
        display(12, 1, " ");
        runOnce1 = true;
      }
      if (digitalRead(button1) == LOW) {
        display(9, 1, " ");
        display(12, 1, ">");
        decision = true;
      } else if (digitalRead(button2) == LOW) {
        display(9, 1, ">");
        display(12, 1, " ");
        decision = false;
      }
    } else {
      if (pump1currentcd <= 1) {
        display(7, 1, "CD:");
        display(10, 1, " ready");
      }
      else if (pump1currentcd < 60) {
        display(7, 1, "CD:");
        display(10, 1, pump1currentcd, 5);
        display(15, 1, "s");
      }
      else if (pump1currentcd > 3600) {
        display(7, 1, "CD:");
        display(10, 1, pump1currentcd / 60 / 60, 5);
        display(15, 1, "h");
      } else {
        display(7, 1, "CD:");
        display(10, 1, pump1currentcd / 60, 5);
        display(15, 1, "m");
      } 
    }

    if (selector == 3){
      runOnce1 = false;
      if (decision) {
        pump1tar = targetMoisture;
        display(4, 0, pump1tar);
        display(0, 1, "target applied! ");
        decision = false;
        selector = 0;
        delay(2000);
        display(0, 1, "Sel: 1          ");
      }
      display(0, 1, "Sel: 1 ");
      if (digitalRead(button1) == LOW) {
        if (pump1dur < 20) {
          pump1dur++;
          delay(200);
        }
      }
      if (digitalRead(button2) == LOW) {
        if (pump1dur > 1) {
          pump1dur--;
          delay(200);
        }
      }      
    }

    if (selector == 4) {
      if(digitalRead(button1) == LOW) {
        if(pump1cd < 9720) {
          if (pump1cd < 10) {
            pump1cd++;
          } else if (pump1cd < 30) {
            pump1cd += 5;
          } else if (pump1cd < 120) {
            pump1cd += 10;
          } else if (pump1cd < 1440) {
            pump1cd += 60;
          } else if (pump1cd < 9720) {
            pump1cd += 360;
          }
          delay(150);
        }
      }
      if(digitalRead(button2) == LOW) {
        if (pump1cd > 1) {
          if (pump1cd <= 10) {
            pump1cd-= 1;
          } else if (pump1cd <= 30) {
            pump1cd -= 5;
          } else if (pump1cd <= 120) {
            pump1cd -= 10;
          } else if (pump1cd <= 1440) {
            pump1cd -= 60;
          } else if (pump1cd <= 9720) {
            pump1cd -= 360;
          }
          delay(150);
        }
      }
    }
  }

  // IF PUMP SELECT IS ON 2
  if (selPump == 2) {
    
    display(0, 0, pump2dry);
    display(8, 0, pump2dur);
    display(12, 0, pump2cd, 4);

    if (selector == 1 || selector == 2) {
      display(4, 0, targetMoisture);
      if (pollingCompleted) {
        targetMoisture = map(analogRead(A1), 0, 1023, 0, 999); // Read Poti for Target
      }
    } else display(4, 0, pump2tar);

    if (selector == 2) {
      display(0, 1, "confirm? ");
      display(10, 1, "no");
      display(13, 1, "yes");
      if (!runOnce2) {
        display(9, 1, ">");
        display(12, 1, " ");
        runOnce2 = true;
      }
      if (digitalRead(button1) == LOW) {
        display(9, 1, " ");
        display(12, 1, ">");
        decision = true;
      } else if (digitalRead(button2) == LOW) {
        display(9, 1, ">");
        display(12, 1, " ");
        decision = false;
      }
    } else {
      if (pump2currentcd <= 1) {
        display(7, 1, "CD:");
        display(10, 1, " ready");
      }
      else if (pump2currentcd < 60) {
        display(7, 1, "CD:");
        display(10, 1, pump2currentcd, 5);
        display(15, 1, "s");
      }
      else if (pump2currentcd > 3600) {
        display(7, 1, "CD:");
        display(10, 1, pump2currentcd / 60 / 60, 5);
        display(15, 1, "h");
      } else {
        display(7, 1, "CD:");
        display(10, 1, pump2currentcd / 60, 5);
        display(15, 1, "m");
      } 
    }

    if (selector == 3){
      runOnce2 = false;
      if (decision) {
        pump2tar = targetMoisture;
        display(4, 0, pump2tar);
        display(0, 1, "target applied! ");
        decision = false;
        selector = 0;
        delay(2000);
        display(0, 1, "Sel: 2          ");
      }
      display(0, 1, "Sel: 2 ");
      if (digitalRead(button1) == LOW) {
        if (pump2dur < 20) {
          pump2dur++;
          delay(200);
        }
      }
      if (digitalRead(button2) == LOW) {
        if (pump2dur > 1) {
          pump2dur--;
          delay(200);
        }
      }      
    }

    if (selector == 4) {
      if(digitalRead(button1) == LOW) {
        if(pump2cd < 9720) {
          if (pump2cd < 10) {
            pump2cd++;
          } else if (pump2cd < 30) {
            pump2cd += 5;
          } else if (pump2cd < 120) {
            pump2cd += 10;
          } else if (pump2cd < 1440) {
            pump2cd += 60;
          } else if (pump2cd < 9720) {
            pump2cd += 360;
          }
          delay(150);
        }
      }
      if(digitalRead(button2) == LOW) {
        if (pump2cd > 1) {
          if (pump2cd <= 10) {
            pump2cd-= 1;
          } else if (pump2cd <= 30) {
            pump2cd -= 5;
          } else if (pump2cd <= 120) {
            pump2cd -= 10;
          } else if (pump2cd <= 1440) {
            pump2cd -= 60;
          } else if (pump2cd <= 9720) {
            pump2cd -= 360;
          }
          delay(150);
        }
      }
    }
  }

  // IF PUMP SELECT IS ON 3
  if (selPump == 3) {
    
    display(0, 0, pump3dry);
    display(8, 0, pump3dur);
    display(12, 0, pump3cd, 4);

    if (selector == 1 || selector == 2) {
      display(4, 0, targetMoisture);
      if (pollingCompleted) {
        targetMoisture = map(analogRead(A1), 0, 1023, 0, 999); // Read Poti for Target
      }
    } else display(4, 0, pump3tar);

    if (selector == 2) {
      display(0, 1, "confirm? ");
      display(10, 1, "no");
      display(13, 1, "yes");
      if (!runOnce3) {
        display(9, 1, ">");
        display(12, 1, " ");
        runOnce3 = true;
      }
      if (digitalRead(button1) == LOW) {
        display(9, 1, " ");
        display(12, 1, ">");
        decision = true;
      } else if (digitalRead(button2) == LOW) {
        display(9, 1, ">");
        display(12, 1, " ");
        decision = false;
      }
    } else {
      if (pump3currentcd <= 1) {
        display(7, 1, "CD:");
        display(10, 1, " ready");
      }
      else if (pump3currentcd < 60) {
        display(7, 1, "CD:");
        display(10, 1, pump3currentcd, 5);
        display(15, 1, "s");
      }
      else if (pump3currentcd > 3600) {
        display(7, 1, "CD:");
        display(10, 1, pump3currentcd / 60 / 60, 5);
        display(15, 1, "h");
      } else {
        display(7, 1, "CD:");
        display(10, 1, pump3currentcd / 60, 5);
        display(15, 1, "m");
      } 
    }

    if (selector == 3){
      runOnce3 = false;
      if (decision) {
        pump3tar = targetMoisture;
        display(4, 0, pump3tar);
        display(0, 1, "target applied! ");
        decision = false;
        selector = 0;
        delay(2000);
        display(0, 1, "Sel: 3          ");
      }
      display(0, 1, "Sel: 3 ");
      if (digitalRead(button1) == LOW) {
        if (pump3dur < 20) {
          pump3dur++;
          delay(200);
        }
      }
      if (digitalRead(button2) == LOW) {
        if (pump3dur > 1) {
          pump3dur--;
          delay(200);
        }
      }      
    }

    if (selector == 4) {
      if(digitalRead(button1) == LOW) {
        if(pump3cd < 9720) {
          if (pump3cd < 10) {
            pump3cd++;
          } else if (pump3cd < 30) {
            pump3cd += 5;
          } else if (pump3cd < 120) {
            pump3cd += 10;
          } else if (pump3cd < 1440) {
            pump3cd += 60;
          } else if (pump3cd < 9720) {
            pump3cd += 360;
          }
          delay(150);
        }
      }
      if(digitalRead(button2) == LOW) {
        if (pump3cd > 1) {
          if (pump3cd <= 10) {
            pump3cd-= 1;
          } else if (pump3cd <= 30) {
            pump3cd -= 5;
          } else if (pump3cd <= 120) {
            pump3cd -= 10;
          } else if (pump3cd <= 1440) {
            pump3cd -= 60;
          } else if (pump3cd <= 9720) {
            pump3cd -= 360;
          }
          delay(150);
        }
      }
    }
  }



  if (selector == 1) { // Select target moisture
    display(3, 0, ">");  
  } else display(3, 0, " ");

  if (selector == 3) { // Select duration
    display(7, 0, ">");
  } else display(7, 0, " ");

  if (selector == 4) { // Select cooldown
    display(11, 0, ">");
  } else display(11, 0, " ");




  if(pollingCompleted) {
    
    pump1dry = map(analogRead(A0), 0, 1023, 0, 999); // Read plant dryness and limit values to 999 max instead of 1023
    pump2dry = map(analogRead(A2), 0, 1023, 0, 999);
    pump3dry = map(analogRead(A3), 0, 1023, 0, 999);

    pollingStartTime = millis();
    pollingCompleted = false;
  }
  if(!pollingCompleted) {
    if (millis() - pollingStartTime >= 300) {
      pollingCompleted = true;
    }
  }


  // Pump triggers if dryness is low (uses Async-system)
  
  if(pump1dry > pump1tar && pump1rdy) {
    pumpOn(1, pump1dur); // Turn pump on
    pump1dst = millis(); // Writes current running duration of arduino
    pump1rdy = false; // Sets Promise to false
    pumpOff();
    pump1currentcd = 1;
  } else if (!pump1rdy) { // Clear Promise for Async-system and updates current cooldown display
    if (pump1currentcd > 0) {
      pump1currentcd = ((pump1cd * 1000 * 60) - (millis() - pump1dst)) / 1000;
    }
    if (millis() - pump1dst >= pump1cd * 1000 * 60) {
      pump1rdy = true;
    }
  }
  if(pump2dry > pump2tar && pump2rdy) {
    pumpOn(2, pump2dur); // Turn pump on
    pump2dst = millis(); // Writes current running duration of arduino
    pump2rdy = false; // Sets Promise to false
    pumpOff();
  } else if (!pump2rdy) { // Clear Promise for Async-system
    if (pump2currentcd > 0) {
    pump2currentcd = ((pump2cd * 1000 * 60) - (millis() - pump2dst)) / 1000;
    }
    if (millis() - pump2dst >= pump2cd * 1000 * 60) {
      pump2rdy = true;
    }
  }
  if(pump3dry > pump3tar && pump3rdy) {
    pumpOn(3, pump3dur); // Turn pump on
    pump3dst = millis(); // Writes current running duration of arduino
    pump3rdy = false; // Sets Promise to false
    pumpOff();
  } else if (!pump3rdy) { // Clear Promise for Async-system
    if (pump3currentcd > 0) {
    pump3currentcd = ((pump3cd * 1000 * 60) - (millis() - pump3dst)) / 1000;
    }
    if (millis() - pump3dst >= pump3cd * 1000 * 60) {
      pump3rdy = true;
    }
  }


  // Button 1 -> +1 selPump
  if (digitalRead(button1) == LOW) {
    if (selPump < pumpQty && selector == 0) {
      selPump++;
      display(4,1,selPump,2);
      delay(200);
    }
  }
  // Button 2 -> -1 selPump
  if (digitalRead(button2) == LOW) {
    if (selPump > 1 && selector == 0) {
      selPump--;
      display(4,1,selPump,2);
      delay(200);
      }
  }
  // Button 3 -> select/confirm
  if (digitalRead(button3) == LOW) {
    if (selector < 4){
      selector++;
      delay(200);
    } else {
      selector = 0;
      delay(200);
    }

  }
  
}
///////////////////////
//      L O O P      //
///////////////////////



// PSEUDO
//
// Selector 0 = View current Settings -> Selector 1 = change Target variable -> Selector 2 = ask for apply target? >no   yes (switchable with + - buttons) -> Selector 3 = apply selection (short confirm notification) Selector 4 =
//
//
//
//
//
//
//