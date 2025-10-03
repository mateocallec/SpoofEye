# Changelog

All notable changes to this project will be documented in this file.  
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),  
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2025-10-03
### Added
- Initial public release of **SpoofEye**.
- Automatic startup at user login without manual intervention.
- Detection of ARP spoofing and malicious DNS servers.
- Real-time user notifications on spoofing attempts.
- Event logging system:
  - Default log file: `~/.spoofeye/spoofeye.log`
- Configurable behavior via `.ini` configuration files.
- Ability to override default configuration paths.
- Command-line interface with the following options:
  - `--version` (`-v`) to print version.
  - `--print-config` (`--config`, `config`) to display current configuration.
  - `--help` (`-h`, `help`) to show usage information.
  - `--config-path` to override configuration file location.
- Debian packaging support with `debian/` and `package/` directories.
- Project documentation:
  - `README.md`, `LICENSE`, `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md`, `DISCLAIMER.md`.
- Test suite and supporting scripts in `tests/` and `scripts/`.
- Credits to external libraries and resources:
  - [nlohmann/json](https://github.com/nlohmann/json)
  - [trickest/resolvers](https://github.com/trickest/resolvers)

---

## [Unreleased]
### Planned
- Support for additional spoofing attack detection mechanisms.
- Enhanced logging and monitoring dashboard.
- Windows and macOS cross-platform support.
