#include <Arduino.h>
#include <HardwareSerial.h>
#include <display/OLED.h>
#include <config/BaudConfig.h>
#include <storage/EEPROM_Storage.h>

// Tạo 2 UART riêng
HardwareSerial Seria1(1);
HardwareSerial Seria2(2);

// UART pins
#define UART1_RX_PIN 16
#define UART1_TX_PIN 17
#define UART2_RX_PIN 5
#define UART2_TX_PIN 4

// Button pins
#define INC_BUTTON 26
#define DEC_BUTTON 27
#define SLCT_BUTTON 25

int baudIndex1 = 0;
int baudIndex2 = 0;
bool isBaud1 = true;
int baudrateA, baudrateB;

String msg_A = "";
String msg_B = "";

volatile bool inInterruptMode = false;
volatile bool buttonPressed = false;
volatile int buttonType = 0; // 0=none, 1=inc, 2=dec, 3=select

// Store original baud rates when entering interrupt mode
int originalBaudIndex1 = 0;
int originalBaudIndex2 = 0;
bool wasInInterruptMode = false;

unsigned long lastDebounceTime = 0;

hw_timer_t* timer = NULL;

// Minimal ISR handler - just set flags and record button type
void IRAM_ATTR handleButtonInterrupt() {
  if (millis() - lastDebounceTime < 50) return;
  lastDebounceTime = millis();

  if (!inInterruptMode) {
    inInterruptMode = true;
    // Store original values when entering interrupt mode
    originalBaudIndex1 = baudIndex1;
    originalBaudIndex2 = baudIndex2;
  }
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

  // Reset timer on any button press
  timerWrite(timer, 0);
  timerAlarmEnable(timer);
}

// Process button actions in main loop (not in ISR)
void processButtonAction() {
  if (!buttonPressed) return;

  buttonPressed = false;
  int* baudIndex = (isBaud1) ? &baudIndex1 : &baudIndex2;

  switch (buttonType) {
    case 1: // INC_BUTTON
      *baudIndex = (*baudIndex + 1) % baud_count;
      break;

    case 2: // DEC_BUTTON
      *baudIndex = (*baudIndex - 1 + baud_count) % baud_count;
      break;

    case 3: // SLCT_BUTTON
      Serial.print("Chọn baudrate kênh truyền ");
      Serial.print(isBaud1 ? "1: " : "2: ");
      Serial.println(baud_list[*baudIndex]);
      isBaud1 = !isBaud1;
      break;
  }

  buttonType = 0;
}

// Check and save baud rates after timeout
void checkAndSaveBaudRates() {
  // Use optimized single-commit save function
  saveBaudRates(baudIndex1, baudIndex2);

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

  baudrateA = baud_list[baudIndex1];
  baudrateB = baud_list[baudIndex2];
}

void handleUARTCommunication() {
  String received;
  if (Seria1.available()) {
    received = Seria1.readStringUntil('\n');
    Serial.print("Từ UART1: ");
    Serial.println(received);
    Seria2.println(received);
    msg_A = received;
  }

  if (Seria2.available()) {
    received = Seria2.readStringUntil('\n');
    Serial.print("Từ UART2: ");
    Serial.println(received);
    Seria1.println(received);
    msg_B = received;
  }
}

// Timer interrupt handler - handles timeout after user stops selecting
void IRAM_ATTR onTimer() {
  inInterruptMode = false;
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
  Seria1.begin(baud_list[baudIndex1], SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
  Seria2.begin(baud_list[baudIndex2], SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);

  baudrateA = baud_list[baudIndex1];
  baudrateB = baud_list[baudIndex2];

  Serial.println("ESP32 UART Bridge ready!");
  Serial.print("UART1 baud rate: ");
  Serial.println(baud_list[baudIndex1]);
  Serial.print("UART2 baud rate: ");
  Serial.println(baud_list[baudIndex2]);

  pinMode(SLCT_BUTTON, INPUT);
  pinMode(INC_BUTTON, INPUT);
  pinMode(DEC_BUTTON, INPUT);

  attachInterrupt(digitalPinToInterrupt(SLCT_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(INC_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DEC_BUTTON), handleButtonInterrupt, RISING);

  // Timer setup: 3 second timeout
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 30000, false); // 3 seconds = 30000 ticks × 100µs
  setup_OLED();
}

void loop() {
  // Process button actions if any
  processButtonAction();

  // Check if we just exited interrupt mode (timer timeout occurred)
  if (wasInInterruptMode && !inInterruptMode) {
    checkAndSaveBaudRates();
    wasInInterruptMode = false;
  }

  // Update interrupt mode state
  wasInInterruptMode = inInterruptMode;

  handleUARTCommunication();

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
  } else {
      menu_msg(msg_A, msg_B, baudrateA, baudrateB);
  }
}
