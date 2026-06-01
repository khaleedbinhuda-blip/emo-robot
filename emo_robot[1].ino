#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#include "BluetoothSerial.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TOUCH 4
#define SDA 21
#define SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RoboEyes<Adafruit_SSD1306> roboEyes(display);
BluetoothSerial SerialBT;

enum Emotion {
  EMO_DEFAULT,
  EMO_HAPPY,
  EMO_ANGRY,
  EMO_SAD,
  EMO_EXCITED,
  EMO_TIRED,
  EMO_CONFUSED,
  EMO_LOVE
};

enum MoodSource { SRC_DEFAULT, SRC_BLUETOOTH, SRC_TOUCH };

Emotion currentEmotion = EMO_ANGRY;
MoodSource activeMoodSource = SRC_DEFAULT;

bool wasTouched = false;
unsigned long touchStartTime = 0;
const unsigned long LONG_TOUCH_MS = 1000;
const unsigned long VERY_LONG_TOUCH_MS = 3000;

bool shaking = false;
unsigned long shakeStart = 0, shakeLastStep = 0;
const unsigned long SHAKE_DURATION = 600, SHAKE_STEP = 80;
int shakePhase = 0;

bool bouncing = false;
unsigned long bounceStart = 0, bounceLastStep = 0;
const unsigned long BOUNCE_DURATION = 800, BOUNCE_STEP = 100;
int bouncePhase = 0;

bool tilting = false;
unsigned long tiltStart = 0, tiltLastStep = 0;
const unsigned long TILT_DURATION = 1000, TILT_STEP = 200;
int tiltPhase = 0;

void startShake() { shaking=true; bouncing=false; tilting=false; shakeStart=shakeLastStep=millis(); shakePhase=0; }
void startBounce() { bouncing=true; shaking=false; tilting=false; bounceStart=bounceLastStep=millis(); bouncePhase=0; }
void startTilt() { tilting=true; shaking=false; bouncing=false; tiltStart=tiltLastStep=millis(); tiltPhase=0; }
void stopAllAnimations() { shaking=false; bouncing=false; tilting=false; }

void updateShake() {
  if (!shaking) return;
  unsigned long now = millis();
  if (now - shakeStart > SHAKE_DURATION) { shaking=false; roboEyes.setPosition(DEFAULT); return; }
  if (now - shakeLastStep > SHAKE_STEP) {
    shakeLastStep = now;
    switch (shakePhase % 4) {
      case 0: roboEyes.setPosition(NE); break; case 1: roboEyes.setPosition(NW); break;
      case 2: roboEyes.setPosition(N);  break; case 3: roboEyes.setPosition(DEFAULT); break;
    }
    shakePhase++;
  }
}

void updateBounce() {
  if (!bouncing) return;
  unsigned long now = millis();
  if (now - bounceStart > BOUNCE_DURATION) { bouncing=false; roboEyes.setPosition(DEFAULT); return; }
  if (now - bounceLastStep > BOUNCE_STEP) {
    bounceLastStep = now;
    switch (bouncePhase % 2) { case 0: roboEyes.setPosition(N); break; case 1: roboEyes.setPosition(DEFAULT); break; }
    bouncePhase++;
  }
}

void updateTilt() {
  if (!tilting) return;
  unsigned long now = millis();
  if (now - tiltStart > TILT_DURATION) { tilting=false; roboEyes.setPosition(DEFAULT); return; }
  if (now - tiltLastStep > TILT_STEP) {
    tiltLastStep = now;
    switch (tiltPhase % 4) {
      case 0: roboEyes.setPosition(NE); break; case 1: roboEyes.setPosition(DEFAULT); break;
      case 2: roboEyes.setPosition(NW); break; case 3: roboEyes.setPosition(DEFAULT); break;
    }
    tiltPhase++;
  }
}

void applyEmotion(Emotion emo) {
  if (emo == currentEmotion) return;
  currentEmotion = emo;
  stopAllAnimations();
  switch (emo) {
    case EMO_HAPPY:   roboEyes.setMood(HAPPY); roboEyes.setAutoblinker(ON,1,1); roboEyes.setIdleMode(OFF); startShake(); break;
    case EMO_ANGRY:   roboEyes.setMood(ANGRY); roboEyes.setAutoblinker(OFF); roboEyes.setIdleMode(OFF); roboEyes.setPosition(DEFAULT); break;
    case EMO_SAD:     roboEyes.setMood(TIRED); roboEyes.setAutoblinker(ON,5,4); roboEyes.setIdleMode(OFF); roboEyes.setPosition(S); break;
    case EMO_EXCITED: roboEyes.setMood(HAPPY); roboEyes.setAutoblinker(ON,1,1); roboEyes.setIdleMode(OFF); startBounce(); break;
    case EMO_TIRED:   roboEyes.setMood(TIRED); roboEyes.setAutoblinker(ON,6,5); roboEyes.setIdleMode(ON,5,3); roboEyes.setPosition(DEFAULT); break;
    case EMO_CONFUSED:roboEyes.setMood(DEFAULT); roboEyes.setAutoblinker(ON,2,1); roboEyes.setIdleMode(OFF); startTilt(); break;
    case EMO_LOVE:    roboEyes.setMood(HAPPY); roboEyes.setAutoblinker(ON,2,2); roboEyes.setIdleMode(OFF); roboEyes.setPosition(N); break;
    default:          roboEyes.setMood(DEFAULT); roboEyes.setAutoblinker(ON,3,2); roboEyes.setIdleMode(ON,2,2); roboEyes.setPosition(DEFAULT); break;
  }
  Serial.printf("Emotion: %d\n", (int)emo);
}

void resetToDefault() { currentEmotion = EMO_ANGRY; applyEmotion(EMO_DEFAULT); }

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH, INPUT);
  Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { Serial.println("SSD1306 failed"); while(true); }
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  applyEmotion(EMO_DEFAULT);
  SerialBT.begin("DoraBrain");
  Serial.println("DoraBrain ready! h=happy a=angry s=sad e=excited t=tired c=confused l=love d=default");
}

void loop() {
  bool touched = digitalRead(TOUCH);
  unsigned long now = millis();

  if (touched) {
    activeMoodSource = SRC_TOUCH;
    if (!wasTouched) { touchStartTime=now; wasTouched=true; resetToDefault(); applyEmotion(EMO_HAPPY); }
    unsigned long held = now - touchStartTime;
    if (held >= VERY_LONG_TOUCH_MS && currentEmotion != EMO_LOVE) applyEmotion(EMO_LOVE);
    else if (held >= LONG_TOUCH_MS && currentEmotion == EMO_HAPPY) applyEmotion(EMO_ANGRY);
  } else {
    if (wasTouched) { wasTouched=false; touchStartTime=0; activeMoodSource=SRC_DEFAULT; resetToDefault(); }
    if (Serial.available()) {
      char cmd = Serial.read();
      if (cmd=='\n'||cmd=='\r') return;
      switch(cmd) {
        case 'h': applyEmotion(EMO_HAPPY);    break; case 'a': applyEmotion(EMO_ANGRY);   break;
        case 's': applyEmotion(EMO_SAD);      break; case 'e': applyEmotion(EMO_EXCITED); break;
        case 't': applyEmotion(EMO_TIRED);    break; case 'c': applyEmotion(EMO_CONFUSED);break;
        case 'l': applyEmotion(EMO_LOVE);     break; case 'd': resetToDefault();          break;
      }
    }
  }

  updateShake(); updateBounce(); updateTilt();
  roboEyes.update();
}
