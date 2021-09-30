/*
 * Bulls and Cows game for the ATTiny85 and SSD1306 OLED
 *
 * https://github.com/datacute/TinyBullsAndCows
 *
 */

#include <avr/sleep.h>
//#include <TinyI2CMaster.h>
#include <TinyWireM.h>
#include <Tiny4kOLED.h>
// ============================================================================

// 3 x 5 pixel digits
// LSB is at the top
const uint8_t digits [] PROGMEM = {
  0x0E,0x11,0x0E, // 0
  0x02,0x1F,0x00, // 1
  0x12,0x19,0x16, // 2
  0x15,0x15,0x0A, // 3
  0x07,0x04,0x1F, // 4
  0x17,0x15,0x09, // 5
  0x0E,0x15,0x09, // 6
  0x01,0x19,0x07, // 7
  0x0A,0x15,0x0A, // 8
  0x12,0x15,0x0E, // 9
};

bool showingSplashScreen = true;
bool enteringDigit = true;

uint8_t selectedRow = 11;
uint8_t numGuesses = 0;
uint8_t guess = 0;
uint8_t guesses[10][6] = {
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
};
uint8_t solution[4] = {0, 0, 0, 0};
bool gameOver = false;
bool gameWon = false;

bool inputUp = false;
bool inputDown = false;
bool inputSwitchIn = false;
bool inputSwitchChanged = false;

const uint8_t downPin = 3;
const uint8_t upPin = 4;
const uint8_t switchPin = 1;

unsigned long lastInput;

ISR(PCINT0_vect) {}

void setup() {
    setupADC();
    setupInputs();
    setupOLED();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void setupInputs() {
  pinMode(upPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  GIMSK = 1 << PCIE;                   // Enable pin-change interrupt
  PCMSK = (1 << upPin) | (1 << downPin) | (1 << switchPin);
}

void setupOLED() {
    oled.begin();
    oled.setFont(FONT6X8);
    showSplashScreen();
}

void setupADC() {
    ADCSRA &= ~(1 << ADEN);              // Disable ADC to save power
}

void showSplashScreen() {
    oled.off();
    if (oled.currentDisplayFrame() == 1) oled.switchDisplayFrame();
    if (oled.currentRenderFrame() == 1) oled.switchRenderFrame();
    oled.clear();
    oled.switchRenderFrame();
    oled.clear();
    oled.switchRenderFrame();

    oled.setCursor(1,0);
    oled.print(F("B U L L S  &  C O W S"));

    oled.setCursor(0,1);
    oled.fillToEOL(0x02);

    oled.setCursor(10,2);
    oled.print(F("Guess four numbers")); // Find four Bulls
    oled.setCursor(13,3);
    oled.print(F("Bulls are correct")); // Bulls are correct
    oled.setCursor(0,4);
    oled.print(F("Cows are in the wrong"));
    oled.setCursor(0,5);
    oled.print(F("place."));
    oled.setCursor(80,6);
    oled.print(F("Datacute"));

    oled.setCursor(00,7);
    oled.startData();
    for (uint8_t i = 0; i < 16; i++) {
        oled.sendData(0x02);
        oled.sendData(0x02);
        oled.sendData(0x02);
        oled.sendData(0x0C);
        oled.sendData(0x10);
        oled.sendData(0x10);
        oled.sendData(0x10);
        oled.sendData(0x0C);
    }
    oled.endData();

    oled.scrollLeftOffset(7,1,7,1);
    oled.setVerticalScrollArea(11, 50);
    oled.activateScroll();
    oled.on();
    showingSplashScreen = true;
}

void loop() {
    readInputs();
    processInputs();
    updateDisplay();
    //if (!(inputSwitchChanged || inputSwitchIn || inputUp || inputDown))
        //sleepWaitingForInput();
}

void readInputs() {
    uint8_t inputs = PINB;

    inputDown = ((inputs >> downPin) & 1) == 0;
    inputUp = ((inputs >> upPin) & 1) == 0;

    bool prevInputSwitchIn = inputSwitchIn;
    inputSwitchIn = ((inputs >> switchPin) & 1) == 0;
    inputSwitchChanged = (prevInputSwitchIn != inputSwitchIn);
}

void processInputs() {
    if (!(inputSwitchChanged || inputUp || inputDown))
        return;

    unsigned long timeNow = millis();
    if (timeNow < lastInput + 200L)
        return;

    lastInput = timeNow;

    if (showingSplashScreen && (inputSwitchChanged || inputSwitchIn)) {
        oled.off();
        oled.deactivateScroll();
        oled.setVerticalScrollArea(0,64);
        oled.clear();
        oled.switchRenderFrame();
        oled.clear();
        oled.on();
        initGame();
        return;
    }

    if (enteringDigit) {
        uint8_t pos = selectedRow - 11;
        uint8_t currentDigit = guesses[guess][pos];
        if (currentDigit == 10) currentDigit = 0;
        if (inputUp && currentDigit < 9) {
            // todo skip over already used digits
            // todo wrap from 9 to 0
            currentDigit++;
            guesses[guess][pos] = currentDigit;
        }
        if (inputDown && currentDigit > 0) {
            // todo skip over already used digits
            // todo wrap from 0 to 9
            currentDigit--;
            guesses[guess][pos] = currentDigit;
        }
        if (inputSwitchChanged && inputSwitchIn) {
            enteringDigit = false;
        }
        return;
    }

    if (inputUp && selectedRow > 0) {
        selectedRow--;
        if (gameOver) {
            if ((selectedRow >= 10) || (numGuesses == 0))
                selectedRow = 10;
            else {
                if (selectedRow > numGuesses - 1)
                    selectedRow = numGuesses - 1;
            }
        } else {
            if ((selectedRow >= guess) && (selectedRow < 11)) {
                if (guess > 0)
                    selectedRow = guess - 1;
                else
                    selectedRow = 11;
            }
        }
    }

    if (inputDown && selectedRow < (gameOver ? 15 : 16)) {
        selectedRow++;
        if (gameOver) {
            if (selectedRow > 10)
                selectedRow = 15;
            else if (selectedRow >= numGuesses)
                selectedRow = 10;
        } else {
            if ((selectedRow >= guess) && (selectedRow < 11))
                selectedRow = 11;
        }
    }

    if (inputSwitchChanged && inputSwitchIn) {
        if (selectedRow >= 11) {
            if (selectedRow < 15) {
                enteringDigit = true;
            } else {
                if (gameOver) {
                    showSplashScreen();
                    return;
                }
                if (selectedRow == 16) {
                    for (uint8_t d = 0; d < 4; d++) {
                        guesses[guess][d] = 10;
                    }
                    gameOver = true;
                    selectedRow = 10;
                    return;
                }
                // submit guess
                numGuesses++;
                // calculate bulls and cows
                uint8_t bulls = 0;
                uint8_t cows = 0;
                uint8_t herd[4] = {10, 10, 10, 10};
                for (uint8_t d = 0; d < 4; d++) {
                    uint8_t digit = guesses[guess][d];
                    if (digit == 10) {
                        guesses[guess][d] = 0;
                        digit = 0;
                    }
                    if (digit == solution[d]) {
                        bulls++;
                    } else {
                        herd[d] = solution[d];
                    }
                }
                guesses[guess][4] = bulls;
                if (bulls == 4) {
                    gameOver = true;
                    gameWon = true;
                    selectedRow = 10;
                    guess++;
                    return;
                }
                for (uint8_t d = 0; d < 4; d++) {
                    uint8_t digit = guesses[guess][d];
                    if (digit != solution[d]) {
                        for (uint8_t h = 0; h < 4; h++) {
                            if (digit == herd[h]) {
                                cows++;
                                herd[h] = 10;
                            }
                        }
                    }
                }
                guesses[guess][5] = cows;
                selectedRow = guess;
                guess++;
                if (numGuesses == 10) {
                    // lost
                    gameOver = true;
                    selectedRow = 10;
                }
            }
        }
    }
}

void initGame() {
    enteringDigit = true;
    selectedRow = 11;
    guess = 0;
    numGuesses = 0;
    gameOver = false;
    gameWon = false;
    showingSplashScreen = false;
    for (uint8_t g = 0; g < 10; g++) {
        for (uint8_t d = 0; d < 4; d++) {
            guesses[g][d]=10;
        }
        guesses[g][4]=0;
        guesses[g][5]=0;
    }
    randomSeed(millis());
    solution[0] = random(10);
    solution[1] = random(10);
    solution[2] = random(10);
    solution[3] = random(10);
}

void updateDisplay() {
    if (showingSplashScreen)
        return;

    uint8_t displayedRow = selectedRow;
    if (selectedRow > guess) {
        if (gameOver) {
            displayedRow = 10;
        } else {
            displayedRow = guess;
        }
    }
    oled.setCursor(0,0);
    oled.startData();
    oled.sendData(displayedRow == 0 ? 0xC0 : ((gameOver && (numGuesses == 0)) ? 0x80 : 0x00));
    for (uint8_t d = 0; d < 10; d++) {
        sendBullsCows(guesses[d][4], guesses[d][5], d == displayedRow);
        if ((d == displayedRow) || ((d + 1 == displayedRow) && (d < 9))) {
            oled.sendData(0xC0);
        } else {
            oled.sendData(((d + 1 < guess) || (guess == 10)) ? 0x00 : 0x80);
        }
    }
    oled.endData();

    oled.setCursor(0,1);
    oled.startData();
    oled.sendData(guess == 0 ? 0x20 : 0x00);
    for (uint8_t d = 0; d < 10; d++) {
        //if (guesses[d][0] < 10) { 
        //if (d < guess || (gameOver && (d == guess))) {
        if (d <= guess) {
            sendDigit12(guesses[d][0], guesses[d][1]);
        } else {
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
        }
        oled.sendData(((d + 1 < guess) || (guess == 10)) ? 0x00 : 0x20);
    }
    oled.endData();

    oled.setCursor(0,2);
    oled.startData();
    oled.sendData(guess == 0 ? 0x08 : 0x00);
    for (uint8_t d = 0; d < 10; d++) {
        //if (guesses[d][0] < 10) { 
        //if (d < guess || (gameOver && (d == guess))) {
        if (d <= guess) {
            sendDigit23(guesses[d][1], guesses[d][2]);
        } else {
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
        }
        oled.sendData(((d + 1 < guess) || (guess == 10)) ? 0x00 : 0x08);
    }
    oled.endData();

    oled.setCursor(0,3);
    oled.startData();
    oled.sendData(guess == 0 ? 0x82 : 0x00);
    for (uint8_t d = 0; d < 10; d++) {
        //if (guesses[d][0] < 10) { 
        //if (d < guess || (gameOver && (d == guess))) {
        if (d <= guess) {
            sendDigit34(guesses[d][2], guesses[d][3]);
        } else {
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
            oled.sendData(0x00);
        }
        oled.sendData(((d + 1 < guess) || (guess == 10)) ? 0x00 : 0x82);
    }
    oled.endData();

    if (gameOver) {
        revealAnswerSquares();
    } else {
        drawHiddenAnswerSquares();
    }

    //uint8_t displayedRow = selectedRow < guess ? selectedRow : guess;
    if (displayedRow == 10) {
        displayDigits(solution[0], solution[1], solution[2], solution[3]);
    } else {
        displayDigits(guesses[displayedRow][0], guesses[displayedRow][1], guesses[displayedRow][2], guesses[displayedRow][3]);

        selectDigit((selectedRow >= 11) && (selectedRow < 15) ? selectedRow - 11 : 5);
    }

    showBullsCowsMenu();

    oled.switchFrame();
}

void sendBullsCows(uint8_t bulls, uint8_t cows, bool rowSelected) {
    uint8_t data = rowSelected ? 0x40 : 0x00;
    uint8_t bullBit = 0x01;
    for (uint8_t b = 0; b < bulls; b++) {
        data |= bullBit;
        bullBit <<= 1;
    }
    uint8_t cowBit = 0x10;
    for (uint8_t c = 0; c < cows; c++) {
        data |= cowBit;
        cowBit >>= 1;
    }
    oled.sendData(data);
    oled.sendData(data);
    oled.sendData(data);
    oled.sendData(data);
    oled.sendData(data);
}

void sendDigit12(uint8_t d1, uint8_t d2) {
    oled.sendData(0x00);
    for (uint8_t r = 0; r < 3; r++) {
        uint8_t b1 = d1 == 10 ? 0 : pgm_read_byte(&digits[d1 * 3 + r]);
        uint8_t b2 = d2 == 10 ? 0 : pgm_read_byte(&digits[d2 * 3 + r]);
        oled.sendData(b1 | (b2 << 6));
    }
    oled.sendData(0x00);
}

void sendDigit23(uint8_t d2, uint8_t d3) {
    oled.sendData(0x00);
    for (uint8_t r = 0; r < 3; r++) {
        uint8_t b2 = d2 == 10 ? 0 : pgm_read_byte(&digits[d2 * 3 + r]);
        uint8_t b3 = d3 == 10 ? 0 : pgm_read_byte(&digits[d3 * 3 + r]);
        oled.sendData((b2 >> 2) | (b3 << 4));
    }
    oled.sendData(0x00);
}

void sendDigit34(uint8_t d3, uint8_t d4) {
    oled.sendData(0x00);
    for (uint8_t r = 0; r < 3; r++) {
        uint8_t b3 = d3 == 10 ? 0 : pgm_read_byte(&digits[d3 * 3 + r]);
        uint8_t b4 = d4 == 10 ? 0 : pgm_read_byte(&digits[d4 * 3 + r]);
        oled.sendData((b3 >> 4) | (b4 << 2));
    }
    oled.sendData(0x00);
}

void drawHiddenAnswerSquares() {
    oled.setCursor(63,0);
    oled.fillLength(0x80, 7);

    oled.setCursor(63,1);
    oled.startData();
    oled.sendData(0xFF);
    oled.sendData(0x20);
    oled.sendData(0x20);
    oled.sendData(0x24);
    oled.sendData(0x20);
    oled.sendData(0x20);
    oled.sendData(0xFF);
    oled.endData();

    oled.setCursor(63,2);
    oled.startData();
    oled.sendData(0xFF);
    oled.sendData(0x08);
    oled.sendData(0x08);
    oled.sendData(0x49);
    oled.sendData(0x08);
    oled.sendData(0x08);
    oled.sendData(0xFF);
    oled.endData();

    oled.setCursor(63,3);
    oled.startData();
    oled.sendData(0xFF);
    oled.sendData(0x82);
    oled.sendData(0x82);
    oled.sendData(0x92);
    oled.sendData(0x82);
    oled.sendData(0x82);
    oled.sendData(0xFF);
    oled.endData();
}

void revealAnswerSquares() {
    oled.setCursor(63,0);
    if (selectedRow == 10) {
        oled.startData();
        oled.sendData(0xC0);
        oled.sendData(0x40);
        oled.sendData(0x40);
        oled.sendData(0x40);
        oled.sendData(0x40);
        oled.sendData(0x40);
        oled.sendData(0xC0);
        oled.endData();
    } else {
        oled.fillLength(0x00, 7);
    }

    oled.setCursor(63,1);
    oled.startData();
    oled.sendData(0x00);
    sendDigit12(solution[0], solution[1]);
    oled.sendData(0x00);
    oled.endData();

    oled.setCursor(63,2);
    oled.startData();
    oled.sendData(0x00);
    sendDigit23(solution[1], solution[2]);
    oled.sendData(0x00);
    oled.endData();

    oled.setCursor(63,3);
    oled.startData();
    oled.sendData(0x00);
    sendDigit34(solution[2], solution[3]);
    oled.sendData(0x00);
    oled.endData();
}

void displayDigits(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
    oled.setFont(FONT8X16DIGITS);

    oled.setCursor(76, 0);
    printDigit(d1);

    oled.setCursor(89, 0);
    printDigit(d2);

    oled.setCursor(102, 0);
    printDigit(d3);

    oled.setCursor(115, 0);
    printDigit(d4);

    oled.setFont(FONT6X8);
}

void printDigit(uint8_t d) {
    oled.print((char)((d == 10 ? 0 : d) + 48));
}

void selectDigit(uint8_t d) {
    uint8_t x = 71;
    for (uint8_t k = 0; k <= 4; k++) {
        if (k == d) {
            if (enteringDigit) {
                oled.setCursor(x, 0);
                oled.startData();
                oled.sendData(0xFE);
                oled.sendData(0xFE);
                oled.sendData(0x06);
                oled.sendData(0x02);
                oled.sendData(0x00);
                oled.endData();

                oled.setCursor(x, 1);
                oled.startData();
                oled.sendData(0x7F);
                oled.sendData(0x7F);
                oled.sendData(0x60);
                oled.sendData(0x40);
                oled.sendData(0x00);
                oled.endData();
            } else {
                oled.setCursor(x, 0);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0xF8);
                oled.sendData(0x04);
                oled.sendData(0x02);
                oled.sendData(0x00);
                oled.endData();

                oled.setCursor(x, 1);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0x1F);
                oled.sendData(0x20);
                oled.sendData(0x40);
                oled.sendData(0x00);
                oled.endData();
            }
        } else if (k == d + 1) {
            if (enteringDigit) {
                oled.setCursor(x, 0);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0x02);
                oled.sendData(0x06);
                oled.sendData(0xFE);
                oled.sendData(0xFE);
                oled.endData();

                oled.setCursor(x, 1);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0x40);
                oled.sendData(0x60);
                oled.sendData(0x7F);
                oled.sendData(0x7F);
                oled.endData();
            } else {
                oled.setCursor(x, 0);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0x02);
                oled.sendData(0x04);
                oled.sendData(0xF8);
                oled.sendData(0x00);
                oled.endData();

                oled.setCursor(x, 1);
                oled.startData();
                oled.sendData(0x00);
                oled.sendData(0x40);
                oled.sendData(0x20);
                oled.sendData(0x1F);
                oled.sendData(0x00);
                oled.endData();
            }
        } else {
            oled.setCursor(x, 0);
            oled.fillLength(0x00, 5);

            oled.setCursor(x, 1);
            oled.fillLength(0x00, 5);
        }
        x += 13;
    }
}

void showBullsCowsMenu() {
    if (selectedRow < guess) {
        oled.setCursor(72, 2);
        oled.print(" Bulls ");
        oled.print((char)(guesses[selectedRow][4]+48));
        oled.print(' ');
        oled.setCursor(72, 3);
        oled.print(" Cows  ");
        oled.print((char)(guesses[selectedRow][5]+48));
        oled.print(' ');
    } else {
        if (gameOver) {
            oled.setCursor(72, 2);
            if (gameWon) {
                oled.print(" You Won ");
            } else {
                oled.print(" You Lost");
            }
            oled.setCursor(72, 3);
            oled.print(selectedRow == 15 ? '[' : ' ');
            oled.print("Restart");
            oled.print(selectedRow == 15 ? ']' : ' ');
        } else {
            oled.setCursor(72, 2);
            oled.print(selectedRow == 15 ? " [" : "  ");
            oled.print("Guess");
            oled.print(selectedRow == 15 ? "] " : "  ");
            oled.setCursor(72, 3);
            oled.print(selectedRow == 16 ? '[' : ' ');
            oled.print("Give Up");
            oled.print(selectedRow == 16 ? ']' : ' ');
        }
    }
}

void sleepWaitingForInput() {
    uint8_t temp = TIMSK;
    TIMSK = 0;                         // Disable timer interrupt(s)
    sleep_enable();
    sleep_cpu();
    TIMSK = temp;                      // Re-enable timer interrupt(s)
}
