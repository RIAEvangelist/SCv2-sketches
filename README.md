# SCv2-sketches
Sketches for SCv2

The primary sketch for productiuon is the [load share](https://github.com/RIAEvangelist/SCv2-sketches/blob/master/loadShare/loadShare.ino)

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
