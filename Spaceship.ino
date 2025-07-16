#include <LiquidCrystal.h>
#include <stdlib.h>  // For random number generation

// Game structures
struct Player {
  int pos;  // 0 (top) or 1 (bottom)
  bool hit;
};

struct Bullet {
  int pos;
  bool active;
  int row;  // 0 (top) or 1 (bottom)
};

// Hardware setup
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int buttonPin = 6;

// Game state
Player player;
Bullet bullets[3];  // Array for multiple bullets
int gameScore = 0;
bool gameOver = false;

// Timing control
unsigned long lastGameUpdate = 0;
const unsigned long gameUpdateInterval = 150; // ms
unsigned long lastBulletSpawn = 0;
const unsigned long bulletSpawnInterval = 1000; // ms

// Button control
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 25;
int lastButtonState = HIGH;
int buttonState = HIGH;

void setup() {
  lcd.begin(16, 2);
  pinMode(buttonPin, INPUT_PULLUP);
  randomSeed(analogRead(0));  // Seed random number generator
  initializeGame();
}

void initializeGame() {
  player.pos = 1;  // Start at bottom
  player.hit = false;
  
  // Initialize all bullets as inactive
  for (int i = 0; i < 3; i++) {
    bullets[i].active = false;
  }
  
  gameScore = 0;
  gameOver = false;
  
  lcd.clear();
}

void loop() {
  checkButton();  // Check button state every loop
  
  if (!gameOver) {
    unsigned long currentTime = millis();
    
    // Update game at consistent intervals
    if (currentTime - lastGameUpdate >= gameUpdateInterval) {
      lastGameUpdate = currentTime;
      updateGame();
      spawnBullets(currentTime);
      renderGame();
    }
  } else {
    displayGameOver();
    // Auto-restart after 2 seconds
    if (millis() - lastGameUpdate > 5000) {
      initializeGame();
    }
  }
}

void spawnBullets(unsigned long currentTime) {
  if (currentTime - lastBulletSpawn >= bulletSpawnInterval) {
    lastBulletSpawn = currentTime;
    
    // Find an inactive bullet to spawn
    for (int i = 0; i < 3; i++) {
      if (!bullets[i].active) {
        bullets[i].pos = 15;  // Start at right edge
        bullets[i].row = random(2);  // Random row (0 or 1)
        bullets[i].active = true;
        break;
      }
    }
  }
}

void checkButton() {
  int reading = digitalRead(buttonPin);
  
  // Reset debounce timer if state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only accept the change if it's stable
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // On button press (LOW with pullup)
      if (buttonState == LOW) {
        player.pos = 1 - player.pos; // Toggle between 0 and 1
      }
    }
  }
  
  lastButtonState = reading;
}

void updateGame() {
  // Update all active bullets
  for (int i = 0; i < 3; i++) {
    if (bullets[i].active) {
      bullets[i].pos--;
      
      // Check if bullet went off screen
      if (bullets[i].pos < 0) {
        bullets[i].active = false;
        gameScore++;
      }
      
      // Check collision with player
      if (bullets[i].pos == 0 && bullets[i].row == player.pos) {
        gameOver = true;
      }
    }
  }
}

void renderGame() {
  lcd.clear();
  
  // Draw player ship (always at left column)
  lcd.setCursor(0, player.pos);
  lcd.print(">");
  
  // Draw all active bullets
  for (int i = 0; i < 3; i++) {
    if (bullets[i].active) {
      lcd.setCursor(bullets[i].pos, bullets[i].row);
      lcd.print(".");
    }
  }
  
  // Display score
  lcd.setCursor(12, 1);
  lcd.print(gameScore);
}

void displayGameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(gameScore);
}