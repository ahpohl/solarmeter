# Aurora Solarmeter

Aurora is a series of indoor and outdoor photovoltaic inverters made by PowerOne and later bought by ABB and Fimer. They do not offer an ethernet connection and cloud data storage, but have the possibility to read the data using a serial RS485 connection.

Solarmeter provides the necessary software to read the data from the inverter. The daemon reads the energy production, outputs a JSON formatted string and sends it to a MQTT broker on the network. From there, the data is forwarded into TimescaleDB for permanent data storage and for visualization with Grafana (all necessary files are provided in the resources folder). My complete software stack is described in detail in the [Smartmeter](https://github.com/ahpohl/smartmeter/wiki) wiki and consists of the following components:

- Solarmeter daemon with RS485 dongle for data readout
- Mosquitto MQTT broker
- Node-RED
- PostgreSQL with TimescaleDB
- Grafana for visualization

The Aurora converter needs to be connected to the host USB with a [RS485-to-USB adapter](https://www.waveshare.com/wiki/USB_TO_RS232/485/TTL) such as the industrial adapter from Waveshare.

## Build instructions

Solarmeter depends on [libabbaurora](https://github.com/ahpohl/libabbaurora), a library which provides the necessary low level methods to access the inverter and on [libmosquitto](https://mosquitto.org/) for connecting to the MQTT broker. Packages of [solarmeter](https://aur.archlinux.org/packages/solarmeter) and [libabbaurora](https://aur.archlinux.org/packages/libabbaurora) are published in the AUR. If you need to compile Solarmeter directly from source, a Makefile for (cross-)compilation is provided.

```
make CROSS_COMPILE=aarch64-unknown-linux-gnu
make install
```

Instructions how to setup a cross toolchain and how to compile and install libmosquitto into the toolchain can be found [here](https://github.com/ahpohl/froniusd/blob/master/resources/mosquitto/README.md). 

## Installation

The Arch Linux package provides a systemd service file and installs the configuration file in `/etc/solarmeter/solarmeter.conf`. The service can be started with:

```
systemctl start solarmeter
```

When building directly from source, the daemon can be directly started with:

```
solarmeter --config resources/config/solarmeter_example.conf

```

## Run in Docker

The Arch Linux package installs a docker compose in `/etc/solarmeter/docker-compose.yaml` and a `Dockerfile`. Alternatively, these files are also in the root folder or the source. The serial device needs to be set in `docker-compose.yaml` in addtion to `solarmeter.conf` to give Docker access to the USB-to-RS485 adapter.

```
docker-compose up --build
```

## Configuration

A minimal `solarmeter.conf` config file looks like this:

```
serial_device /dev/ttyUSB0
mqtt_topic solarmeter
mqtt_broker localhost
payment_kwh 0.3914
```

A fully commented example is provided in the resources section.

## JSON output

Example JSON string published by Solarmeter in the solarmeter/live topic:

```
[{
    "time":                    # Timestamp, sec since epoch
    "total_energy":54361.99,   # AC total energy counter, kWh
    "voltage_p1":298.09,       # DC voltage string 1, V
    "current_p1":0.59292,      # DC current string 1, A
    "power_p1":170.23,         # DC power string 1, W
    "voltage_p2":296.91,       # DC voltage string 1, V
    "current_p2":0.58406,      # DC current string 2, A
    "power_p2":167.23,         # DC power string 2, W
    "grid_voltage":233.50,     # AC voltage, V
    "grid_current":1.32307,    # AC current, A
    "grid_power":333.79,       # AC active power, W
    "frequency":49.966,        # AC active power, W
    "efficiency":98.91,        # AC conversion efficiency
    "inverter_temp":31.11,     # Inverter circuit temperature (DC/AC), °C
    "booster_temp":30.70,      # Booster circuit temperature (DC/DC), °C
    "r_iso":20.00              # DC Insulation resistance
},
{
    "payment":0.3914           # Payment per kWh, € (or whatever)
    "serial_num":              # Sequential serial number
    "part_num":                # ABB product identification code
    "mfg_date":                # Manufacturing week and year
    "firmware":                # Firmware version installed
    "inverter_type":           # Indoor/outdoor and type
    "grid_standard":           # Grid standard
}]
```