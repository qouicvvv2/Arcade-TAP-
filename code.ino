#include <Wire.h>

#define BUZZER 3
#define START_BTN 12

const int ledPins[5] = {2, 4, 5, 6, 7};
const int btnPins[5] = {8, 9, 10, 11, A0}; 

int lives = 3;
int score = 0;
bool gameActive = false;

// أزمنة الجولات (بالمللي ثانية)
const int TIME_ROUND_1 = 1000; // 1 ثانية
const int TIME_ROUND_2 = 550;  // 0.55 ثانية
const int TIME_ROUND_3 = 450;  // 0.45 ثانية

// --- نغمات أركيد خلفية (Arcade Background Music) ---
const int bgMelody[] = {262, 330, 392, 523, 392, 330, 294, 349, 440, 587, 440, 349};
const int bgNoteDuration = 120; // سرعة عزف النغمة
unsigned long lastBgNoteTime = 0;
int currentBgNote = 0;

void setup() {
  Wire.begin(); // تفعيل بروتوكول الشاشة I2C
  
  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(btnPins[i], INPUT_PULLUP);
  }
  
  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  
  clearAllLeds();
  sendScoreToDisplay(0);
}

void loop() {
  // --- شاشة الانتظار ---
  if (!gameActive) {
    clearAllLeds(); 
    
    if (digitalRead(START_BTN) == LOW) {
      delay(50);
      if (digitalRead(START_BTN) == LOW) {
        while (digitalRead(START_BTN) == LOW); 
        playArcadeIntroMusic(); // موسيقى أركيد حماسية بالبداية
        gameActive = true;
        lives = 3;
        score = 0;
        sendScoreToDisplay(score);
      }
    }
    return;
  }

  // --- الفوز التام عند 150 نقطة ---
  if (score >= 150) {
    playVictorySequence();
    gameActive = false;
    return;
  }

  // ==========================================
  // 🟢 الراوند الأول: (من 0 إلى 45 نقطة)
  // ==========================================
  if (score < 50) {
    int target = random(0, 5);
    digitalWrite(ledPins[target], HIGH);

    unsigned long startTime = millis();
    bool hit = false;

    while (millis() - startTime < TIME_ROUND_1) {
      updateArcadeBgMusic(); // عزف موسيقى الخلفية أثناء الانتظار
      
      if (digitalRead(btnPins[target]) == LOW) {
        delay(20);
        if (digitalRead(btnPins[target]) == LOW) {
          hit = true;
          break;
        }
      }
    }

    digitalWrite(ledPins[target], LOW);

    if (hit) {
      score += 5;
      sendScoreToDisplay(score);
      playCorrectSound();
      
      if (score >= 50) {
        playRoundBreak(2);
      }
    } else {
      lives--;
      playWrongSound();
      checkGameOver();
    }
  }

  // ==========================================
  // 🟡 الراوند الثاني: (من 50 إلى 95 نقطة) - تسريع ومضاعفة!
  // ==========================================
  else if (score >= 50 && score < 100) {
    int target1 = random(0, 5);
    int target2 = random(0, 5);
    while (target2 == target1) target2 = random(0, 5);

    digitalWrite(ledPins[target1], HIGH);
    digitalWrite(ledPins[target2], HIGH);

    unsigned long startTime = millis();
    bool hit1 = false, hit2 = false;

    while (millis() - startTime < TIME_ROUND_2) {
      updateArcadeBgMusic(); // عزف موسيقى الخلفية
      
      if (digitalRead(btnPins[target1]) == LOW) hit1 = true;
      if (digitalRead(btnPins[target2]) == LOW) hit2 = true;
      if (hit1 && hit2) break;
    }

    digitalWrite(ledPins[target1], LOW);
    digitalWrite(ledPins[target2], LOW);

    if (hit1 && hit2) {
      score += 5;
      sendScoreToDisplay(score);
      playCorrectSound();
      
      if (score >= 100) {
        playRoundBreak(3);
      }
    } else {
      lives--;
      playWrongSound();
      checkGameOver();
    }
  }

  // ==========================================
  // 🔴 الراوند الثالث: (من 100 إلى 150 نقطة) - 3 لمبات وتحدي سريع جداً!
  // ==========================================
  else if (score >= 100) {
    int t1 = random(0, 5);
    int t2 = random(0, 5);
    while (t2 == t1) t2 = random(0, 5);
    int t3 = random(0, 5);
    while (t3 == t1 || t3 == t2) t3 = random(0, 5);

    digitalWrite(ledPins[t1], HIGH);
    digitalWrite(ledPins[t2], HIGH);
    digitalWrite(ledPins[t3], HIGH);

    unsigned long startTime = millis();
    bool hit1 = false, hit2 = false, hit3 = false;

    while (millis() - startTime < TIME_ROUND_3) {
      updateArcadeBgMusic(); // عزف موسيقى الخلفية
      
      if (digitalRead(btnPins[t1]) == LOW) hit1 = true;
      if (digitalRead(btnPins[t2]) == LOW) hit2 = true;
      if (digitalRead(btnPins[t3]) == LOW) hit3 = true;
      if (hit1 && hit2 && hit3) break;
    }

    digitalWrite(ledPins[t1], LOW);
    digitalWrite(ledPins[t2], LOW);
    digitalWrite(ledPins[t3], LOW);

    if (hit1 && hit2 && hit3) {
      score += 5;
      sendScoreToDisplay(score);
      playCorrectSound();
    } else {
      lives--;
      playWrongSound();
      checkGameOver();
    }
  }
}

// --- دالة تشغيل موسيقى الأركيد بالخلفية بدون تأخير (Non-blocking) ---
void updateArcadeBgMusic() {
  if (millis() - lastBgNoteTime >= bgNoteDuration) {
    lastBgNoteTime = millis();
    tone(BUZZER, bgMelody[currentBgNote], 40); // نغمة قصيرة جداً وممتعة
    currentBgNote = (currentBgNote + 1) % 12;   // تكرار الحلقة الموسيقية
  }
}

// --- مقدمة حماسية ---
void playArcadeIntroMusic() {
  int notes[] = {262, 330, 392, 523, 659, 784, 1046, 1318};
  for (int i = 0; i < 8; i++) {
    int rLed = random(0, 5);
    digitalWrite(ledPins[rLed], HIGH);
    tone(BUZZER, notes[i], 80);
    delay(90);
    digitalWrite(ledPins[rLed], LOW);
  }
  for(int f = 1000; f < 2000; f += 200) {
    tone(BUZZER, f, 30);
    delay(35);
  }
  noTone(BUZZER);
  clearAllLeds();
  delay(200);
}

// --- فاصل الجولات ---
void playRoundBreak(int nextRound) {
  clearAllLeds();
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, 800 + (i * 300), 70);
    for (int j = 0; j < 5; j++) digitalWrite(ledPins[j], HIGH);
    delay(80);
    clearAllLeds();
    delay(50);
  }
  for (int r = 0; r < 3; r++) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(30);
      digitalWrite(ledPins[i], LOW);
    }
  }
  tone(BUZZER, 1500, 250);
  delay(300);
  noTone(BUZZER);
}

// --- إرسال النتيجة للشاشة ---
void sendScoreToDisplay(int currentScore) {
  Wire.beginTransmission(8);
  Wire.write(currentScore);
  Wire.endTransmission();
}

void playCorrectSound() {
  tone(BUZZER, 1500, 50); delay(50);
  tone(BUZZER, 2000, 60); delay(60);
}

void playWrongSound() {
  tone(BUZZER, 180, 120); delay(120);
  tone(BUZZER, 130, 180); delay(180);
}

void playGameOverSound() {
  int failNotes[] = {400, 350, 300, 220, 150};
  for(int i=0; i<5; i++) {
    tone(BUZZER, failNotes[i], 150);
    delay(170);
  }
  noTone(BUZZER);
}

void playVictorySequence() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) digitalWrite(ledPins[j], HIGH);
    tone(BUZZER, 1046, 80); delay(100);
    clearAllLeds();
    tone(BUZZER, 1568, 80); delay(100);
  }
  tone(BUZZER, 2093, 500); delay(550);
  noTone(BUZZER);
}

void checkGameOver() {
  if (lives <= 0) {
    delay(100);
    playGameOverSound();
    gameActive = false;
    clearAllLeds();
  }
}

void clearAllLeds() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}
