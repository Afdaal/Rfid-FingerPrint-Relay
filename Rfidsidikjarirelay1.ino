#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

SoftwareSerial mySerial(2, 3);  // RX, TX for fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define RST_PIN         9          // Reset pin for RFID module
#define SS_PIN          10         // Slave Select pin for RFID module
#define RELAY_PIN       8          // Pin to control relay

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Initialize MFRC522

void setup() {
  Serial.begin(9600);             // Initialize serial communication
  SPI.begin();                    // Initialize SPI bus
  mfrc522.PCD_Init();              // Initialize RFID module
  mySerial.begin(57600);           // Initialize fingerprint sensor
  
  if (!finger.verifyPassword()) {
    Serial.println("Gagal terhubung dengan sensor sidik jari");
    while (1) {
      delay(1);
    }
  }

  pinMode(RELAY_PIN, OUTPUT);      // Set relay pin as output
  digitalWrite(RELAY_PIN, HIGH);    // Turn off relay by default
}

void loop() {
  // Scan RFID card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("Kartu RFID ID:  ");

    // Display RFID card ID
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Check RFID card ID to activate relay
    if (checkRFID(mfrc522.uid.uidByte, mfrc522.uid.size)) {
      digitalWrite(RELAY_PIN, LOW);  // Activate relay
      Serial.println("Relay Aktif karena Kartu RFID Benar");
      delay(5000);  // Hold relay for 5 seconds
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay Nonaktif");
    } else {
      Serial.println("ID kartu RFID tidak valid!");
    }

    mfrc522.PICC_HaltA();  // Halt card
  }

  // Scan fingerprint
  int fingerprintID = getFingerPrint();

  if (fingerprintID != -1) {
    Serial.print("Sidik jari terdeteksi! ID: ");
    Serial.println(fingerprintID);
    
    if (fingerprintID == 1) {
      digitalWrite(RELAY_PIN, LOW);  // Activate relay
      Serial.println("Relay Aktif karena Sidik Jari Benar");
      delay(5000);  // Hold relay for 5 seconds
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay Nonaktif");
    } else {
      Serial.println("ID sidik jari tidak valid!");
    }
  }
  delay(50);  // Add some delay before next scan
}

// Function to check if RFID card ID is valid
bool checkRFID(byte *uid, byte uidLength) {
  // Replace with allowed RFID card ID
  byte allowedUID[] = {0xC3, 0x41, 0xE2, 0x12};

  // Compare RFID card ID
  for (byte i = 0; i < uidLength; i++) {
    if (uid[i] != allowedUID[i]) {
      return false;  // Invalid RFID card ID
    }
  }
  return true;  // Valid RFID card ID
}

// Function to get fingerprint ID
int getFingerPrint() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // Fingerprint detected
  return finger.fingerID;
}
