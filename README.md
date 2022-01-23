# SCv2-sketches
Sketches for SCv2

The primary sketch for productiuon is the [load share](https://github.com/RIAEvangelist/SCv2-sketches/blob/master/loadShare/loadShare.ino)

#### Licensed under MIT license
See the [MIT license](https://github.com/RIAEvangelist/SCv2-sketches/blob/master/license) file.

# Primary Code Functionality

* Calculates and shares target wattage among upto 3 SCv2 modules
* normalizes output power
* normalizes output voltage
* defaults to 6600.00 watts target charge rate from station
* stores new target charge rate to eeprom when modified by user via api
* starts up with target charge rate wattage from eeprom or defualt if not set
* ramps up at 50 watts per second
* ramps down instantly
* cuts charge rate back to 75% of target wattage over 113.5v if over 1800.00 watts

# API COMMANDS

Version 2 of the API accepts one command, change target wattage * 100 
Every command must be ended by a newline char of ` \n `

1kw = ` 100000 ` you can look at this as 1000.00 watts


# API PUSHED DATA

```javascript

    //battery volts
    Serial.print(volts);
    Serial.print(",");

    //total charging amps
    Serial.print(amps);
    Serial.print(",");

    //total charging amps per charger
    Serial.print(amps/chargerCount);
    Serial.print(",");

    //charger count
    Serial.print(chargerCount);
    Serial.print(",");

    //target watts
    Serial.print(watts);
    Serial.print(",");

    //target max watts
    Serial.print(MAX_STATION_WATTS);
    Serial.print(",");

    //target total amps
    Serial.print(chargingAmps*chargerCount);
    Serial.print(",");

    //target amps per charger
    Serial.print(chargingAmps);

    //start extending with a comma
    Serial.print(",");

    //print api version
    Serial.println(VERSION);
  
```
