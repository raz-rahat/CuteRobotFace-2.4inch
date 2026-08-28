/* ================================================================
   CUTE ROBOT FACE V2 - SMOOTH + GYRO + NO FLICKER

   ESP32-S3 N16R8
   ST7789 240x320
   XPT2046 Touch
   MPU6500 / MPU6050 FAMILY

   TFT:
   SCK      GPIO 13
   MISO     GPIO 12
   MOSI     GPIO 11
   CS       GPIO 10
   DC       GPIO 9
   RST      GPIO 8
   LED      GPIO 5
   TOUCH_CS GPIO 7

   MPU:
   SDA      GPIO 16
   SCL      GPIO 15
   ADDRESS  0x68

   ================================================================ */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

// ================================================================
// PIN CONFIGURATION
// ================================================================

#define TFT_SCK    13
#define TFT_MISO   12
#define TFT_MOSI   11

#define TFT_CS     10
#define TFT_DC      9
#define TFT_RST     8
#define TFT_LED     5

#define TOUCH_CS    7
#define BOOT_BUTTON 0   // ESP32-S3 onboard BOOT button (active LOW)

#define MPU_SDA    16
#define MPU_SCL    15
#define MPU_ADDR   0x68


// ================================================================
// DISPLAY COLOR CONFIGURATION
// ================================================================

// false = Normal color
// true  = Inverted color
const bool DISPLAY_INVERTED = false;


// ================================================================
// TOUCH CALIBRATION
// ================================================================

#define RAW_X_MIN  430
#define RAW_X_MAX  3706

#define RAW_Y_MIN  310
#define RAW_Y_MAX  3679


// ================================================================
// SCREEN SIZE
// ================================================================

#define SCREEN_W 240
#define SCREEN_H 320


// ================================================================
// TFT OBJECT
// ================================================================

Adafruit_ST7789 tft(
  &SPI,
  TFT_CS,
  TFT_DC,
  TFT_RST
);

XPT2046_Touchscreen touch(TOUCH_CS);


// ================================================================
// OFF-SCREEN FRAME BUFFER
//
// This is the important part for reducing screen flicker.
// We draw everything in RAM first, then send one complete frame.
// ================================================================

GFXcanvas16 canvas(
  SCREEN_W,
  SCREEN_H
);


// ================================================================
// COLORS
// ================================================================

#define BG_COLOR    ST77XX_BLACK
#define WHITE       ST77XX_WHITE
#define CYAN        ST77XX_CYAN
#define GREEN       ST77XX_GREEN
#define YELLOW      ST77XX_YELLOW
#define RED         ST77XX_RED

#define BLUE        0x001F
#define PINK        0xF81F
#define ORANGE      0xFD20
#define DARK_GRAY   0x4208


// ================================================================
// EXPRESSIONS
// ================================================================

enum Expression
{
  IDLE,
  HAPPY,
  LOVE,
  CURIOUS,
  SLEEPY,
  ANGRY,
  SURPRISE,
  EXPR_COUNT
};

Expression currentExpr = IDLE;


// ================================================================
// FACE POSITION
// ================================================================

const int leftEyeX  = 60;
const int rightEyeX = 180;
const int eyeY      = 140;

const int eyeW = 80;
const int eyeH = 100;


// ================================================================
// PUPIL ANIMATION
// ================================================================

float pupilX = 0;
float pupilY = 0;

float targetPupilX = 0;
float targetPupilY = 0;


// ================================================================
// MPU / GAZE / REACTION SYSTEM V3
// ================================================================
bool gyroOK = false;

// User tuning values
float TILT_SENS = 0.80f;       // lower = more sensitive
float SHAKE_TRIGGER = 90.0f;   // light shake threshold (deg/s)
float SURPRISE_TRIGGER = 260.0f; // very fast movement only
bool GAZE_INVERT = false;
bool GAZE_SWAP_AXES = false;   // set true if sensor is mounted 90 degrees

float gyroOffsetX = 0;
float gyroOffsetY = 0;
float gyroPupilX = 0;
float gyroPupilY = 0;
float accelX = 0, accelY = 0, accelZ = 1;
float tiltRoll = 0, tiltPitch = 0;
float lastGx = 0, lastGy = 0;
unsigned long lastGyroRead = 0;
unsigned long upsideStart = 0;

// Reaction states
enum Reaction { REACT_NONE, REACT_FEAR, REACT_PANIC, REACT_SURPRISE, REACT_WAKE };
Reaction reaction = REACT_NONE;
unsigned long reactionStart = 0;
int reactionPhase = 0;
int mood = 100;
float faceShakeX = 0, faceShakeY = 0;

// Gaze configuration
float gazeTargetX = 0, gazeTargetY = 0;
float gazeLeadX = 0, gazeLeadY = 0;

// Tuning menu
bool tuningMenu = false;
int tuningItem = 0;
const char* tuningNames[] = {"TILT SENS", "GAZE RANGE", "SHAKE", "INVERT", "SWAP AXES"};
const int TUNING_COUNT = 5;
float gazeRange = 28.0f;
unsigned long lastMenuTouch = 0;

// ================================================================
// BLINK VARIABLES
// ================================================================

float blinkAmount = 0.0;

bool blinking = false;
bool blinkClosing = true;

unsigned long blinkStart = 0;
unsigned long nextBlink = 0;


// ================================================================
// TIMER VARIABLES
// ================================================================

unsigned long lastFrame = 0;
unsigned long lastIdleMove = 0;
unsigned long lastTouch = 0;

// Angry tap detection
int tapCount = 0;
unsigned long tapWindowStart = 0;
unsigned long angryUntil = 0;
const int ANGRY_TAPS = 5;
const unsigned long TAP_WINDOW = 1800;
const unsigned long ANGRY_TIME = 2200;

// BOOT button -> manual FEAR
bool bootWasPressed = false;
unsigned long lastBootPress = 0;

const int FRAME_TIME = 70;  // ~14 FPS


// ================================================================
// MPU REGISTER READ
// ================================================================

uint8_t mpuRead8(uint8_t reg)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0)
  {
    return 0xFF;
  }

  Wire.requestFrom(
    MPU_ADDR,
    (uint8_t)1
  );

  if (Wire.available())
  {
    return Wire.read();
  }

  return 0xFF;
}


// ================================================================
// MPU WRITE
// ================================================================

bool mpuWrite8(
  uint8_t reg,
  uint8_t value
)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}


// ================================================================
// MPU READ 16 BIT
// ================================================================
int16_t mpuRead16(uint8_t reg)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom(MPU_ADDR, (uint8_t)2);
  if (Wire.available() >= 2)
    return ((int16_t)Wire.read() << 8) | Wire.read();
  return 0;
}

void readMPU(int16_t &ax, int16_t &ay, int16_t &az,
             int16_t &gx, int16_t &gy)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return;
  Wire.requestFrom(MPU_ADDR, (uint8_t)10);
  if (Wire.available() >= 10) {
    ax = ((int16_t)Wire.read() << 8) | Wire.read();
    ay = ((int16_t)Wire.read() << 8) | Wire.read();
    az = ((int16_t)Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read(); // temperature
    gx = ((int16_t)Wire.read() << 8) | Wire.read();
    gy = ((int16_t)Wire.read() << 8) | Wire.read();
  }
}

// ================================================================
// MPU INITIALIZATION
// ================================================================
bool initMPU()
{
  Serial.println("\nMPU INITIALIZATION...");
  Wire.begin(MPU_SDA, MPU_SCL);
  Wire.setClock(400000);
  delay(100);
  uint8_t who = mpuRead8(0x75);
  Serial.printf("WHO_AM_I = 0x%02X\n", who);
  if (who == 0xFF || who == 0x00) {
    Serial.println("MPU NOT DETECTED! Face will run without gyro features.");
    return false;
  }
  if (!mpuWrite8(0x6B, 0x00)) return false;
  delay(80);
  mpuWrite8(0x1B, 0x08); // gyro ±500 dps
  mpuWrite8(0x1C, 0x00); // accel ±2G
  mpuWrite8(0x1A, 0x03); // DLPF
  Serial.println("MPU OK!");
  return true;
}

// ================================================================
// GYRO CALIBRATION
// ================================================================
void calibrateGyro()
{
  if (!gyroOK) return;
  Serial.println("Keep robot still - Calibrating...");
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 120; i++) {
    sumX += mpuRead16(0x43);
    sumY += mpuRead16(0x45);
    delay(4);
  }
  gyroOffsetX = (float)sumX / 120.0f;
  gyroOffsetY = (float)sumY / 120.0f;
  Serial.printf("Gyro offsets: %.1f, %.1f\n", gyroOffsetX, gyroOffsetY);
}

void setReaction(Reaction r)
{
  reaction = r;
  reactionStart = millis();
  reactionPhase = 0;
  faceShakeX = faceShakeY = 0;
}

void finishReaction()
{
  reaction = REACT_NONE;
  currentExpr = IDLE;
  faceShakeX = faceShakeY = 0;
}

// ================================================================
// UPDATE MPU + TILT GAZE + SHAKE/FALL
// ================================================================
void updateGyro()
{
  if (!gyroOK) return;
  unsigned long now = millis();
  if (now - lastGyroRead < 20) return;
  float dt = (now - lastGyroRead) / 1000.0f;
  lastGyroRead = now;
  if (dt <= 0 || dt > 0.2f) dt = 0.02f;

  int16_t axr=0, ayr=0, azr=16384, gxr=0, gyr=0;
  readMPU(axr, ayr, azr, gxr, gyr);
  float gx = (gxr - gyroOffsetX) / 65.5f; // ±500 dps
  float gy = (gyr - gyroOffsetY) / 65.5f;
  accelX = axr / 16384.0f;
  accelY = ayr / 16384.0f;
  accelZ = azr / 16384.0f;

  // Roll / pitch from gravity. This gives stable gaze while tilted.
  tiltRoll = atan2f(accelY, accelZ) * 57.2958f;
  tiltPitch = atan2f(-accelX, sqrtf(accelY*accelY + accelZ*accelZ)) * 57.2958f;

  float axisX = tiltRoll;
  float axisY = tiltPitch;
  if (GAZE_SWAP_AXES) { float t=axisX; axisX=axisY; axisY=t; }
  if (GAZE_INVERT) { axisX=-axisX; axisY=-axisY; }

  // TILT_SENS: 1.0 is intentionally more sensitive than the old gyro-only system.
  gazeTargetX = constrain(axisX / TILT_SENS, -gazeRange, gazeRange);
  gazeTargetY = constrain(axisY / TILT_SENS, -gazeRange*0.78f, gazeRange*0.78f);

  // Fast rotation lead: eyes react slightly before the body settles.
  float leadX = constrain(gy * 0.075f, -11.0f, 11.0f);
  float leadY = constrain(-gx * 0.075f, -9.0f, 9.0f);
  if (GAZE_SWAP_AXES) { float t=leadX; leadX=leadY; leadY=t; }
  if (GAZE_INVERT) { leadX=-leadX; leadY=-leadY; }
  gazeLeadX += (leadX - gazeLeadX) * 0.38f;
  gazeLeadY += (leadY - gazeLeadY) * 0.38f;

  gyroPupilX += ((gazeTargetX + gazeLeadX) - gyroPupilX) * 0.34f;
  gyroPupilY += ((gazeTargetY + gazeLeadY) - gyroPupilY) * 0.34f;

  // Upside-down hold -> tuning menu.
  bool upsideDown = (accelZ < -0.72f);
  if (upsideDown) {
    if (upsideStart == 0) upsideStart = now;
    if (!tuningMenu && now - upsideStart >= 3000) {
      tuningMenu = true;
      tuningItem = 0;
      currentExpr = IDLE;
    }
  } else upsideStart = 0;

  // If menu is open, sensor reactions are paused.
  if (tuningMenu) return;

  float amag = sqrtf(accelX*accelX + accelY*accelY + accelZ*accelZ);
  float gyroMag = sqrtf(gx*gx + gy*gy);

  // --------------------------------------------------------------
  // AUTOMATIC FEAR / PANIC / SURPRISE FROM MOTION IS DISABLED.
  // Movement is used only for gaze. FEAR is now triggered by the
  // onboard BOOT button so the robot can be picked up normally.
  // --------------------------------------------------------------
}

// ================================================================
// REACTION UPDATE
// ================================================================
void updateReaction()
{
  if (reaction == REACT_NONE) return;
  unsigned long e = millis() - reactionStart;

  if (reaction == REACT_FEAR) {
    if (e >= 2400) { reactionPhase=1; reactionStart=millis(); reaction=REACT_WAKE; }
  } else if (reaction == REACT_PANIC) {
    if (e < 1700) return;
    reactionPhase=1; reactionStart=millis(); reaction=REACT_FEAR;
  } else if (reaction == REACT_SURPRISE) {
    if (e >= 900) finishReaction();
  } else if (reaction == REACT_WAKE) {
    if (e >= 850) finishReaction();
  }
}

// ================================================================
// TUNING MENU
// ================================================================
void drawTuningMenu()
{
  canvas.fillScreen(BG_COLOR);
  canvas.setTextColor(CYAN);
  canvas.setTextSize(2);
  canvas.setCursor(34, 18); canvas.print("GAZE TUNING");
  canvas.setTextSize(1);
  canvas.setCursor(25, 43); canvas.print("UP/DOWN: item   LEFT/RIGHT: value");

  for (int i=0;i<TUNING_COUNT;i++) {
    int y=70+i*38;
    if (i==tuningItem) canvas.fillRoundRect(12,y-5,216,30,7,DARK_GRAY);
    canvas.setTextColor(i==tuningItem?WHITE:CYAN);
    canvas.setCursor(20,y); canvas.print(tuningNames[i]);
    canvas.setCursor(145,y);
    if(i==0) canvas.printf("%.2f",TILT_SENS);
    else if(i==1) canvas.printf("%.0f",gazeRange);
    else if(i==2) canvas.printf("%.1f",SHAKE_TRIGGER);
    else if(i==3) canvas.print(GAZE_INVERT?"ON":"OFF");
    else canvas.print(GAZE_SWAP_AXES?"ON":"OFF");
  }
  canvas.setTextColor(YELLOW);
  canvas.setCursor(34,278); canvas.print("Touch: row select / +/-");
  canvas.setCursor(50,296); canvas.print("CENTER = EXIT");
}

void handleTuningTouch(int x,int y)
{
  unsigned long now=millis();
  if(now-lastMenuTouch<220) return;
  lastMenuTouch=now;
  if(y>60 && y<265) {
    int idx=(y-60)/38;
    if(idx>=0 && idx<TUNING_COUNT) {
      tuningItem=idx;
      if(x<80) {
        if(idx==0) TILT_SENS=max(0.50f,TILT_SENS-0.10f);
        else if(idx==1) gazeRange=max(8.0f,gazeRange-1);
        else if(idx==2) SHAKE_TRIGGER=max(1.0f,SHAKE_TRIGGER-0.5f);
        else if(idx==3) GAZE_INVERT=!GAZE_INVERT;
        else GAZE_SWAP_AXES=!GAZE_SWAP_AXES;
      } else if(x>160) {
        if(idx==0) TILT_SENS=min(4.0f,TILT_SENS+0.10f);
        else if(idx==1) gazeRange=min(30.0f,gazeRange+1);
        else if(idx==2) SHAKE_TRIGGER=min(60.0f,SHAKE_TRIGGER+0.5f);
        else if(idx==3) GAZE_INVERT=!GAZE_INVERT;
        else GAZE_SWAP_AXES=!GAZE_SWAP_AXES;
      }
    }
  } else if(y>275) {
    tuningMenu=false;
    upsideStart=0;
  }
}

// ================================================================
// TOUCH MAP
// ================================================================

void mapTouch(
  int rawX,
  int rawY,
  int &x,
  int &y
)
{
  x = map(
    rawX,
    RAW_X_MIN,
    RAW_X_MAX,
    0,
    SCREEN_W - 1
  );

  y = map(
    rawY,
    RAW_Y_MIN,
    RAW_Y_MAX,
    0,
    SCREEN_H - 1
  );

  x = constrain(
    x,
    0,
    SCREEN_W - 1
  );

  y = constrain(
    y,
    0,
    SCREEN_H - 1
  );
}


// ================================================================
// DRAW ROUND EYE
// ================================================================

void drawRoundEye(
  int cx,
  int cy,
  int ew,
  int eh,
  float px,
  float py,
  uint16_t color,
  float openAmount
)
{
  // Subtle eyeball drift: the white eye itself follows the gaze a little,
  // while the pupil moves more. This creates a more natural eyeball effect.
  int eyeDriftX = constrain((int)(px * 0.12f), -4, 4);
  int eyeDriftY = constrain((int)(py * 0.10f), -3, 3);
  cx += eyeDriftX;
  cy += eyeDriftY;

  int visibleH =
    max(
      4,
      (int)(
        eh * openAmount
      )
    );

  int topY =
    cy - visibleH / 2;

  int radius =
    min(
      ew,
      visibleH
    ) / 2;

  canvas.fillRoundRect(
    cx - ew / 2,
    topY,
    ew,
    visibleH,
    radius,
    color
  );

  if (visibleH > 25)
  {
    int pupilSize =
      min(
        20,
        visibleH / 3
      );

    int pX =
      cx + (int)px;

    int pY =
      cy + (int)py;

    int limitX =
      ew / 2 -
      pupilSize -
      4;

    int limitY =
      visibleH / 2 -
      pupilSize -
      4;

    pX = constrain(
      pX,
      cx - limitX,
      cx + limitX
    );

    pY = constrain(
      pY,
      cy - limitY,
      cy + limitY
    );

    // Pupil
    canvas.fillCircle(
      pX,
      pY,
      pupilSize,
      BG_COLOR
    );

    // Eye shine
    canvas.fillCircle(
      pX - pupilSize / 3,
      pY - pupilSize / 3,
      max(
        2,
        pupilSize / 4
      ),
      WHITE
    );
  }
}


// ================================================================
// HAPPY EYE
// ================================================================

void drawHappyEye(
  int cx,
  int cy,
  int width,
  int height,
  uint16_t color
)
{
  for (int i = 0; i < 7; i++)
  {
    canvas.drawLine(
      cx - width / 2,
      cy - height / 2 + i,
      cx,
      cy + height / 2 + i,
      color
    );

    canvas.drawLine(
      cx,
      cy + height / 2 + i,
      cx + width / 2,
      cy - height / 2 + i,
      color
    );
  }
}


// ================================================================
// SLEEPY EYE
// ================================================================

void drawSleepyEye(
  int cx,
  int cy,
  int width,
  uint16_t color
)
{
  for (int i = 0; i < 4; i++)
  {
    canvas.drawLine(
      cx - width / 2,
      cy + i,
      cx + width / 2,
      cy + i,
      color
    );
  }
}


// ================================================================
// HEART
// ================================================================

void drawHeart(
  int cx,
  int cy,
  int size,
  uint16_t color
)
{
  int r =
    size / 4;

  canvas.fillCircle(
    cx - r,
    cy - r / 2,
    r,
    color
  );

  canvas.fillCircle(
    cx + r,
    cy - r / 2,
    r,
    color
  );

  canvas.fillTriangle(
    cx - size / 2,
    cy,
    cx + size / 2,
    cy,
    cx,
    cy + size / 2,
    color
  );
}


// ================================================================
// SMILE
// ================================================================

void drawSmile(
  int mx,
  int my,
  uint16_t color
)
{
  for (int i = 0; i < 4; i++)
  {
    canvas.drawLine(
      mx - 22,
      my - 8 + i,
      mx,
      my + 10 + i,
      color
    );

    canvas.drawLine(
      mx,
      my + 10 + i,
      mx + 22,
      my - 8 + i,
      color
    );
  }
}


// ================================================================
// BLUSH
// ================================================================

void drawBlush()
{
  for (int i = 0; i < 3; i++)
  {
    canvas.drawLine(
      leftEyeX - 45,
      eyeY + 60 + i * 4,
      leftEyeX - 18,
      eyeY + 48 + i * 4,
      PINK
    );

    canvas.drawLine(
      rightEyeX + 18,
      eyeY + 48 + i * 4,
      rightEyeX + 45,
      eyeY + 60 + i * 4,
      PINK
    );
  }
}


// ================================================================
// DRAW ANGRY EYE
// ================================================================

void drawAngryEye(
  int cx,
  int cy,
  int ew,
  int eh,
  float px,
  float py,
  uint16_t color,
  float openAmount,
  bool isLeft
)
{
  drawRoundEye(
    cx,
    cy,
    ew,
    eh,
    px,
    py,
    color,
    openAmount
  );

  if (openAmount > 0.4)
  {
    if (isLeft)
    {
      canvas.fillTriangle(
        cx - ew / 2,
        cy - eh / 2,
        cx + ew / 2,
        cy - eh / 2,
        cx - ew / 2,
        cy - eh / 6,
        BG_COLOR
      );
    }
    else
    {
      canvas.fillTriangle(
        cx - ew / 2,
        cy - eh / 2,
        cx + ew / 2,
        cy - eh / 2,
        cx + ew / 2,
        cy - eh / 6,
        BG_COLOR
      );
    }
  }
}


// ================================================================
// DRAW SURPRISE MOUTH
// ================================================================

void drawSurpriseMouth(
  int mx,
  int my,
  uint16_t color
)
{
  canvas.fillCircle(
    mx,
    my,
    16,
    color
  );

  canvas.fillCircle(
    mx,
    my,
    9,
    BG_COLOR
  );
}


// ================================================================
// DRAW BIG HAPPY MOUTH
// ================================================================

void drawHappyMouth(
  int mx,
  int my,
  uint16_t color
)
{
  canvas.fillRoundRect(
    mx - 32,
    my - 8,
    64,
    28,
    14,
    color
  );

  canvas.fillRoundRect(
    mx - 25,
    my - 7,
    50,
    16,
    8,
    BG_COLOR
  );
}


// ================================================================
// DRAW FACE INTO CANVAS
// ================================================================

void drawFace()
{
  if (tuningMenu) { drawTuningMenu(); return; }

  canvas.fillScreen(BG_COLOR);

  // Reaction animation has priority over normal expressions.
  if (reaction != REACT_NONE) {
    float t=(millis()-reactionStart)/1000.0f;
    if(reaction==REACT_FEAR || reaction==REACT_WAKE) {
      float shakeX=sin(millis()*0.11f)*6.0f;
      float shakeY=cos(millis()*0.15f)*5.0f;
      int cy=eyeY+(int)shakeY;
      drawRoundEye(leftEyeX+(int)shakeX,cy,104,142,0,0,CYAN,1.0f);
      drawRoundEye(rightEyeX+(int)shakeX,cy,104,142,0,0,CYAN,1.0f);
      // tiny pupils = fear
      canvas.fillCircle(leftEyeX+(int)shakeX,cy+(int)shakeY,5,BG_COLOR);
      canvas.fillCircle(rightEyeX+(int)shakeX,cy+(int)shakeY,5,BG_COLOR);
      if(reaction==REACT_WAKE && t>0.55f) drawHappyMouth(SCREEN_W/2,255,GREEN);
      else drawSurpriseMouth(SCREEN_W/2,255,CYAN);
      return;
    }
    if(reaction==REACT_PANIC) {
      float sx=sin(millis()*0.25f)*10.0f;
      float sy=cos(millis()*0.31f)*7.0f;
      int cy=eyeY+(int)sy;
      drawRoundEye(leftEyeX+(int)sx,cy,118,155,0,0,YELLOW,1.0f);
      drawRoundEye(rightEyeX+(int)sx,cy,118,155,0,0,YELLOW,1.0f);
      drawSurpriseMouth(SCREEN_W/2,255,YELLOW);
      return;
    }
    if(reaction==REACT_SURPRISE) {
      drawRoundEye(leftEyeX,eyeY,110,150,0,0,YELLOW,1.0f);
      drawRoundEye(rightEyeX,eyeY,110,150,0,0,YELLOW,1.0f);
      drawSurpriseMouth(SCREEN_W/2,255,YELLOW);
      return;
    }
  }

  float openAmount =
    1.0 -
    blinkAmount;

  float breathe =
    sin(
      millis() / 900.0
    ) * 2.0;

  int cy =
    eyeY +
    (int)breathe;

  // Final eye position:
  // idle movement + gyro movement
  float finalPupilX =
    pupilX +
    gyroPupilX;

  float finalPupilY =
    pupilY +
    gyroPupilY;

  int mouthX =
    SCREEN_W / 2;

  int mouthY =
    255;


  // ==============================================================
  // IDLE
  // ==============================================================

  if (currentExpr == IDLE)
  {
    drawRoundEye(
      leftEyeX,
      cy,
      eyeW,
      eyeH,
      finalPupilX,
      finalPupilY,
      CYAN,
      openAmount
    );

    drawRoundEye(
      rightEyeX,
      cy,
      eyeW,
      eyeH,
      finalPupilX,
      finalPupilY,
      CYAN,
      openAmount
    );

    drawSmile(
      mouthX,
      mouthY,
      CYAN
    );
  }


  // ==============================================================
  // HAPPY
  // ==============================================================

  else if (currentExpr == HAPPY)
  {
    drawHappyEye(
      leftEyeX,
      cy,
      72,
      30,
      GREEN
    );

    drawHappyEye(
      rightEyeX,
      cy,
      72,
      30,
      GREEN
    );

    drawBlush();

    drawHappyMouth(
      mouthX,
      mouthY,
      GREEN
    );
  }


  // ==============================================================
  // LOVE
  // ==============================================================

  else if (currentExpr == LOVE)
  {
    drawHeart(
      leftEyeX,
      cy,
      60,
      PINK
    );

    drawHeart(
      rightEyeX,
      cy,
      60,
      PINK
    );

    drawBlush();

    drawSmile(
      mouthX,
      mouthY,
      PINK
    );

    drawHeart(
      32,
      50,
      18,
      PINK
    );

    drawHeart(
      205,
      70,
      15,
      PINK
    );
  }


  // ==============================================================
  // CURIOUS
  // ==============================================================

  else if (currentExpr == CURIOUS)
  {
    drawRoundEye(
      leftEyeX,
      cy,
      70,
      90,
      finalPupilX + 8,
      finalPupilY,
      BLUE,
      openAmount
    );

    drawRoundEye(
      rightEyeX,
      cy - 10,
      92,
      125,
      finalPupilX + 12,
      finalPupilY - 5,
      BLUE,
      openAmount
    );

    drawSmile(
      mouthX,
      mouthY,
      BLUE
    );
  }


  // ==============================================================
  // SLEEPY
  // ==============================================================

  else if (currentExpr == SLEEPY)
  {
    drawSleepyEye(
      leftEyeX,
      cy,
      65,
      BLUE
    );

    drawSleepyEye(
      rightEyeX,
      cy,
      65,
      BLUE
    );

    canvas.drawLine(
      mouthX - 15,
      mouthY,
      mouthX + 15,
      mouthY,
      BLUE
    );

    canvas.setTextColor(
      BLUE
    );

    canvas.setTextSize(2);

    canvas.setCursor(
      185,
      35
    );

    canvas.print(
      "Z"
    );

    canvas.setTextSize(1);

    canvas.setCursor(
      205,
      20
    );

    canvas.print(
      "z"
    );
  }


  // ==============================================================
  // ANGRY
  // ==============================================================

  else if (currentExpr == ANGRY)
  {
    drawAngryEye(
      leftEyeX,
      cy,
      eyeW,
      85,
      finalPupilX,
      finalPupilY + 5,
      RED,
      openAmount,
      true
    );

    drawAngryEye(
      rightEyeX,
      cy,
      eyeW,
      85,
      finalPupilX,
      finalPupilY + 5,
      RED,
      openAmount,
      false
    );

    canvas.drawLine(
      mouthX - 25,
      mouthY + 7,
      mouthX,
      mouthY - 4,
      RED
    );

    canvas.drawLine(
      mouthX,
      mouthY - 4,
      mouthX + 25,
      mouthY + 7,
      RED
    );
  }


  // ==============================================================
  // SURPRISE
  // ==============================================================

  else if (currentExpr == SURPRISE)
  {
    drawRoundEye(
      leftEyeX,
      cy,
      95,
      135,
      finalPupilX * 0.3,
      finalPupilY * 0.3,
      YELLOW,
      1.0
    );

    drawRoundEye(
      rightEyeX,
      cy,
      95,
      135,
      finalPupilX * 0.3,
      finalPupilY * 0.3,
      YELLOW,
      1.0
    );

    drawSurpriseMouth(
      mouthX,
      mouthY,
      YELLOW
    );
  }
}


// ================================================================
// SEND COMPLETE FRAME TO TFT
//
// This avoids visible fillScreen flickering.
// ================================================================

void showFrame()
{
  tft.drawRGBBitmap(
    0,
    0,
    canvas.getBuffer(),
    SCREEN_W,
    SCREEN_H
  );
}


// ================================================================
// UPDATE BLINK
// ================================================================

void updateBlink()
{
  unsigned long now =
    millis();

  if (
    currentExpr == HAPPY ||
    currentExpr == LOVE ||
    currentExpr == SLEEPY
  )
  {
    blinkAmount = 0;
    blinking = false;

    return;
  }


  if (!blinking)
  {
    if (now >= nextBlink)
    {
      blinking = true;
      blinkClosing = true;
      blinkStart = now;
    }

    return;
  }


  unsigned long elapsed =
    now - blinkStart;


  if (blinkClosing)
  {
    blinkAmount =
      constrain(
        elapsed / 90.0,
        0.0,
        1.0
      );

    if (blinkAmount >= 1.0)
    {
      blinkClosing = false;
      blinkStart = now;
    }
  }
  else
  {
    blinkAmount =
      1.0 -
      constrain(
        elapsed / 120.0,
        0.0,
        1.0
      );

    if (blinkAmount <= 0)
    {
      blinkAmount = 0;

      blinking = false;

      nextBlink =
        now +
        random(
          1800,
          4500
        );
    }
  }
}


// ================================================================
// UPDATE IDLE EYES
// ================================================================

void updateIdleEyes()
{
  unsigned long now=millis();
  if(now-lastIdleMove > (unsigned long)random(2000,4001)) {
    lastIdleMove=now;
    targetPupilX=random(-12,13);
    targetPupilY=random(-6,7);
  }
  pupilX += (targetPupilX-pupilX)*0.045f;
  pupilY += (targetPupilY-pupilY)*0.045f;
}

// ================================================================
// BOOT BUTTON -> FEAR
// Press the onboard BOOT button once to trigger the fear reaction.
// ================================================================
void handleBootButton()
{
  bool pressed = (digitalRead(BOOT_BUTTON) == LOW);
  unsigned long now = millis();

  if (pressed && !bootWasPressed && (now - lastBootPress > 300))
  {
    lastBootPress = now;
    mood = max(0, mood - 5);
    if (reaction == REACT_NONE)
    {
      setReaction(REACT_FEAR);
    }
  }

  bootWasPressed = pressed;
}

// ================================================================
// REPEATED TOUCH -> ANGRY
// Five quick taps anywhere on the screen make the robot angry.
// ================================================================
void updateAngryTapTimer()
{
  unsigned long now = millis();

  if (tapWindowStart != 0 && now - tapWindowStart > TAP_WINDOW)
  {
    tapCount = 0;
    tapWindowStart = 0;
  }

  if (angryUntil != 0 && now >= angryUntil)
  {
    angryUntil = 0;
    if (reaction == REACT_NONE) currentExpr = IDLE;
  }
}

// ================================================================
// HANDLE TOUCH
// ================================================================

void handleTouch()
{
  if (!touch.touched()) return;


  unsigned long now =
    millis();

  if (
    now - lastTouch < 600
  )
  {
    return;
  }

  TS_Point p =
    touch.getPoint();

  int x;
  int y;

  mapTouch(
    p.x,
    p.y,
    x,
    y
  );

  lastTouch = now;

  if (tuningMenu) {
    handleTuningTouch(x,y);
    return;
  }

  // Count rapid taps. Five taps = ANGRY expression.
  if (tapWindowStart == 0 || now - tapWindowStart > TAP_WINDOW)
  {
    tapWindowStart = now;
    tapCount = 1;
  }
  else
  {
    tapCount++;
  }

  if (tapCount >= ANGRY_TAPS)
  {
    currentExpr = ANGRY;
    angryUntil = now + ANGRY_TIME;
    tapCount = 0;
    tapWindowStart = 0;
    Serial.println("ANGRY: repeated taps");
    return;
  }

  // LEFT
  if (x < 80)
  {
    int e =
      (int)currentExpr - 1;

    if (e < 0)
    {
      e =
        EXPR_COUNT - 1;
    }

    currentExpr =
      (Expression)e;
  }


  // RIGHT
  else if (x > 160)
  {
    int e =
      ((int)currentExpr + 1) %
      EXPR_COUNT;

    currentExpr =
      (Expression)e;
  }


  // CENTER
  else
  {
    currentExpr =
      (Expression)random(
        0,
        EXPR_COUNT
      );
  }


  Serial.print(
    "Touch: "
  );

  Serial.print(x);

  Serial.print(
    ", "
  );

  Serial.println(y);
}


// ================================================================
// SETUP
// ================================================================

void setup()
{
  pinMode(BOOT_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println(
    "================================="
  );

  Serial.println(
    "CUTE ROBOT FACE V2"
  );

  Serial.println(
    "SMOOTH + GYRO"
  );

  Serial.println(
    "================================="
  );


  // BACKLIGHT
  pinMode(
    TFT_LED,
    OUTPUT
  );

  digitalWrite(
    TFT_LED,
    HIGH
  );


  // SPI
  SPI.begin(
    TFT_SCK,
    TFT_MISO,
    TFT_MOSI,
    TFT_CS
  );


  // TFT
  tft.init(
    SCREEN_W,
    SCREEN_H
  );

  tft.setRotation(2);


  // IMPORTANT: COLOR INVERSION FIX
  tft.invertDisplay(
    DISPLAY_INVERTED
  );

  tft.fillScreen(
    BG_COLOR
  );

  Serial.println(
    "TFT: OK"
  );


  // TOUCH
  if (touch.begin())
  {
    touch.setRotation(2);

    Serial.println(
      "Touch: OK"
    );
  }
  else
  {
    Serial.println(
      "Touch: NOT DETECTED"
    );
  }


  // MPU
  gyroOK =
    initMPU();

  if (gyroOK)
  {
    calibrateGyro();
  }


  // RANDOM
  randomSeed(
    micros()
  );

  nextBlink =
    millis() +
    2000;


  // START SCREEN
  canvas.fillScreen(
    BG_COLOR
  );

  canvas.setTextColor(
    CYAN
  );

  canvas.setTextSize(2);

  canvas.setCursor(
    35,
    145
  );

  canvas.println(
    "HELLO!"
  );

  showFrame();

  delay(1000);


  Serial.println("ROBOT READY!");
  Serial.printf("TILT_SENS=%.2f SHAKE_TRIGGER=%.1f\n", TILT_SENS, SHAKE_TRIGGER);
}


// ================================================================
// LOOP
// ================================================================

void loop()
{
  handleBootButton();
  updateGyro();
  updateReaction();
  updateAngryTapTimer();

  handleTouch();

  unsigned long now =
    millis();

  if (
    now - lastFrame >= FRAME_TIME
  )
  {
    lastFrame = now;

    updateBlink();

    updateIdleEyes();

    drawFace();

    showFrame();
  }
}