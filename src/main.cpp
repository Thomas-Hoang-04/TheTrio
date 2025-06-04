#include <Arduino.h>
#include <HardwareSerial.h>
#include <OLED.h>

// Tạo 2 UART riêng
HardwareSerial Seria1(1);
HardwareSerial Seria2(2);

// Button pin definitions
#define INC_BUTTON 27
#define DEC_BUTTON 26
#define SLCT_BUTTON 25

const int baud_list[] = {600,750,1200,2400,4800,9600,19200,31250,38400,57600,74880,115200,230400};
const int baud_count = sizeof(baud_list) / sizeof(baud_list[0]);


int baudIndex1 = 0;
int baudIndex2 = 0;
bool isBaud1 = true;
 char * msg_A, *msg_B;


volatile unsigned long lastInterruptTime = 0;
volatile bool inInterruptMode = false;

hw_timer_t * timer = NULL;

// Button interrupt handler
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long currentTime = millis();
  int* baudIndex = (isBaud1) ? &baudIndex1 : &baudIndex2 ;  

  if (!inInterruptMode) {
    inInterruptMode = true;
  }
  lastInterruptTime = currentTime;

  if (digitalRead(INC_BUTTON) == HIGH) {
    *baudIndex = (*baudIndex + 1) % baud_count;
  }
  else if (digitalRead(DEC_BUTTON) == HIGH) {
    *baudIndex = (*baudIndex - 1 + baud_count) % baud_count;
  }
  else if (digitalRead(SLCT_BUTTON) == HIGH) {
    Serial.print("Chọn baudrate kênh truyền ");
    Serial.print(isBaud1 ? "1: " : "2: ");
    Serial.println(baud_list[*baudIndex]);

    if (isBaud1) {
      baudIndex1 = *baudIndex;
    } else {
      baudIndex2 = *baudIndex;
    }

    isBaud1 = !isBaud1;
  }

  timerWrite(timer, 0);
}


// Timer interrupt handler
void IRAM_ATTR onTimer() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime >= 1000) {
    inInterruptMode = false;
    timerAlarmDisable(timer);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Seria1.begin(9600, SERIAL_8N1, 16, 17);
  Seria2.begin(9600, SERIAL_8N1, 5, 4);

  Serial.println("ESP32 UART Bridge ready!");

  pinMode(SLCT_BUTTON, INPUT);
  pinMode(INC_BUTTON, INPUT);
  pinMode(DEC_BUTTON, INPUT);

  attachInterrupt(digitalPinToInterrupt(SLCT_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(INC_BUTTON), handleButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DEC_BUTTON), handleButtonInterrupt, RISING);

  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);
}

void loop() {
  menu_msg(msg_A,msg_B);

  if (Seria1.available()) {
    String received = Seria1.readStringUntil('\n');
    Serial.print("Từ UART1: ");
    Serial.println(received);
    Seria2.println(received);
    msg_A 
    

  }

  if (Seria2.available()) {
    String received = Seria2.readStringUntil('\n');
    Serial.print("Từ UART2: ");
    Serial.println(received);
    Seria1.println(received);

  }

  if (inInterruptMode) {
    Serial.print("Đang chọn baudrate cho UART");
    if(isBaud1){
      Serial.print( "1: "); 
      Serial.println(baud_list[baudIndex1]);
    }else{
      Serial.print( "2: "); 
      Serial.println(baud_list[baudIndex2]);
    }



    delay(500);
  }
}
