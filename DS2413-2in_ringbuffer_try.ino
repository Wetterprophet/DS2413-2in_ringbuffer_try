#include <OneWire.h>

#define DS2413_ONEWIRE_PIN  (33)

#define DS2413_FAMILY_ID    0x3A
#define DS2413_ACCESS_READ  0xF5
#define DS2413_ACCESS_WRITE 0x5A
#define DS2413_ACK_SUCCESS  0xAA
#define DS2413_ACK_ERROR    0xFF

OneWire oneWire(DS2413_ONEWIRE_PIN);
uint8_t address[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int IOA = 0;
int IOB = 2;
boolean on = 0;

void printBytes(uint8_t* addr, uint8_t count, bool newline = 0)
{
  for (uint8_t i = 0; i < count; i++)
  {
    Serial.print(addr[i] >> 4, HEX);
    Serial.print(addr[i] & 0x0f, HEX);
    Serial.print(" ");
  }
  if (newline)
  {
    Serial.println();
  }
}

byte read(void)
{
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

bool write(uint8_t state)
{
  uint8_t ack = 0;

  /* Top six bits must '1' */
  state |= 0xFC;

  oneWire.reset();
  oneWire.select(address);
  oneWire.write(DS2413_ACCESS_WRITE);
  oneWire.write(state);
  oneWire.write(~state);                    /* Invert data and resend     */
  ack = oneWire.read();                     /* 0xAA=success, 0xFF=failure */
  if (ack == DS2413_ACK_SUCCESS)
  {
    oneWire.read();                          /* Read the status byte      */
  }
  oneWire.reset();

  return (ack == DS2413_ACK_SUCCESS ? true : false);
}

void setup(void)
{
  //BeanHid.enable();

  Serial.begin(115200);

  Serial.println(F("Looking for a DS2413 on the bus"));

  /* Try to find a device on the bus */
  oneWire.reset_search();
  delay(250);
  if (!oneWire.search(address))
  {
    printBytes(address, 8);
    Serial.println(F("No device found on the bus!"));
    //Bean.setLed(255, 0, 0);
    //Bean.sleep(3000);
    //Bean.setLed(0, 0, 0);


    oneWire.reset_search();
    while (1);
  }

  /* Check the CRC in the device address */
  if (OneWire::crc8(address, 7) != address[7])
  {
    Serial.println(F("Invalid CRC!"));
    //Bean.setLed(255, 70, 0);
    //Bean.sleep(3000);
    //Bean.setLed(0, 0, 0);
    while (1);
  }

  /* Make sure we have a DS2413 */
  if (address[0] != DS2413_FAMILY_ID)
  {
    printBytes(address, 8);
    Serial.println(F(" is not a DS2413!"));
    //Bean.setLed(255, 70, 0);
    //Bean.sleep(3000);
    //Bean.setLed(0, 0, 0);
    while (1);
  }

  Serial.print(F("Found a DS2413: "));
  //Bean.setLed(0, 255, 0);
  //Bean.sleep(3000);
  //Bean.setLed(0, 0, 0);

  printBytes(address, 8);
  Serial.println(F(""));
}

void loop(void)
{

  /* Read */
  //  delay(10);
  uint8_t state = read();
  //  String IO = String(state);
  int IOint = int(state);
  //  Serial.println(state, HEX);
  //  Serial.println(state, BIN);
  //  Serial.print (F("IO= "));
  //  Serial.println(IO);



  if (state == -1) {
    Serial.println(F("Failed reading the DS2413"));
  }
  /*
    else if (IO == "10") {
    Serial.println(F("A low,  B low"));
    }
    else if (IO == "14") {
    Serial.println(F("A high, B low"));
    }
    else if (IO == "11") {
    Serial.println(F("A low,  B high"));
    }
    else if (IO == "15") {
    Serial.println(F("A high, B high"));
    }
  */


  switch (IOint) {
    case 10:
      //      Serial.println("A low,  B low");
      //Bean.setLed(0, 0, 0);
      IOA = 0;
      IOB = 2;
      break;
    case 14:
      //      Serial.println("A high, B low");
      //Bean.setLed(255, 0, 0);
      //BeanHid.sendMediaControl(VOLUME_DOWN);
      //delay(20);

      IOA = 1;
      IOB = 2;
      break;
    case 11:
      //      Serial.println("A low , B high");
      //Bean.setLed(0, 255, 0);
      //BeanHid.sendMediaControl(VOLUME_UP);
      //delay(20);

      IOA = 0;
      IOB = 3;
      break;
    case 15:
      //      Serial.println("A high, B high");
      //Bean.setLed(0, 0, 255);
      if (on == 0) {
        //BeanHid.sendMediaControl(PLAY);
        on = 1;
      } else {
        //BeanHid.sendMediaControl(STOP);
        on = 0;
      }

      //delay(200);

      IOA = 1;
      IOB = 3;
      break;
  }
  Serial.print(F("IOA = "));
  Serial.print(IOA);
  Serial.print(F(", IOB = "));
  Serial.println(IOB);

  /* Write */
  /*
    bool ok = false;
    ok = write(0x0);
    if (!ok) Serial.println(F("Wire failed"));
    delay(5000);
    ok = write(0x1);
    if (!ok) Serial.println(F("Wire failed"));
    delay(1000);
    ok = write(0x2);
    if (!ok) Serial.println(F("Wire failed"));
    delay(1000);
    ok = write(0x3);
    delay(1000);
  */
}
