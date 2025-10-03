# Contributing to SpoofEye

First off, thank you for considering contributing to **SpoofEye**! 🎉 Your help improves the project and the experience for everyone. This document outlines how you can contribute effectively.

---

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Reporting Issues](#reporting-issues)
- [Submitting Pull Requests](#submitting-pull-requests)
- [Coding Standards](#coding-standards)
- [Documentation](#documentation)
- [License](#license)

---

## Code of Conduct
By participating in SpoofEye, you agree to follow our [Code of Conduct](CODE_OF_CONDUCT.md).  
We aim for a welcoming, respectful, and inclusive community.

---

## Getting Started
1. **Fork the repository** on GitHub.
2. **Clone your fork locally**:
   ```bash
   git clone https://github.com/mateocallec/SpoofEye.git
````

3. **Create a new branch** for your feature or bugfix:

   ```bash
   git checkout -b feature/your-feature-name
   ```
4. Install any dependencies listed in the repository and ensure you can build and run SpoofEye locally.

---

## Reporting Issues

When reporting bugs or suggesting features:

* Use clear and descriptive titles.
* Include steps to reproduce the issue.
* Attach screenshots or logs if relevant.
* Tag the issue appropriately (bug, enhancement, question, etc.).

We use GitHub Issues for tracking, so please submit issues there.

---

## Submitting Pull Requests

1. Ensure your code passes tests and builds cleanly.
2. Write **clear commit messages** explaining your changes.
3. Make sure your code adheres to our coding standards (see below).
4. Open a pull request (PR) from your branch to the `main` branch.
5. Include a brief description of your changes and why they are needed.

Your PR will be reviewed by maintainers. Constructive feedback may require changes before merging.

---

## Coding Standards

SpoofEye follows these guidelines to keep the code clean and maintainable:

* **Language:** C/C++ (adjust if using other languages)
* **Formatting:** Use consistent indentation and spacing.
* **Naming:** Use descriptive variable and function names.
* **Comments:** Use **Doxygen-style comments** for functions, classes, and major logic blocks.
* **Commit Messages:** Use imperative tense, e.g., `Add feature X` or `Fix bug in Y`.

**Example Doxygen Comment:**

```cpp
/**
 * @brief Calculates the checksum of a given buffer.
 * 
 * @param buffer Pointer to the data buffer.
 * @param length Length of the buffer in bytes.
 * @return uint32_t Calculated checksum.
 */
uint32_t calculateChecksum(const uint8_t* buffer, size_t length);
```

---

## Documentation

* Keep documentation updated along with code changes.
* Use Doxygen comments for all public functions, classes, and modules.
* Ensure README and any user-facing docs reflect your changes.

---

## License

By contributing, you agree that your contributions will be licensed under the [GPLv3 License](LICENSE).

---

Thank you for helping improve **SpoofEye**!
Your contributions make this project better for everyone.
