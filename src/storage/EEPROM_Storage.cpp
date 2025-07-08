#include "EEPROM_Storage.h"
#include "config/BaudConfig.h"  // Chứa baud_list[] và baud_count

void setupEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    
    if (!isEEPROMInitialized()) {
        if (initializeEEPROM()) {
            Serial.println("EEPROM initialized with default values");
        } else {
            Serial.println("EEPROM initialization failed");
        }
    } else {
        Serial.println("EEPROM already initialized");
    }
}

bool isEEPROMInitialized() {
    uint8_t flag = EEPROM.read(EEPROM_INIT_FLAG_ADDR);
    uint8_t version = EEPROM.read(EEPROM_VERSION_ADDR);
    return (flag == EEPROM_INIT_VALUE && version == EEPROM_VERSION);
}

bool initializeEEPROM() {
    int defaultIndex = 5; // default baud rate indices (index 5 = 9600 baud)

    EEPROM.put(EEPROM_BAUD1_ADDR, defaultIndex);
    EEPROM.put(EEPROM_BAUD2_ADDR, defaultIndex);
    EEPROM.write(EEPROM_INIT_FLAG_ADDR, EEPROM_INIT_VALUE);
    EEPROM.write(EEPROM_VERSION_ADDR, EEPROM_VERSION);

    if (EEPROM.commit()) {
        Serial.println("EEPROM initialized with default baud rates (9600)");
        return true;
    } else {
        Serial.println("EEPROM commit failed");
        return false;
    }
}

void saveBaudRates(int baudIndex1, int baudIndex2) {
    int currentIndex1, currentIndex2;
    EEPROM.get(EEPROM_BAUD1_ADDR, currentIndex1);
    EEPROM.get(EEPROM_BAUD2_ADDR, currentIndex2);

    bool changed1 = (currentIndex1 != baudIndex1);
    bool changed2 = (currentIndex2 != baudIndex2);

    if (changed1 || changed2) {
        if (changed1) EEPROM.put(EEPROM_BAUD1_ADDR, baudIndex1);
        if (changed2) EEPROM.put(EEPROM_BAUD2_ADDR, baudIndex2);

        if (EEPROM.commit()) {
            if (changed1 && validateBaudRateIndex(baudIndex1)) {
                Serial.print("UART_A baud rate updated to: ");
                Serial.println(baud_list[baudIndex1]);
            }

            if (changed2 && validateBaudRateIndex(baudIndex2)) {
                Serial.print("UART_B baud rate updated to: ");
                Serial.println(baud_list[baudIndex2]);
            }

            Serial.println("Baud rate configuration saved to EEPROM");
        } else {
            Serial.println("EEPROM commit failed during baud rate save");
        }
    }
}

void loadBaudRates(int* baudIndex1, int* baudIndex2) {
    if (!baudIndex1 || !baudIndex2) {
        Serial.println("Invalid pointer parameters to loadBaudRates");
        return;
    }

    EEPROM.get(EEPROM_BAUD1_ADDR, *baudIndex1);
    EEPROM.get(EEPROM_BAUD2_ADDR, *baudIndex2);

    bool corrected = false;

    if (!validateBaudRateIndex(*baudIndex1)) {
        *baudIndex1 = 5;
        Serial.println("Invalid UART1 baud index, using default (9600)");
        corrected = true;
    }

    if (!validateBaudRateIndex(*baudIndex2)) {
        *baudIndex2 = 5;
        Serial.println("Invalid UART2 baud index, using default (9600)");
        corrected = true;
    }

    Serial.print("Baud rates loaded from EEPROM - UART1 index: ");
    Serial.print(*baudIndex1);
    Serial.print(" (");
    Serial.print(baud_list[*baudIndex1]);
    Serial.print("), UART2 index: ");
    Serial.print(*baudIndex2);
    Serial.print(" (");
    Serial.print(baud_list[*baudIndex2]);
    Serial.println(")");

    if (corrected) {
        saveBaudRates(*baudIndex1, *baudIndex2); 
    }
}

bool validateBaudRateIndex(int index) {
    return index >= 0 && index < baud_count;
}
