#include <OneWire.h>
#include "Ringbuffer.h"
#define DS2413_ONEWIRE_PIN (33)
#define DS2413_FAMILY_ID 0x3A
#define DS2413_ACCESS_READ 0xF5
#define DS2413_ACCESS_WRITE 0x5A
#define DS2413_ACK_SUCCESS 0xAA
#define DS2413_ACK_ERROR 0xFF
TaskManager manager;
int DS2413TaskId;
int addToRingbufferTaskId;
OneWire oneWire(DS2413_ONEWIRE_PIN);
uint8_t address[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int IOint = 0;
int IOA = 0;
int IOB = 2;
RingBuffer<int>  touchBuffer(30);
#define TOUCH_A 14
#define TOUCH_B 11
#define TOUCH_BOTH 15
#define TOUCH_NONE 10


void printBytes(uint8_t *addr, uint8_t count, bool newline = 0) {
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(addr[i] >> 4, HEX);
    Serial.print(addr[i] & 0x0f, HEX);
    Serial.print(" ");
  }
  if (newline) {
    Serial.println();
  }
}

byte read(void) {
  bool ok = false;
  uint8_t results;
  oneWire.reset();
  oneWire.select(address);
  oneWire.write(DS2413_ACCESS_READ);

  results = oneWire.read();                 /* Get the register results   */
  ok = (!results & 0x0F) == (results >> 4); /* Compare nibbles            */
  results &= 0x0F;                          /* Clear inverted values      */

  oneWire.reset();

  // return ok ? results : -1;
  return results;
}

bool write(uint8_t state) {
  uint8_t ack = 0;

  /* Top six bits must '1' */
  state |= 0xFC;

  oneWire.reset();
  oneWire.select(address);
  oneWire.write(DS2413_ACCESS_WRITE);
  oneWire.write(state);
  oneWire.write(~state); /* Invert data and resend     */
  ack = oneWire.read();  /* 0xAA=success, 0xFF=failure */
  if (ack == DS2413_ACK_SUCCESS) {
    oneWire.read(); /* Read the status byte      */
  }
  oneWire.reset();

  return (ack == DS2413_ACK_SUCCESS ? true : false);
}



bool DS2413Task() {
  uint8_t state = read();
  IOint = int(state);
  return true;
}

bool addToRingbufferTask(){
  touchBuffer = IOint;
  Serial.printf("\ncurrent touchBuffer: ");
  for (int i= 0; i < touchBuffer.count(); i++) {
    Serial.printf(" %d ,",touchBuffer[i]);
  }

  if( touchBuffer.get(touchBuffer.count()-1) == TOUCH_NONE) { //if touch is released

   touchBuffer.clear();
   Serial.printf("buffer cleared");
  }

  return true;
}




void setupRingbuffer() {
  Serial.println(F("Ringbuffer started"));
  addToRingbufferTaskId = manager.addTask(addToRingbufferTask, 75,true);
}

void setupDS2413(){
  DS2413TaskId = manager.addTask(DS2413Task, 75, true);

  Serial.println(F("Looking for a DS2413 on the bus"));

  /* Try to find a device on the bus */
  oneWire.reset_search();
  delay(250);
  if (!oneWire.search(address)) {
    printBytes(address, 8);
    Serial.println(F("No device found on the bus!"));

    oneWire.reset_search();
    while (1)
      ;
  }

  /* Check the CRC in the device address */
  if (OneWire::crc8(address, 7) != address[7]) {
    Serial.println(F("Invalid CRC!"));

    while (1)
      ;
  }

  /* Make sure we have a DS2413 */
  if (address[0] != DS2413_FAMILY_ID) {
    printBytes(address, 8);
    Serial.println(F(" is not a DS2413!"));
    while (1)
      ;
  }

  Serial.print(F("Found a DS2413: "));
  printBytes(address, 8);
  Serial.println(F(""));
}


void setup() {
  Serial.begin(115200);
  setupDS2413();
  setupRingbuffer();
}

void loop() {
  manager.runTasks();
}
