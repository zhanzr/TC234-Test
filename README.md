TC234 Demo.

The Die Temperature Sensor (DTS) generates a measurement result that indicates
directly the current temperature. The result of the measurement is displayed via bit field
DTSSTAT.RESULT. In order to start one measurement bit DTSCON.START needs to
be set.
The DTS has to be enabled before it can be used via bit DTSCON.PWD. When the DTS
is powered after the start-up time of the DTS (defined in the Data Sheet) a temperature
measurement can be started.
Note: If bit field DTSSTAT.RESULT is read before the first measurement was finished it
will return 0x000.
When a measurement is started the result is available after the measurement time
passed. If the DTS is ready to start a measurement can be checked via bit
DTSSTAT.RDY. If a started measurement is finished or still in progress is indicated via
the status bit DTSSTAT.BUSY. The measurement time is also defined in the Data Sheet.
In order to adjust production variations bit field DTSCON.CAL is available.
DTSCON.CAL is automatically programmed with a predefined value.
Note: The first measurement after the DTS was powered delivers a result without
calibration adjustment and should be ignored therefore.
The formula to calculate the die temperature is defined in the Data Sheet.
Note: The maximum resolution is only achieved for a measurement that is part of
multiple continuous measurements (recommended to ignore are two here).
An interrupt service request (SRC_SCUDTS) is generated when DTS busy indication
changes from 1 to 0.
