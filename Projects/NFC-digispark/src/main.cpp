#include <Wire.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <NDEF.h>
#include <NfcAdapter.h>

#define RED_PIN 3
#define BLUE_PIN 4
#define RELAY_PIN 5

#define SECRET "emgarageopen" 

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

void setup(void) {
  Serial.begin(115201);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);    // Turn on the RED LED
  digitalWrite(BLUE_PIN, HIGH);    // Turn on the RED LED
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Garage Door Reader!");
  nfc.begin();
}

void loop(void) {
  Serial.println("Scan an NFC tag");

  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();

    Serial.println(tag.getTagType());
    Serial.print("UID: ");Serial.println(tag.getUidString());

    if (tag.hasNdefMessage()) {
      NdefMessage message = tag.getNdefMessage();
      Serial.print("\nThis NFC Tag contains an NDEF Message with ");
      Serial.print(message.getRecordCount());
      Serial.print(" NDEF Record");
      if (message.getRecordCount() != 1) {
        Serial.print("s");
      }
      Serial.println(".");

      // cycle through the records, printing some info from each
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++) {
        Serial.print("\nNDEF Record ");Serial.println(i+1);
        NdefRecord record = message.getRecord(i);
        // NdefRecord record = message[i]; // alternate syntax

        Serial.print("  TNF: ");Serial.println(record.getTnf());
        Serial.print("  Type: ");Serial.println(record.getType()); // will be "" for TNF_EMPTY

        // The TNF and Type should be used to determine how your application processes the payload
        // There's no generic processing for the payload, it's returned as a byte[]
        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);

        // Print the Hex and Printable Characters
        Serial.print("  Payload (HEX): ");
        PrintHexChar(payload, payloadLength);

        // Force the data into a String (might work depending on the content)
        // Real code should use smarter processing
        String payloadAsString = "";
        for (int c = 0; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
        Serial.print("  Payload (as String): ");
        Serial.println(payloadAsString);
        if(payloadAsString.substring(1) == SECRET) {
          Serial.println("  opening garage!");
          digitalWrite(RED_PIN, LOW);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(RELAY_PIN, HIGH);
          delay(1500);
        } else { // Wrong card, reject
          Serial.print("  Bad password!"); Serial.println(payloadAsString);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(RED_PIN, HIGH);
          delay(1500);
        }
        digitalWrite(BLUE_PIN, HIGH);
        digitalWrite(RED_PIN, LOW);

        // id is probably blank and will return ""
        String uid = record.getId();
        if (uid != "") {
          Serial.print("  ID: ");Serial.println(uid);
        } else {
          Serial.print("  ID: NULL");
        }
      }
    }
  }
  delay(500);
}
