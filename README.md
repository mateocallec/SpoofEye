<div align="center">
    <a href="https://github.com/mateocallec/SpoofEye"><img src="https://github.com/mateocallec/SpoofEye/blob/main/docs/img/icon-2048x2048.png?raw=true" alt="SpoofEye" height="217" /></a>
</div>

<div>&nbsp;</div>

<div align="center">
    <a href="https://github.com/mateocallec/SpoofEye/releases"><img src="https://img.shields.io/github/v/release/mateocallec/SpoofEye?label=lastest%20release&color=blue&style=flat-square" alt="Latest release" /></a>
    <a href="https://rojo.space/docs"><img src="https://img.shields.io/github/license/mateocallec/SpoofEye?label=license&color=white&style=flat-square" alt="License" /></a>
    <a href="https://github.com/mateocallec/SpoofEye/releases"><img src="https://img.shields.io/badge/platform-Linux-darkred" alt="Supported platforms" /></a>
</div>

<hr />

**SpoofEye** is a security tool designed to protect end systems against common network spoofing attacks such as ARP spoofing, malicious DNS servers, and similar threats. It runs automatically at each user login, continuously monitoring the system for suspicious activity.

Whenever a spoofing attempt is detected, **SpoofEye** immediately logs the event and can send notifications to alert the user.

---

## Version

**SpoofEye v1.0.0**
Author: Mateo Florian Callec
License: GNU General Public License v3
Copyright (c) 2025

Repository: [https://github.com/mateocallec/SpoofEye.git](https://github.com/mateocallec/SpoofEye.git)
Contact: [mateo@callec.net](mailto:mateo@callec.net)

---

## Features

* Automatic startup at each user login (no manual launch required).

* Detects ARP spoofing and malicious DNS servers.

* Sends real-time notifications when a spoofing attempt is detected.

* Logs all detected events for later analysis.

* Log file is stored at:

  ```
  ~/.spoofeye/spoofeye.log
  ```

* Configurable behavior via `.ini` configuration files.

* Supports overriding default configuration paths.

---

## Usage

Although **SpoofEye** runs automatically at login, you can also launch it manually or use command-line options for configuration and debugging.

```bash
spoofeye COMMAND
```

### Available Commands

| Command          | Aliases              | Description                              |
| ---------------- | -------------------- | ---------------------------------------- |
| `--version`      | `-v`                 | Print software version (stops execution) |
| `--print-config` | `--config`, `config` | Print software configuration             |
| `--help`         | `-h`, `help`         | Show this help message (stops execution) |
| `--config-path`  | N/A                  | Override configuration file path         |

### Examples

```bash
spoofeye --version
spoofeye --config-path /path/to/config.ini
spoofeye --help
```

---

## Repository Structure

| File / Folder        | Description                                                                                                                                           |
| -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| `CODE_OF_CONDUCT.md` | Guidelines for contributing respectfully to the project                                                                                               |
| `CONTRIBUTING.md`    | Instructions for contributing to SpoofEye                                                                                                             |
| `SECURITY.md`        | Guidelines for reporting security vulnerabilities                                                                                                     |
| `DISCLAIMER.md`      | Legal disclaimers and usage warnings                                                                                                                  |
| `LICENSE`            | GNU General Public License v3                                                                                                                         |
| `CHANGELOG.md`       | List of changes by version                                                                                                                            |
| `README.md`          | Project overview and instructions                                                                                                                     |
| `Makefile`           | Build instructions and automation                                                                                                                     |
| `include/`           | Header files (`.hpp`) and libraries                                                                                                                   |
| `src/`               | Source code files (`.cpp`)                                                                                                                            |
| `scripts/`           | Scripts for building, development, and testing                                                                                                        |
| `resources/`         | Configuration files and other resources                                                                                                               |
| `tests/`             | Test files for verifying software functionality                                                                                                       |
| `docs/`              | Additional documentation                                                                                                                              |
| `debian/`            | Clean build folder containing all Debian packaging files and scripts for SpoofEye. Generated/managed during the build process.                        |
| `package/`           | Source files for SpoofEye that will be included in the Debian package (binaries, service files, configuration, desktop entry, logrotate, docs, etc.). |

---

## Logs

By default, all events detected by **SpoofEye** are written to:

```
~/.spoofeye/spoofeye.log
```

This log can be monitored in real time using:

```bash
tail -f ~/.spoofeye/spoofeye.log
```

---

## Credits

* [nlohmann/json](https://github.com/nlohmann/json) – JSON library for C++
* [trickest/resolvers](https://github.com/trickest/resolvers?tab=readme-ov-file) – DNS resolver resources

---

## License

This program is free software: you can **redistribute** it and/or **modify** it under the terms of the GNU General Public License as published by the Free Software Foundation, either **version 3 of the License, or (at your option) any later version**.

This program is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For details, see the [GNU General Public License](LICENSE).

You should have received a copy of the GNU General Public License along with this program. If not, see [https://www.gnu.org/licenses/](https://www.gnu.org/licenses/).
