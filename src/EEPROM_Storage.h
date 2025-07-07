#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>

// EEPROM addresses for storing baud rate indices
#define EEPROM_BAUD1_ADDR 0
#define EEPROM_BAUD2_ADDR 4
#define EEPROM_INIT_FLAG_ADDR 8
#define EEPROM_INIT_VALUE 0xAA

// Function declarations
void setupEEPROM();
void saveBaudRates(int baudIndex1, int baudIndex2);
void loadBaudRates(int* baudIndex1, int* baudIndex2);
bool isEEPROMInitialized();
void initializeEEPROM();

#endif