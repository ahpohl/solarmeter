# Aurora Solarmeter

The Solarmeter daemon is built upon the [libabbaurora](https://ahpohl.github.io/libabbaurora/) library to communicate with the ABB Aurora inverter. The daemon reads the energy production from the inverter, outputs a JSON formatted string and sends it to a MQTT broker on the network. From there, the data is forwarded into a time series database for permanent data storage and for visualization. The complete software stack consists of the following components, which need to be installed and configured separately:
- Solarmeter daemon with RS485 dongle for data readout
- Mosquitto MQTT broker
- Node-RED (MQTT client, PostgreSQL and optional email alerts)
- PostgreSQL with TimescaleDB and pg_cron extensions
- Grafana for visualization

The software stack is light weight in terms of necessary resources and runs on any SBC such as an Odroid C2/C4 or Raspberry Pi 3/4.

## Changelog

All notable changes and releases are documented in the [CHANGELOG](CHANGELOG.md).

## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details
