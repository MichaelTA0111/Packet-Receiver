# Packet Receiver

This application is a packet receiver to accompany the DPDK packet processing application [CHERI Networking](https://gitlab.eeecs.qub.ac.uk/40266651/cheri_networking). This application should be used in conjunction with the IPC mode of operation of CHERI Networking.

## Build

Run the `make` command to build the application.

## Usage

Use the command `./application.o CONSUMER_NUMBER` to run the application manually, where `CONSUMER_NUMBER` is the integer ID of the consumer.

The `-q` flag or `PYTILIA_QUIET` environment variable will run the application in quiet mode.

The `-v` flag or `PYTILIA_VERBOSE` environment variable will run the application in verbose mode.

