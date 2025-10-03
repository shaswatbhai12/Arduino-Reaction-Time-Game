// Reaction Time Game for Arduino (Tinkercad-ready)
// LED on pin 8, buzzer on pin 9, button on pin 2 (uses INPUT_PULLUP)

const int ledPin = 8;
const int buzzerPin = 9;
const int buttonPin = 2;

unsigned long goTime = 0;
unsigned long startWait = 0;
bool waitingForGo = false;
bool roundActive = false;
bool pressedBeforeGo = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // button to GND
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
  randomSeed(analogRead(A0)); // seed RNG
  Serial.println("Reaction Time Game ready!");
  Serial.println("Press the button to start a round.");
}

void loop() {
  // Read button (pressed when LOW because of INPUT_PULLUP)
  bool buttonPressed = (digitalRead(buttonPin) == LOW);

  if (!roundActive && buttonPressed) {
    // start a new round
    startNewRound();
    // simple debounce wait
    delay(150);
  }

  if (waitingForGo) {
    // if player presses before GO, mark false start
    if (buttonPressed) {
      pressedBeforeGo = true;
    }
    // check if random wait elapsed
    if (millis() - startWait >= (unsigned long) (goTime)) {
      // GO signal
      digitalWrite(ledPin, HIGH);
      unsigned long goSignalAt = millis();
      waitingForGo = false;
      roundActive = true;
      Serial.println("GO! Press the button now!");
      // now wait for button press to measure reaction time
      unsigned long reactionStart = millis();
      // wait for press (with simple debounce)
      while (true) {
        bool bp = (digitalRead(buttonPin) == LOW);
        if (bp) {
          unsigned long reactionEnd = millis();
          unsigned long reactionTime = reactionEnd - reactionStart;
          if (pressedBeforeGo) {
            Serial.println("False start! You pressed before GO.");
            playToneSequence(false);
          } else {
            Serial.print("Reaction time: ");
            Serial.print(reactionTime);
            Serial.println(" ms");
            playToneForTime(reactionTime);
          }
          delay(600); // short pause before reset
          resetRound();
          break;
        }
        // tiny sleep to avoid busy loop
        delay(5);
      }
    }
  }
}

// start a new round: choose random wait, show "get ready"
void startNewRound() {
  pressedBeforeGo = false;
  roundActive = false;
  waitingForGo = true;
  // random delay between 1000 and 4000 ms
  goTime = random(1000, 4000);
  startWait = millis();
  Serial.print("Get ready... waiting ");
  Serial.print(goTime);
  Serial.println(" ms before GO");
  // small blinking animation while waiting
  unsigned long blinkUntil = startWait + goTime;
  while (millis() < blinkUntil) {
    digitalWrite(ledPin, HIGH);
    delay(150);
    digitalWrite(ledPin, LOW);
    delay(150);
    // allow early button press detection during blink
    if (digitalRead(buttonPin) == LOW) {
      pressedBeforeGo = true;
    }
  }
  // note: actual GO handling is in loop() to keep structure consistent
}

// reset round state
void resetRound() {
  digitalWrite(ledPin, LOW);
  roundActive = false;
  waitingForGo = false;
  pressedBeforeGo = false;
  Serial.println("Round over. Press button to play again.");
}

// play buzzer tone based on reaction time (faster => higher pitch)
void playToneForTime(unsigned long reactionMs) {
  // clamp reaction time
  if (reactionMs < 50) reactionMs = 50;
  if (reactionMs > 2000) reactionMs = 2000;
  // map 50..2000 ms -> 1200..300 Hz (faster -> higher freq)
  int freq = map(reactionMs, 50, 2000, 1200, 300);
  int dur = 150;
  tone(buzzerPin, freq, dur);
  delay(dur + 20);
  noTone(buzzerPin);
}

// short beep sequence for false start or celebration
void playToneSequence(bool good) {
  if (!good) {
    // sad buzzer: low beep
    tone(buzzerPin, 300, 200);
    delay(250);
    noTone(buzzerPin);
  } else {
    // celebratory 3-beep sequence
    tone(buzzerPin, 800, 120);
    delay(140);
    tone(buzzerPin, 1000, 100);
    delay(120);
    tone(buzzerPin, 1200, 90);
    delay(110);
    noTone(buzzerPin);
  }
}
