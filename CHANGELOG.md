# Solarmeter changelog

## v0.1.5 - 2024-10-26
* add docker-compose support
* fix build errors
* update readme
* add cross compile support
* add llvm clang code formatting
* postgres: add average power and max power columns to database schema
* postgres: updated retention policies

## v0.1.4 - 2022-12-29
* fix mqtt status topic not retained
* automatically reconnect to broker

## v0.1.3 - 2022-10-30
* TimescaleDB: replaced \_time\_bucket() with timescaledb\_experimental.time\_bucket\_ng()
* created yearly\_view

## v0.1.2 - 2022-01-02
* set debug level in config file

## v0.1.1 - 2021-12-29
* add dockerfile for Synology (contribution by jekkos)
* retain mqtt messages

## v0.1.0 - 2021-10-08
* initial release
