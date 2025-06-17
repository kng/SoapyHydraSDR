# Soapy SDR plugin for HydraSDR

An initial Soapy driver and needs some further testing when the SDR is available. This is a port of the SoapyAirspy driver.

## Dependencies

* SoapySDR - https://github.com/pothosware/SoapySDR/wiki
* libhydrasdr - https://github.com/hydrasdr/rfone_host


`SoapySDRUtil --probe="driver=hydrasdr"`

```
######################################################
##     Soapy SDR -- the SDR abstraction library     ##
######################################################

Probe device driver=hydrasdr

----------------------------------------------------
-- Device identification
----------------------------------------------------
  driver=HydraSDR
  hardware=HydraSDR
  serial=36b463dc395884c7

----------------------------------------------------
-- Peripheral summary
----------------------------------------------------
  Channels: 1 Rx, 0 Tx
  Timestamps: NO
  Other Settings:
     * Bias tee - Enable the 4.5v DC Bias tee to power LNA / etc. via antenna connection.
       [key=biastee, default=false, type=bool]
     * Bit Pack - Enable packing 4 12-bit samples into 3 16-bit words for 25% less USB trafic.
       [key=bitpack, default=false, type=bool]

----------------------------------------------------
-- RX Channel 0
----------------------------------------------------
  Full-duplex: NO
  Supports AGC: YES
  Stream formats: CS16, CF32
  Native format: CS16 [full-scale=32767]
  Antennas: RX
  Full gain range: [0, 45] dB
    LNA gain range: [0, 15] dB
    MIX gain range: [0, 15] dB
    VGA gain range: [0, 15] dB
  Full freq range: [24, 1800] MHz
    RF freq range: [24, 1800] MHz
  Sample rates: 10, 5, 2.5 MSps
```
