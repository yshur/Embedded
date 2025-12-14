#include <Arduino.h>

/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         25         // Configurable, see typical pin layout above
#define SS_PIN          27         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

enum Mode { IDLE, LOAD, GATE };
Mode mode = IDLE;
int pendingLoad = 0;

const byte BUDGET_BLOCK = 4;
MFRC522::MIFARE_Key key;

bool selectAndAuth(byte block) {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  // Default key A = FF FF FF FF FF FF
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    block,
    &key,
    &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return false;
  }
  return true;
}

bool writeBudget(int budget) {
  if (!selectAndAuth(BUDGET_BLOCK)) return false;

  byte buffer[16];
  memset(buffer, 0, sizeof(buffer));

  // Save "B=25" as text
  String s = "B=" + String(budget);
  s.toCharArray((char*)buffer, 16);

  MFRC522::StatusCode status = mfrc522.MIFARE_Write(BUDGET_BLOCK, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return false;
  }

  Serial.print("Wrote budget = ");
  Serial.println(budget);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return true;
}

int readBudget(bool &ok) {
  ok = false;
  if (!selectAndAuth(BUDGET_BLOCK)) return 0;

  byte buffer[18];
  byte size = sizeof(buffer);

  MFRC522::StatusCode status = mfrc522.MIFARE_Read(BUDGET_BLOCK, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return 0;
  }

  // buffer should be text - can end with zeros at the end
  buffer[16] = 0; // validate the end of the string
  String raw = String((char*)buffer);
  raw.trim();

  if (!raw.startsWith("B=")) { ok = false; return 0; }
  int budget = raw.substring(2).toInt();

  Serial.print("Read raw = '");
  Serial.print((char*)buffer);
  Serial.println("'");

  Serial.print("Parsed budget = ");
  Serial.println(budget);

  ok = true;
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return budget;
}

void setup() {
//   Serial.begin(115200);// Initialize serial communications with the PC
//   delay(1000);

	Serial.begin(115200);		
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	// Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  Serial.println(F("Commands: 'L 10' or 'G'"));
}

void loop() {
  // Get command from Serial:
  //   L <num>  => LOAD mode, write num to card
  //   G        => GATE mode, read and decrement if possible
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("L ")) {
      pendingLoad = cmd.substring(2).toInt();
      mode = LOAD;
      Serial.printf("LOAD mode: put card to write %d\n", pendingLoad);
    } else if (cmd == "G") {
      mode = GATE;
      Serial.println("GATE mode: put card to decrement if possible");
    } else {
      Serial.println("Commands: 'L 10' or 'G'");
    }
  }

  if (mode == IDLE) {
    // delay(200);
    return;
  }

  // LOAD: write only when asking
  if (mode == LOAD) {
    while (!writeBudget(pendingLoad)) {
      delay(200);
    }
    Serial.println("Loaded OK");
    Serial.println(F("Commands: 'L 10' or 'G'"));
    mode = IDLE;
    return;
  }

  // GATE: 1 Read, and if there's budget, decrement by 1
  if (mode == GATE) {
    bool ok;
    int b = readBudget(ok);

    if (ok) {
      if (!b) {
        Serial.println("Denied: budget=0");
        Serial.println(F("Commands: 'L 10' or 'G'"));
        mode = IDLE;
        return;
      }
      while (!writeBudget(b - 1)) {
        delay(200);
      }
      Serial.printf("Entry OK. New budget=%d\n", b - 1);
    }
  }
}
