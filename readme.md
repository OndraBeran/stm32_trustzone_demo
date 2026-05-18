# stm32_trustzone_demo
This project is an example application showcasing the use of TrustZone on STM32U5 in combination with the TROPIC01 Secure Element.

## How to run
### Prerequisites
- Visual Studio Code
- STM32Cube Extension for VSCode
- STM32CubeProgrammer
- cmake (installed and added to the path)

### Preparing the Nucleo board
By default, TrustZone is disabled on the U5 chips, enabling it requires programming the option bytes. Sepecifically:

- `TZEN` bit to 1 (found in *Option bytes -> User Configuration*)
- `SECWM2_PSTRT` to `0x01` (found in *Option bytes -> Secure Area 2*)
- `SECWM2_PEND` to `0x00`

To do so, the STM32CubeProgrammer can be used. Note that the `SECWM2_*` bytes may only appear after enabling TrustZone by setting the `TZEN` bit to 1.

### Required hardware

- TROPIC01 Arduino shield
- 3x4 Membrane keypad
- 16x2 LCD with I2C converter

Wiring diagram can be found in the thesis.

### Cloning the repo
Use the following command to copy the project including the libtropic submodule:
```bash
git clone --recursive git@github.com:OndraBeran/stm32_trustzone_demo.git
```

After opening the project with VSCode, navigate to the STM32Cube extesion. Click on *Set Up STM32Cube project(s) -> configure*.

After configuring, navigate to the Debug tab. There are 2 *Run and Debug* profiles. *default* runs the normal PIN verification application, *malicious* runs the "malicious" code used for testing the TrustZone configuration.

For more details, see chapters 6-8 of the thesis.


<img src="https://fit.cvut.cz/static/images/fit-cvut-logo-en.svg" alt="FIT CTU logo" height="200">

This software was developed with the support of the **Faculty of Information Technology, Czech Technical University in Prague**.
For more information, visit [fit.cvut.cz](https://fit.cvut.cz).
