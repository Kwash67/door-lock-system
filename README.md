# Door Lock System

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

A secure, embedded door lock system built with Mbed OS, supporting user and admin authentication, persistent user management, and a keypad interface. Designed for microcontroller platforms, this project demonstrates robust access control, flash-based credential storage, and modular embedded design.

## Features

- **User & Admin Authentication**: Separate roles for normal users and administrators
- **Keypad Input**: Hardware keypad for password entry
- **Persistent Storage**: User credentials stored in microcontroller flash memory
- **Admin Menu**: Add, remove, or update users and passwords
- **Visual Feedback**: SLCD display and LEDs for status indication
- **Lockdown Mode**: Security lockout after repeated failed attempts
- **Debounced Input**: Reliable keypad input handling

## Getting Started

### Prerequisites
- [Mbed CLI](https://os.mbed.com/docs/mbed-os/latest/build-tools/mbed-cli.html)
- Supported Mbed OS hardware (e.g., NXP/Freescale Kinetis)

### Building & Running

1. **Clone the repository:**
   ```sh
   git clone https://github.com/Kwash67/door-lock-system.git
   cd door-lock-system
   ```
2. **Import Mbed OS dependencies:**
   ```sh
   mbed deploy
   ```
3. **Build the project:**
   ```sh
   mbed compile -m <TARGET> -t <TOOLCHAIN>
   ```
   Replace `<TARGET>` and `<TOOLCHAIN>` with your board and toolchain (e.g., `K64F`, `GCC_ARM`).
4. **Flash to device:**
   - Drag-n-drop the `.bin` file to your board, or use `mbed flash`.

### Example Usage

On boot, the system prompts for a password via the keypad. Users and admins have different access:

```cpp
while (true) {
    if (tries <= max_tries) {
        while (!inputModule.hasPassword()) {
            inputModule.processInput();
        }
        if (inputModule.hasPassword()) {
            const char* password = inputModule.getEnteredPassword();
            auth = userManager.authenticate(password);
        }
        if (auth == 'u') {
            openDoor();
            inputModule.reset();
        } else if (auth == 'a') {
            userManager.launch_admin();
            inputModule.reset();
        } else if (auth == 'x') {
            // Failed attempt logic
        }
    } else {
        // Lockdown logic
    }
}
```

## Support & Documentation
- For issues, use the [GitHub Issues](../../issues) page
- See [`CONTRIBUTING.md`](CONTRIBUTING.md) for contribution guidelines
- License: See [`LICENSE`](LICENSE)
- Mbed OS docs: [os.mbed.com/docs/mbed-os/latest/](https://os.mbed.com/docs/mbed-os/latest/)

## Maintainers & Contributions
- Maintained by: [Kwame Andoh](https://github.com/Kwash67)
- Contributions welcome! Please see [`CONTRIBUTING.md`](CONTRIBUTING.md)

---

*This project is not affiliated with Arm Mbed. For hardware support, visit the [Mbed forums](https://forums.mbed.com/).*

## Demo Video

See the system in action on the FRDM KL46Z board:

[Watch the demo video](https://drive.google.com/file/d/1tYJgLYylGV4k29tyH4k7WVyckEmvQu-z/view?resourcekey)
