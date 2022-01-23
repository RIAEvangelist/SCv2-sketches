#include <FlashStorage.h>

// Create a structure that is big enough to contain a name
// and a surname. The "valid" variable is set to "true" once
// the structure is filled with actual data for the first time.
typedef struct {
  long OUTPUT_VOLTS_END;
  short chargerCount;
  long MAX_STATION_WATTS;
  short RAMP_RATE;
  byte CPlus;
  char bleName[20];
} ChargingData;

// Reserve a portion of flash memory to store a "ChargingData" and
// call it "chargeData".
FlashStorage(chargeData, ChargingData);

// Note: the area of flash memory reserved lost every time
// the sketch is uploaded on the board.

void setup() {
  SERIAL_PORT_MONITOR.begin(9600);
  while (!SERIAL_PORT_MONITOR) { }

  // Create a "ChargingData" variable and call it "chargerConfig"
  ChargingData chargerConfig;

  // Read the content of "chargeData" into the "chargerConfig" variable
  chargerConfig = chargeData.read();

  // If this is the first run the "valid" value should be "false"...
  if (chargerConfig.valid == false) {

    // ...in this case we ask for user data.
    SERIAL_PORT_MONITOR.setTimeout(30000);
    SERIAL_PORT_MONITOR.println("Configure charger:");

    SERIAL_PORT_MONITOR.println("OUTPUT_VOLTS_END:");
    long userOUTPUT_VOLTS_END = SERIAL_PORT_MONITOR.readStringUntil('\n');

    SERIAL_PORT_MONITOR.println("chargerCount:");
    short userChargerCount = SERIAL_PORT_MONITOR.readStringUntil('\n');

    // Fill the "chargerConfig" structure with the data entered by the user...
    name.toCharArray(chargerConfig.name, 20);
    surname.toInt(chargerConfig.surname);
    // set "valid" to true, so the next time we know that we
    // have valid data inside
    chargerConfig.valid = true;

    // ...and finally save everything into "chargeData"
    chargeData.write(chargerConfig);

    // Print a confirmation of the data inserted.
    SERIAL_PORT_MONITOR.println();
    SERIAL_PORT_MONITOR.print("Your name: ");
    SERIAL_PORT_MONITOR.println(chargerConfig.name);
    SERIAL_PORT_MONITOR.print("and your surname: ");
    SERIAL_PORT_MONITOR.println(chargerConfig.surname);
    SERIAL_PORT_MONITOR.println("have been saved. Thank you!");

  } else {

    // Say hello to the returning user!
    SERIAL_PORT_MONITOR.println();
    SERIAL_PORT_MONITOR.print("Hi ");
    SERIAL_PORT_MONITOR.print(chargerConfig.name);
    SERIAL_PORT_MONITOR.print(" ");
    SERIAL_PORT_MONITOR.print(chargerConfig.surname);
    SERIAL_PORT_MONITOR.println(", nice to see you again :-)");

  }
}

void loop() {
  // Do nothing...
}
