#include <Arduino.h>
#include <HardwareSerial.h>
#include <OLED.h>
#include "EEPROM_Storage.h"

// Tạo 2 UART riêng
HardwareSerial Seria1(1);
HardwareSerial Seria2(2);

// Button pin definitions
#define INC_BUTTON 26
#define DEC_BUTTON 27
#define SLCT_BUTTON 25
#define SWITCH_CHANNEL_BUTTON 12

const int baud_list[] = {600,750,1200,2400,4800,9600,19200,31250,38400,57600,74880,115200,230400};
const int baud_count = sizeof(baud_list) / sizeof(baud_list[0]);

int baudIndex1 = 0;
int baudIndex2 = 0;
bool isBaud1 = true;
bool displayUART_A = true; // Display mode: true=UART_A, false=UART_B
String msg_A, msg_B;

volatile bool inInterruptMode = false;
volatile bool buttonPressed = false;
volatile int buttonType = 0; // 0=none, 1=inc, 2=dec, 3=select
volatile bool justEnteredMenu = false; // Flag to prevent immediate channel switch
volatile bool switchPressed = false; // Flag for display switch button

// Store original baud rates when entering interrupt mode
int originalBaudIndex1 = 0;
int originalBaudIndex2 = 0;
bool wasInInterruptMode = false;

hw_timer_t * timer = NULL;

// Minimal ISR handler - just set flags and record button type
void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;

  // Record which button was pressed
  if (digitalRead(INC_BUTTON) == HIGH) {
    buttonType = 1;
  }
  else if (digitalRead(DEC_BUTTON) == HIGH) {
    buttonType = 2;
  }
  else if (digitalRead(SLCT_BUTTON) == HIGH) {
    buttonType = 3;
  }

  // Only process if in interrupt mode or if SELECT button pressed
  if (inInterruptMode || buttonType == 3) {
    // Enter interrupt mode on SELECT button press
    if (!inInterruptMode && buttonType == 3) {
      inInterruptMode = true;
      isBaud1 = true; // Always default to UART_A
      justEnteredMenu = true; // Mark that we just entered menu
      // Store original values when entering interrupt mode
      originalBaudIndex1 = baudIndex1;
      originalBaudIndex2 = baudIndex2;
    }

    // Reset timer on valid button press
    timerWrite(timer, 0);
    timerAlarmEnable(timer);
  } else {
    // Ignore INC/DEC buttons when not in menu
    buttonPressed = false;
    buttonType = 0;
  }
}

// Process button actions in main loop (not in ISR)
void processButtonAction() {
  if (!buttonPressed || !inInterruptMode) return;

  buttonPressed = false;
  int* baudIndex = (isBaud1) ? &baudIndex1 : &baudIndex2;

  switch (buttonType) {
    case 1: // INC_BUTTON - only works in menu mode
      *baudIndex = (*baudIndex + 1) % baud_count;
      Serial.print("Tăng baud rate cho ");
      Serial.print(isBaud1 ? "UART_A: " : "UART_B: ");
      Serial.println(baud_list[*baudIndex]);
      break;

    case 2: // DEC_BUTTON - only works in menu mode
      *baudIndex = (*baudIndex - 1 + baud_count) % baud_count;
      Serial.print("Giảm baud rate cho ");
      Serial.print(isBaud1 ? "UART_A: " : "UART_B: ");
      Serial.println(baud_list[*baudIndex]);
      break;

    case 3: // SLCT_BUTTON - switch channel in menu mode
      if (justEnteredMenu) {
        // First SELECT press just entered menu, stay on UART_A
        justEnteredMenu = false;
        Serial.println("Mở menu cấu hình baud rate - UART_A");
      } else {
        // Subsequent SELECT presses switch channels
        isBaud1 = !isBaud1;
        Serial.print("Chuyển sang ");
        Serial.println(isBaud1 ? "UART_A" : "UART_B");
      }
      break;
  }

  buttonType = 0;
}

// ISR for display switch button
void IRAM_ATTR handleSwitchInterrupt() {
  // Only process if not in baud rate menu
  if (!inInterruptMode) {
    switchPressed = true;
  }
}

// Check and save baud rates after timeout
void checkAndSaveBaudRates() {
  // Use optimized single-commit save function
  saveBaudRatesOptimized(baudIndex1, baudIndex2);

  // Update UART configurations if they changed
  if (originalBaudIndex1 != baudIndex1) {
    Seria1.begin(baud_list[baudIndex1], SERIAL_8N1, 16, 17);
    Serial.print("Cập nhật baud rate cho UART_A: ");
    Serial.println(baud_list[baudIndex1]);
  }

  if (originalBaudIndex2 != baudIndex2) {
    Seria2.begin(baud_list[baudIndex2], SERIAL_8N1, 5, 4);
    Serial.print("Cập nhật baud rate cho UART_B: ");
    Serial.println(baud_list[baudIndex2]);
  }
}

// Timer interrupt handler - handles timeout after user stops selecting
void IRAM_ATTR onTimer() {
  inInterruptMode = false;
  justEnteredMenu = false; // Reset menu entry flag
  timerAlarmDisable(timer);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize EEPROM and load saved baud rates
  setupEEPROM();
  loadBaudRates(&baudIndex1, &baudIndex2);

  // Store initial values
  originalBaudIndex1 = baudIndex1;
  originalBaudIndex2 = baudIndex2;

  // Initialize UART with loaded baud rates
  Seria1.begin(baud_list[baudIndex1], SERIAL_8N1, 16, 17);
  Seria2.begin(baud_list[baudIndex2], SERIAL_8N1, 5, 4);

  Serial.println("ESP32 UART Bridge ready!");
  Serial.print("UART1 baud rate: ");
  Serial.println(baud_list[baudIndex1]);
  Serial.print("UART2 baud rate: ");
  Serial.println(baud_list[baudIndex2]);

  pinMode(SLCT_BUTTON, INPUT);
  pinMode(INC_BUTTON, INPUT);
  pinMode(DEC_BUTTON, INPUT);
  pinMode(SWITCH_CHANNEL_BUTTON, INPUT);

  attachInterrupt(digitalPinToInterrupt(SLCT_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(INC_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DEC_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(SWITCH_CHANNEL_BUTTON), handleSwitchInterrupt, RISING);

  // Timer setup: 3 second timeout
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 30000, false); // 3 seconds = 30000 ticks × 100µs
  setup_OLED();
}

void loop() {
  // Process button actions if any
  processButtonAction();

  // Process display switch button (only when not in baud rate menu)
  if (switchPressed && !inInterruptMode) {
    switchPressed = false;
    displayUART_A = !displayUART_A;
    Serial.print("Chuyển hiển thị sang ");
    Serial.println(displayUART_A ? "UART_A" : "UART_B");
  }

  // Check if we just exited interrupt mode (timer timeout occurred)
  if (wasInInterruptMode && !inInterruptMode) {
    checkAndSaveBaudRates();
    wasInInterruptMode = false;
  }

  // Update interrupt mode state
  wasInInterruptMode = inInterruptMode;

  if (!inInterruptMode) {
    // Show separate menus for UART_A and UART_B
    if (displayUART_A) {
      menu_UART_A(msg_A);
    } else {
      menu_UART_B(msg_B);
    }
  }

  if (Seria1.available()) {
    String received = Seria1.readStringUntil('\n');
    Serial.print("Từ UART1: ");
    Serial.println(received);
    Seria2.println(received);
    msg_A = received;
  }

  if (Seria2.available()) {
    String received = Seria2.readStringUntil('\n');
    Serial.print("Từ UART2: ");
    Serial.println(received);
    Seria1.println(received);
    msg_B = received;
  }

  if (inInterruptMode) {
    Serial.print("Đang chọn baudrate cho UART");
    if(isBaud1) {
      menu_UART(baud_list[baudIndex1], "UART_A");
      Serial.print("1: ");
      Serial.println(baud_list[baudIndex1]);
    } else {
      menu_UART(baud_list[baudIndex2], "UART_B");
      Serial.print("2: ");
      Serial.println(baud_list[baudIndex2]);
    }

    delay(500);
  }
}
