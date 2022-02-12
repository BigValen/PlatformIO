#include <main.h>
#include <Wire.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <NfcAdapter.h>

#define RED_PIN 3
#define GREEN_PIN 4
#define BLUE_PIN 5
#define RELAY_PIN 7

//#define NDEF_DEBUG 1
//#define NFC_DEBUG 1

#define SECRET "garageopen" 

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);


void setup(void) {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  delay(2000);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);

  Serial.println("Garage Door Reader!");
  nfc.begin();
  delay(300);
}

void loop(void) {

  Serial.println("Scan an NFC tag");
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
#ifdef NFC_DEBUG
    Serial.print("Tag Type: "); Serial.println(tag.getTagType());
    Serial.print("UID: ");Serial.println(tag.getUidString());
#endif

    if (tag.hasNdefMessage()) {
      NdefMessage message = tag.getNdefMessage();
#ifdef NFC_DEBUG
      Serial.print("\nThis NFC Tag contains an NDEF Message with ");
      Serial.print(message.getRecordCount());
      Serial.print(" NDEF Record");
      if (message.getRecordCount() != 1) {
        Serial.print("s");
      }
      Serial.println(".");
#endif

      // cycle through the records, printing some info from each
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++) {
        NdefRecord record = message.getRecord(i);
#ifdef NFC_DEBUG
        Serial.print("\nNDEF Record ");Serial.println(i+1);
        Serial.print("  TNF: ");Serial.println(record.getTnf());
        Serial.print("  Type: ");Serial.println(record.getType()); // will be "" for TNF_EMPTY
#endif

        // The TNF and Type should be used to determine how your application processes the payload
        // There's no generic processing for the payload, it's returned as a byte[]
        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);

#ifdef NFC_DEBUG
        Serial.print("  Payload (HEX): ");
        PrintHexChar(payload, payloadLength);
#endif

        // Force the data into a String (might work depending on the content)
        // Real code should use smarter processing
        String payloadAsString = "";
        for (int c = 0; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
#ifdef NFC_DEBUG
        Serial.print("  Payload (as String): ");
        Serial.println(payloadAsString);
#endif
        if(payloadAsString.substring(3) == SECRET) {
          accept();
        } else { // Wrong card, reject
          reject(payloadAsString.substring(3));
        }
      }
    } else {
      reject("No ndef msg");
    }
  }
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  delay(200);
}

void reject(String msg){
  Serial.print("  open rejected!"); Serial.println(msg);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);
  delay(1500);
}

void accept(void){
  Serial.println("  opening garage!");
  digitalWrite(RED_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  delay(1500);
}