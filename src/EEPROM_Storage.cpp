#include "EEPROM_Storage.h"

void setupEEPROM() {
    EEPROM.begin(512); // Initialize EEPROM with 512 bytes
    
    // Check if EEPROM has been initialized before
    if (!isEEPROMInitialized()) {
        initializeEEPROM();
        Serial.println("EEPROM initialized with default values");
    } else {
        Serial.println("EEPROM already initialized");
    }
}

bool isEEPROMInitialized() {
    uint8_t flag = EEPROM.read(EEPROM_INIT_FLAG_ADDR);
    return (flag == EEPROM_INIT_VALUE);
}

void initializeEEPROM() {
    // Set default baud rate indices (index 5 = 9600 baud)
    EEPROM.writeInt(EEPROM_BAUD1_ADDR, 5);  // Default to 9600 for UART1
    EEPROM.writeInt(EEPROM_BAUD2_ADDR, 5);  // Default to 9600 for UART2
    EEPROM.write(EEPROM_INIT_FLAG_ADDR, EEPROM_INIT_VALUE);
    EEPROM.commit();
    Serial.println("EEPROM initialized with default baud rates (9600)");
}

void saveBaudRates(int baudIndex1, int baudIndex2) {
    EEPROM.writeInt(EEPROM_BAUD1_ADDR, baudIndex1);
    EEPROM.writeInt(EEPROM_BAUD2_ADDR, baudIndex2);
    EEPROM.commit();
}

void loadBaudRates(int* baudIndex1, int* baudIndex2) {
    *baudIndex1 = EEPROM.readInt(EEPROM_BAUD1_ADDR);
    *baudIndex2 = EEPROM.readInt(EEPROM_BAUD2_ADDR);
    
    // Validate loaded values (ensure they're within valid range)
    const int baud_count = 13; // Based on baud_list size in main.cpp
    
    if (*baudIndex1 < 0 || *baudIndex1 >= baud_count) {
        *baudIndex1 = 5; // Default to 9600 (index 5)
    }
    
    if (*baudIndex2 < 0 || *baudIndex2 >= baud_count) {
        *baudIndex2 = 5; // Default to 9600 (index 5)
    }
    
    Serial.print("Baud rates loaded from EEPROM - UART1 index: ");
    Serial.print(*baudIndex1);
    Serial.print(", UART2 index: ");
    Serial.println(*baudIndex2);
} 