

// Choose your I2C implementation before including Tiny4kOLED.h
// The default is selected is Wire.h
#include <avr/sleep.h>
#include <EEPROM.h>

// To use the Wire library:
//#include <Wire.h>

// To use the Adafruit's TinyWireM library:
#include <TinyWireM.h>

// To use the TinyI2C library from https://github.com/technoblogy/tiny-i2c
//#include <TinyI2CMaster.h>

// The blue OLED screen requires a long initialization on power on.
// The code to wait for it to be ready uses 20 bytes of program storage space
// If you are using a white OLED, this can be reclaimed by uncommenting
// the following line (before including Tiny4kOLED.h):
#define TINY4KOLED_QUICK_BEGIN,

#include <Tiny4kOLED.h>

#define SCREEN_WIDTH 16
#define SCREEN_HEIGHT 24

#define EEPROM_ADDR 4

#define PIN_LEFT PB1
#define PIN_RIGHT PB4
#define PIN_ROTATE PB3


const uint8_t nums[80] PROGMEM = {
0x0,0xe,0x11,0x19,0x15,0x13,0x11,0xe,
0x0,0xe,0x4,0x4,0x4,0x4,0xc,0x4,
0x0,0x1f,0x8,0x4,0x2,0x1,0x11,0xe,
0x0,0xe,0x11,0x1,0x2,0x4,0x2,0x1f,
0x0,0x2,0x2,0x1f,0x12,0xa,0x6,0x2,
0x0,0xe,0x11,0x1,0x1,0x1e,0x10,0x1f,
0x0,0xe,0x11,0x11,0x1e,0x10,0x8,0x6,
0x0,0x8,0x8,0x8,0x4,0x2,0x1,0x1f,
0x0,0xe,0x11,0x11,0xe,0x11,0x11,0xe,
0x0,0xc,0x2,0x1,0xf,0x11,0x11,0xe,
};

const uint8_t pieces[7][16] PROGMEM = {{
0,0,1,0,
0,0,1,0,
0,0,1,0,
0,0,1,0
},{
0,0,0,0,
0,1,1,0,
0,1,1,0,
0,0,0,0
},{
0,0,0,0,
0,1,1,0,
0,0,1,1,
0,0,0,0
},{
0,0,0,0,
0,1,1,0,
1,1,0,0,
0,0,0,0
},{
0,0,0,0,
1,1,1,0,
0,1,0,0,
0,0,0,0
},{
0,1,0,0,
0,1,0,0,
1,1,0,0,
0,0,0,0
},{
0,1,0,0,
0,1,0,0,
0,1,1,0,
0,0,0,0
}};
typedef struct activePiece activePiece;
struct activePiece
{
    int x;
    uint8_t y;
    uint8_t *piece;
    uint8_t id;
};

uint8_t nextPiece[16];

uint8_t left = 20;
uint8_t n = 1;
int level = 0;
int linesCleared = 0;
uint8_t npiece; // next piece

uint8_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

bool madeLine = false;



activePiece active = { 4, 0, 0, 1 };

unsigned long elapsed = 0;
unsigned long ms = 0;
unsigned int await = 720;

unsigned long score = 0;
unsigned long highScore = 0;

void readPiece(uint8_t p[16], uint8_t n) {
  for(int i=0;i<16;i++){
    p[i] = pgm_read_byte(&pieces[n][i]);
  }
}





void setup() {

  randomSeed(analogRead(A3));
  oled.begin();
  elapsed = millis();
  ms = millis();
  oled.setInverse(false);
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      int pos = y * SCREEN_WIDTH + x;
      if (x == 0 || x >= 11 || y == SCREEN_HEIGHT - 1) {
        pixels[pos / 8] |= 1 << (pos % 8);
      }
    }
  }
//
//  EEPROM.update(EEPROM_ADDR, 0);
//  EEPROM.update(EEPROM_ADDR+8, 0);
//  EEPROM.update(EEPROM_ADDR+2, 0);

  // Read old high score

  unsigned long btm = EEPROM.read(EEPROM_ADDR);
  unsigned long top = EEPROM.read(EEPROM_ADDR+8);
  if (top == 255) {
    top = 0;
  }

  highScore = 10UL * (btm + (top << 8));
  //highScore = top;

//  if (highScore == 0xffffffff) {
//    //eeprom has not been written yet
//    highScore = 0;
//  }
  
  // Two rotations are supported, 
  // The begin() method sets the rotation to 1.
  //oled.setRotation(0);
  //oled.setFont(FONT6X8);

  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ROTATE, INPUT_PULLUP);



  uint8_t p[16];
  npiece = random(8);
  if (npiece == 7) {
    npiece = random(7);
  }
  readPiece(p, npiece);
  active.id = npiece;
  active.piece = p;

  npiece = random(8);
  if (npiece == 7) {
    npiece = random(7);
  }

  readPiece(nextPiece, npiece);
  
  // To clear all the memory, clear both rendering frames:
  oled.clear();
  drawNextPiece();
  writeHighScore();
  oled.switchRenderFrame();
  oled.clear();
  drawNextPiece();
  writeHighScore();

  //read next piece
  readPiece(nextPiece, npiece);

  // 16 pixels wide, each pixel is represented by one bit.
  // higher order bit is on the left, low order bit on the right

  draw();
  oled.switchDisplayFrame();

  oled.on();
}

uint8_t page_pixels[SCREEN_HEIGHT][4] = {{0, 0, 0, 0}};

void draw() {
  //oled.clear();
  uint8_t page = 0;
  
  for (uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
    if (y == SCREEN_HEIGHT - 1) {
      for (uint8_t p = 0; p < 4; p++) {
        page_pixels[y][p] = 0xff;
      }
      continue;
    }
    for (uint8_t p = 0; p < 4; p++) {
      page_pixels[y][p] = 0;
    }

    page_pixels[y][0] = 1;
    page_pixels[y][3] = 0x80;
    
    for (uint8_t x = 1; x < 11; x++) {
      int pos = y * SCREEN_WIDTH + x;
      bool piecePixel = false;
      bool hasPiece = x >= active.x && y >= active.y && x < active.x + 4 && y < active.y + 4;
      if (hasPiece) {
        int ind = (y - active.y) * 4 + (x - active.x);
        piecePixel = active.piece[ind] & 1;
      }
      if (((pixels[pos / 8] >> (pos % 8)) & 1) || piecePixel) {
        uint8_t pix = x * 3 - 2;

        for (uint8_t k = 0; k < 3; k++) {
          uint8_t p = (pix + k) / 8;
          page_pixels[y][p] |= (1 << ((pix + k) % 8));
        }
      }
    }

  }
  for (uint8_t p = 0; p < 4; p++) {
    oled.setCursor(30, p);
    oled.startData();

    oled.sendData(0xff);

    for (uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
      oled.repeatData(page_pixels[y][p], 3);
    }

    oled.endData();
  }
}


void writeHighScore() {
    int divisor = 1;
    for (uint8_t l = 0; l < 4; l++) {
        oled.setCursor(SCREEN_HEIGHT * 3 + 48, l);
        oled.startData();
        
        
        uint8_t n = (highScore / divisor) % 10;
        divisor *= 10;

        uint8_t nx = (highScore / divisor) % 10;
        
        for (uint8_t i = 7; i >= 1; i--) {
          int a = pgm_read_byte(&nums[i + n * 8]) >> l;
          int b = pgm_read_byte(&nums[i + nx * 8]) << (6-l);
          oled.sendData((byte)(a | b) & 0xff);
        }
        oled.endData();
    }
}


void writeScore() {
    int divisor = 1;
    for (uint8_t l = 0; l < 4; l++) {
        oled.setCursor(SCREEN_HEIGHT * 3 + 37, l);
        oled.startData();
        
        
        uint8_t n = (score / divisor) % 10;
        divisor *= 10;

        uint8_t nx = (score / divisor) % 10;
        
        for (uint8_t i = 7; i >= 1; i--) {
          int a = pgm_read_byte(&nums[i + n * 8]) >> l;
          int b = pgm_read_byte(&nums[i + nx * 8]) << (6-l);
          oled.sendData((byte)(a | b) & 0xff);
        }
        oled.endData();
    }
}

void drawNextPiece() {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t i = 0; i < 4; i++) {
      page_pixels[y][i] = 0;
    }
    for (uint8_t x = 4; x < 8; x++) {
      bool piecePixel = false;
      int ind = y * 4 + x - 4;
      piecePixel = nextPiece[ind] & 1;
      
      
      if (piecePixel) {
        uint8_t pix = x * 3 - 2;
        for (uint8_t k = 0; k < 3; k++) {
          uint8_t p = (pix + k) / 8;
          page_pixels[y][p] |= (1 << ((pix + k) % 8));
        }
      }
    }
  }

  
  for (uint8_t p = 0; p < 4; p++) {
    oled.setCursor(5, p);
    oled.startData();
    for (uint8_t y = 0; y < 4; y++) {
      oled.repeatData(page_pixels[y][p], 3);
    }

    oled.endData();
  }
}

unsigned long lastMoved = 0;
unsigned long lastRotated = 0;


bool canMoveDown() {
    for (uint8_t i = 0; i < 4;i++) {
        for (uint8_t j = 0; j < 4;j++) {
            int pos = (active.y + i + 1) * SCREEN_WIDTH + active.x + j;
            int below = (pixels[pos / 8] >> (pos % 8)) & 1;

            if (active.piece[i * 4 + j] && below) {
                return false;
            }
        }
    }
    
    return true;
}

bool canMoveLeft() {
    for (uint8_t i = 0; i < 4;i++) {
        for (uint8_t j = 0; j < 4;j++) {
            int pos = (active.y + i) * SCREEN_WIDTH + active.x + j - 1;
            uint8_t left = (pixels[pos / 8] >> (pos % 8)) & 1;

            if (active.piece[i * 4 + j] && left) {
                return false;
            }
        }
    }
    
    return true;
}
bool canMoveRight() {
    for (uint8_t i = 0; i < 4;i++) {
        for (uint8_t j = 0; j < 4;j++) {
            if (active.piece[i * 4 + j] == 1) {
              int pos = (active.y + i) * SCREEN_WIDTH + active.x + j + 1;
              uint8_t right = (pixels[pos / 8] >> (pos % 8)) & 1;
              if (active.piece[i * 4 + j] && right) {
                return false;
              }
            }
        }
    }
    
    return true;
}

void rotatePiece(uint8_t data[16], uint8_t newData[16]) {
    for(uint8_t i = 0; i < 4; i++) {
        for(uint8_t j = 0; j < 4; j++) {
          newData[j * 4 + 3 - i] = data[i * 4 + j];
        }
    }
}

void getAwait() {
  if (level == 1) {
    await = 640;
  } else if(level == 2) {
    await = 580;
  } else if(level == 3) {
    await = 500;
  } else if(level == 4) {
    await = 440;
  } else if(level == 5) {
    await = 360;
  } else if(level == 6) {
    await = 300;
  } else if(level == 7) {
    await = 220;
  } else if(level == 8) {
    await = 140;
  } else if(level == 9) {
    await = 100;
  } else if(level == 10) {
    await = 80;
  } else if(level == 13) {
    await = 60;
  } else if(level == 16) {
    await = 40;
  } else if(level >= 19) {
    await = 20;
  } else {
    await = 720;
  }
}

void loop() {
  if (madeLine) {
    madeLine = false;
    oled.setInverse(false);
  }
  ms = millis();
  if (lastMoved + 100 < ms) {
    bool rPressed = ((PINB >> PIN_RIGHT) & 1) == 0;
    bool lPressed = ((PINB >> PIN_LEFT) & 1) == 0;
    
    if (rPressed && canMoveRight()) {
      active.x += 1;
    }
    
    if (lPressed && canMoveLeft()) {
      active.x -= 1;
    }
    lastMoved = ms;
  }

  getAwait();

  if (analogRead(0) < 950) {
    await = 20;
  }
  
  if (((PINB >> PIN_ROTATE) & 1) == 0 && lastRotated + 150 < ms) {
    uint8_t np[16];
    uint8_t op[16];
    for (uint8_t l = 0; l < 16; l++) {
      op[l] = active.piece[l];
    }

    //Rotate piece
    rotatePiece(op, np);
    // set rotated piece as active
    active.piece = np;

    int old_x = active.x;

    //check collisions
    
    bool movedLeft = false;
    bool movedRight = false;
    if(!canMoveRight()) {
      active.x -= 1;
      movedLeft = true;
    }
    if(!canMoveLeft()) {
      active.x += 1;
      movedRight = true;
    }

    if (movedLeft && movedRight) {
      // could not rotate
      active.piece = op;
      active.x = old_x;
    } else {
      if (movedLeft) {
        while(!canMoveRight()) {
          active.x -= 1;
        }
        active.x +=1;
      }
      if (movedRight) {
        while(!canMoveLeft()) {
          active.x += 1;
        }
        active.x -=1;
      }
    }
    
    lastRotated = ms;
  }
  
  if (elapsed + await < ms) {
    
    if (canMoveDown()) {
      active.y += 1;
    } else {
      // cannot move down, make piece part of pixels
      for (uint8_t i = 0; i < 4;i++) {
        for (uint8_t j = 0; j < 4;j++) {
          int pos = (active.y + i) * SCREEN_WIDTH + active.x + j;

          if (active.piece[i * 4 + j] == 1) {
            pixels[pos / 8] |= 1 << (pos % 8);
          }
        }
      }

      //check if a line was created
      uint8_t nlines = 0;
      for (uint8_t y = 0; y < SCREEN_HEIGHT - 1; y++) {
        int pos = y * SCREEN_WIDTH / 8;
        
        if (pixels[pos] == 0xFF && pixels[pos + 1] == 0xFF) {
          nlines++;
          madeLine = true;
          // move all pixels above
          
          for (uint8_t y2 = y; y2 >= 2; y2--) {
            int p = y2 * SCREEN_WIDTH / 8;
            
            pixels[p] = pixels[p - 2];
            pixels[p + 1] = pixels[p - 1];
          }
        }
      }

      if (nlines > 0) {
        linesCleared += nlines;
        if (nlines == 1) {
          score += 40 * (level + 1);
        } else if (nlines == 2) {
          score += 100 * (level + 1);
        } else if (nlines == 3) {
          score += 300 * (level + 1);
        } else if (nlines == 4) {
          score += 1200 * (level + 1);
        }
      }

      

      //Check if game over
      if (active.y <=1 ) {
        oled.setInverse(true);
        set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
        sleep_enable();
      
        sleep_mode();     
      }

      if (madeLine) {
        oled.setInverse(true);
        //save high score
        if (score > highScore) {
          highScore = score;
          unsigned long toWrite = score / 10;
          EEPROM.update(EEPROM_ADDR+8, (uint8_t)((toWrite >> 8) & 0xff));
          EEPROM.update(EEPROM_ADDR, (uint8_t)(toWrite & 0xff));
        }
      }
      
      active = { 4, 0, 0, npiece };
      uint8_t p[16];
      readPiece(p, npiece);
      active.piece = p;
      
      npiece = random(8);
      if (npiece == 7 || npiece == active.id) {
        npiece = random(7);
      }

      readPiece(nextPiece, npiece);
      
      

      drawNextPiece();
      writeHighScore();

      oled.switchRenderFrame();
      drawNextPiece();
      writeHighScore();
      oled.switchRenderFrame();
    }
    
    elapsed = ms;
    
    if (linesCleared > (level+1) * 10) {
      level += 1;
    }
  }
  oled.switchRenderFrame();
  
  draw();
  writeScore();
  
  oled.switchDisplayFrame();

}
