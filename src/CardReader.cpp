#include "CardReader.h"
#include <ArduinoJson.h>

Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

void setupCardReader()
{
    Wire.begin(PN532_SDA, PN532_SCL);
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    {
        Serial.println("Didn't find PN532 board");
        while (1)
            ;
    }
    nfc.SAMConfig();
    Serial.println("PN532 initialized");
}

void readCard()
{
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success)
    {
        Serial.print("Card UID: ");
        for (uint8_t i = 0; i < uidLength; i++)
        {
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else
    {
        Serial.println("No card detected");
    }
}

void writeCard(const String &data)
{
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;
    unsigned long startTime = millis();
    const unsigned long TIMEOUT = 5000; // 5 second timeout

    Serial.println("Place card on reader to write data...");

    // Wait for card with timeout
    while (millis() - startTime < TIMEOUT)
    {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500);
        if (success)
            break;
    }

    if (!success || (millis() - startTime >= TIMEOUT))
    {
        Serial.println("No card detected or timeout occurred");
        return;
    }

    Serial.print("Card detected. UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Write data to block 4 (first user data block after system blocks)
    // MIFARE Classic blocks are 16 bytes each
    uint8_t blockNumber = 4;
    uint8_t dataBlock[16] = {0};

    // Copy the string data to the block (max 16 bytes)
    uint8_t dataLen = min((uint8_t)data.length(), (uint8_t)16);
    memcpy(dataBlock, data.c_str(), dataLen);

    // Write the block
    if (nfc.mifareclassic_WriteDataBlock(blockNumber, dataBlock))
    {
        Serial.println("Data written successfully!");
        Serial.print("Data written: ");
        for (int i = 0; i < 16; i++)
        {
            Serial.print((char)dataBlock[i]);
        }
        Serial.println();
    }
    else
    {
        Serial.println("Failed to write data to card");
    }
}

void readCardData()
{
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;
    unsigned long startTime = millis();
    const unsigned long TIMEOUT = 5000; // 5 second timeout

    Serial.println("Place card on reader to read data...");

    // Wait for card with timeout
    while (millis() - startTime < TIMEOUT)
    {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500);
        if (success)
            break;
    }

    if (!success || (millis() - startTime >= TIMEOUT))
    {
        Serial.println("No card detected or timeout occurred");
        return;
    }

    Serial.print("Card detected. UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    delay(100); // Give card time to settle

    // Read data blocks (blocks 4-63 for MIFARE Classic 1K)
    // Each block is 16 bytes
    Serial.println("Card data:");
    uint8_t dataBlock[16];

    for (uint8_t blockNum = 4; blockNum < 64; blockNum++)
    {
        // Skip trailer blocks (3, 7, 11, 15, etc.)
        if ((blockNum + 1) % 4 == 0)
            continue;

        // Authenticate before reading each block
        uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNum, 0, keya))
        {
            // Authentication failed, try next block
            continue;
        }

        delay(10); // Small delay between operations

        if (nfc.mifareclassic_ReadDataBlock(blockNum, dataBlock))
        {
            Serial.print("Block ");
            Serial.print(blockNum);
            Serial.print(": ");
            for (int i = 0; i < 16; i++)
            {
                if (dataBlock[i] >= 32 && dataBlock[i] <= 126)
                    Serial.print((char)dataBlock[i]);
                else
                    Serial.print(".");
            }
            Serial.print(" | HEX: ");
            for (int i = 0; i < 16; i++)
            {
                if (dataBlock[i] < 16)
                    Serial.print("0");
                Serial.print(dataBlock[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
        else
        {
            Serial.print("Block ");
            Serial.print(blockNum);
            Serial.println(": Failed to read");
        }
    }
}

// Write data across multiple blocks
void writeDataMultipleBlocks(const String &data)
{
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;
    unsigned long startTime = millis();
    const unsigned long TIMEOUT = 5000; // 5 second timeout

    Serial.println("Place card on reader to write data...");

    // Wait for card with timeout
    while (millis() - startTime < TIMEOUT)
    {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500);
        if (success)
            break;
    }

    if (!success || (millis() - startTime >= TIMEOUT))
    {
        Serial.println("No card detected or timeout occurred");
        return;
    }

    Serial.print("Card detected. UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    delay(100); // Give card time to settle

    uint16_t dataLen = data.length();
    uint8_t blockNum = 4;
    uint16_t offset = 0;
    bool allSuccess = true;

    // Write data across multiple blocks
    while (offset < dataLen && blockNum < 64)
    {
        // Skip trailer blocks
        if ((blockNum + 1) % 4 == 0)
        {
            blockNum++;
            continue;
        }

        // Authenticate before writing each block
        uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNum, 0, keya))
        {
            Serial.print("Failed to authenticate block ");
            Serial.println(blockNum);
            allSuccess = false;
            blockNum++;
            continue;
        }

        delay(10); // Small delay between operations

        uint8_t dataBlock[16] = {0};
        uint8_t bytesToWrite = min((uint16_t)16, (uint16_t)(dataLen - offset));

        // Copy data to block
        for (int i = 0; i < bytesToWrite; i++)
        {
            dataBlock[i] = data[offset + i];
        }

        // Write block
        if (nfc.mifareclassic_WriteDataBlock(blockNum, dataBlock))
        {
            Serial.print("Block ");
            Serial.print(blockNum);
            Serial.println(" written successfully");
        }
        else
        {
            Serial.print("Failed to write block ");
            Serial.println(blockNum);
            allSuccess = false;
        }

        offset += bytesToWrite;
        blockNum++;
    }

    if (allSuccess)
    {
        Serial.println("Data written successfully!");
    }
    else
    {
        Serial.println("Some blocks failed to write");
    }
}

// Read all data from multiple blocks
String readDataMultipleBlocks()
{
    uint8_t success;
    uint8_t uid[7];
    uint8_t uidLength;
    String allData = "";
    unsigned long startTime = millis();
    const unsigned long TIMEOUT = 5000; // 5 second timeout

    Serial.println("Place card on reader to read data...");

    // Wait for card with timeout
    while (millis() - startTime < TIMEOUT)
    {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500);
        if (success)
            break;
        delay(100);
    }

    if (!success || (millis() - startTime >= TIMEOUT))
    {
        Serial.println("No card detected or timeout occurred");
        return "";
    }

    Serial.print("Card detected. UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    delay(100); // Give card time to settle

    uint8_t dataBlock[16];
    bool hasData = false;

    for (uint8_t blockNum = 4; blockNum < 64; blockNum++)
    {
        // Skip trailer blocks
        if ((blockNum + 1) % 4 == 0)
            continue;

        // Authenticate before reading each block
        uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (!nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNum, 0, keya))
        {
            // Authentication failed, try next block
            continue;
        }

        delay(10); // Small delay between operations

        if (nfc.mifareclassic_ReadDataBlock(blockNum, dataBlock))
        {
            bool blockHasData = false;
            for (int i = 0; i < 16; i++)
            {
                if (dataBlock[i] != 0)
                {
                    allData += (char)dataBlock[i];
                    blockHasData = true;
                    hasData = true;
                }
            }
            if (blockHasData)
            {
                Serial.print("Block ");
                Serial.print(blockNum);
                Serial.println(" read successfully");
            }
        }
        else
        {
            Serial.print("Failed to read block ");
            Serial.println(blockNum);
        }
    }

    Serial.print("Read data: ");
    Serial.println(allData);
    return allData;
}

void handleCardSerialInput()
{
    if (Serial.available() > 0)
    {
        // Read data from serial until newline
        String inputData = Serial.readStringUntil('\n');
        inputData.trim(); // Remove whitespace

        if (inputData.length() > 0)
        {
            // Check for commands
            if (inputData.equalsIgnoreCase("r") || inputData.equalsIgnoreCase("read"))
            {
                readCardData();
            }
            else if (inputData.equalsIgnoreCase("readall"))
            {
                readDataMultipleBlocks();
            }
            else if (inputData.startsWith("w:"))
            {
                // Write command: w:data
                String dataToWrite = inputData.substring(2);
                Serial.print("Writing to card: ");
                Serial.println(dataToWrite);
                writeDataMultipleBlocks(dataToWrite);
            }
            else if (inputData.startsWith("j:"))
            {
                // JSON write command: j:{"key":"value"}
                String jsonData = inputData.substring(2);
                Serial.print("Writing JSON to card: ");
                Serial.println(jsonData);
                writeDataMultipleBlocks(jsonData);
            }
            else
            {
                // Default behavior: write the input as data
                Serial.print("Writing to card: ");
                Serial.println(inputData);
                writeCard(inputData);
            }
        }
    }
}

String userDataToJson(const User &user)
{
    StaticJsonDocument<256> doc;
    doc["email"] = user.email;
    doc["password"] = user.password;
    // Add more fields as needed

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

User jsonToUserData(const String &json)
{
    StaticJsonDocument<256> doc;
    deserializeJson(doc, json);

    User user;
    user.email = doc["email"].as<String>();
    user.password = doc["password"].as<String>();
    // Initialize more fields as needed

    return user;
}
