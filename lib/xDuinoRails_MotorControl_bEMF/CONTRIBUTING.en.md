# Contributing

Thank you for your interest in contributing to the `xDuinoRails_MotorControl_bEMF` project. This document outlines the guidelines and best practices for contributing.

## How to Contribute

We welcome contributions in the form of bug reports, feature requests, and pull requests.

- **Bug Reports:** If you find a bug, please open an issue on GitHub and provide a detailed description of the problem, including the hardware you are using and steps to reproduce the issue.
- **Feature Requests:** If you have an idea for a new feature, please open an issue to discuss it. This allows us to align on the design and implementation before you start coding.
- **Pull Requests:** We welcome pull requests for bug fixes and new features. Before submitting a pull request, please ensure that your code adheres to the guidelines in this document.

## Development Guidelines

### Code Style

- **Naming Conventions:** Use `snake_case` for file and variable names.
- **Comments:** Add comments to explain complex logic or magic numbers.
- **Variable Alignment:** Align variable assignments with the `=` sign, and right-align numbers.

### Documentation

- All documentation is located in the `docs` directory.
- Use `UPPER_SNAKE_CASE` for all `.md` filenames.
- When you make changes, please update the relevant documentation files (`HOW_TO_USE.md`, `USER_MANUAL.md`, `CORE_CONCEPTS.md`, `DEVELOPER_REFERENCE.md`, `TECHNICAL_DEBTS.md`).

### Testing

- Add a comment to each test case, summarizing its goal and the steps it performs.
- Reference the underlying specification or documentation for each test case.

## Out of Scope

The following topics are out of scope for this project:
- Digital model railroad protocols (e.g., DCC, RailCom, ACC).
- User interfaces and communication (e.g., CLI, OLED displays, web servers, BLE).

We look forward to your contributions!
