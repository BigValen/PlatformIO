#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define RED_LED 5
#define BLUE_LED 6
#define RELAY_ON_PIN 7
#define RELAY_OFF_PIN 8

#define "SECRET" SECRET

PN532_SPI pn532spi(SPI, 10); // pin 10 is for the NIC SPI
NfcAdapter nfc = NfcAdapter(pn532spi);

void setup(void) {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RELAY_ON_PIN, OUTPUT);
  pinMode(RELAY_OFF_PIN, OUTPUT);
  digitalWrite(RED_LED, LOW);    // Turn on the RED LED
  digitalWrite(BLUE_LED, HIGH);    // Turn on the RED LED
  digitalWrite(RELAY_ON_PIN, LOW);
  digitalWrite(RELAY_OFF_PIN, LOW);
  Serial.println("NDEF Reader");
  nfc.begin();
}

void loop(void) {
  Serial.println("Scan an NFC tag");

  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
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
          digitalWrite(RED_LED, LOW);
          digitalWrite(BLUE_LED, LOW);
          digitalWrite(RELAY_ON_PIN, HIGH);
          digitalWrite(RELAY_OFF_PIN, LOW);
          delay(1500);
          digitalWrite(RELAY_ON_PIN, LOW);
          digitalWrite(BLUE_LED, HIGH);
          digitalWrite(RELAY_ON_PIN, HIGH);
          delay(100);
          digitalWrite(RELAY_OFF_PIN, HIGH);
          delay(500);
          digitalWrite(RELAY_OFF_PIN, LOW);
        } else { // Wrong card, reject
          digitalWrite(BLUE_LED, LOW);
          digitalWrite(RED_LED, HIGH);
          delay(1500);
        }
        digitalWrite(BLUE_LED, HIGH);
        digitalWrite(RED_LED, LOW);

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
