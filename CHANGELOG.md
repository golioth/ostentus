<!-- Copyright (c) 2023 Golioth, Inc. -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Ability to read single byte and byte array from Ostentus
- Read firmware version number from Ostentus
- Read number of unoccupied fifo slots from Ostentus
- Command to reset Ostentus

### Changed
- Use hardware i2c instead of PIO
- General clean up of i2c peripheral flow for readability

## [0.1.1] - 2023-07-03

### Added
- ePaper display (full refresh)
- ePaper display (partial refresh)
- ePaper slideshow
- LED control
- Capacitive touch Left/Right to change slideshow pages
- Capacitive touch Top toggles summary page

- GitHub workflow to create draft release and add compiled binaries to it.

### Changed
- Update to most recent Golioth Zephyr SDK release v0.8.0 which uses:
  - nRF Connect SDK v2.5.0(NCS)
  - Zephyr v3.5.0
- Upgrade `golioth/golioth-zephyr-boards` dependency to [`v1.0.1`](https://github.com/golioth/golioth-zephyr-boards/tree/v1.0.1)
- Dependencies use https instead of ssh GitHub URLs
- libostentus removed from code base and included as a Zephyr module

### Fixed
- Fix build error when `CONFIG_LIB_OSTENTUS=n` on the `aludel_mini_v1_sparkfun9160` board.