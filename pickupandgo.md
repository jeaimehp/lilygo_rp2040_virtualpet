# Pick up and go

## User intent and current state

This project makes an animated squirrel digital pet named Acorn on a LILYGO
T-PicoC3. The user asked to write the program, flash the Pico, initialize a Git
repository, and preserve notes. All of those actions are complete.

This session focuses **only on PicoArduino**, the RP2040 side of the T-PicoC3.
Another connected board is a Particle Xenon; do not operate on it.

## Location and files

- Repository: `/Users/jeaimehp/Documents/Arduino/SquirrelPet`
- Sketch: `SquirrelPet.ino`
- Setup and build instructions: `README.md`
- Implementation and flash history: `NOTES.md`
- Local Adafruit libraries: `dependencies/`
- Compiled firmware: `build/SquirrelPet.ino.uf2` (ignored by Git)
- Git branch: `main`; no remote has been configured.
- Initial commits: `460e6bd` (pet program) and `5a0adbc` (dependencies and notes).

## Board identification — verify before uploading

- Manufacturer/model: LILYGO T-PicoC3, with RP2040 and ESP32-C3.
- USB product on the RP2040 side: `PicoArduino`.
- USB vendor/product: `2E8A:000A`.
- USB serial: `DE616C825776382A`.
- Last known upload port: `/dev/cu.usbmodem124301`.
- Corresponding tty: `/dev/tty.usbmodem124301`.
- `/dev/cu.usbmodem21401` belongs to the Xenon and is outside this project's scope.
- Serial port names can change after reconnecting. Match the USB serial rather
  than assuming the old port still identifies the same board.
- The reversible USB-C connection can select the RP2040 or ESP32-C3 side.

USB inspection commands:

```sh
ioreg -r -c IOSerialBSDClient -l
ioreg -p IOUSB -l -w 0
```

Official hardware documentation: https://github.com/Xinyuan-LilyGO/T-PicoC3

## Display and controls

- Built-in 1.14-inch ST7789V LCD, 135 × 240 pixels; sketch uses 240 × 135 landscape.
- GPIO mapping: SCK 2, MOSI 3, CS 5, DC 1, RESET 0, backlight 4, power enable 22.
- Buttons: GPIO 6 and GPIO 7, active low with internal pull-ups.
- Button 1 feeds; button 2 plays; both toggle sleep. A single button wakes Acorn.
- Pet blinks and swishes a curled tail, eats an acorn, and plays with a ball.
- Food, joy, and energy change every 10 seconds. State resets after reboot.
- Rendering uses an allocated 64,800-byte RGB565 framebuffer to avoid flicker.
- No Wi-Fi, external image assets, or ESP32-C3 firmware changes are needed.

## Toolchain and dependency issue

Installed on October 9, 2026:

- Arduino CLI 1.5.1 via Homebrew, executable `/opt/homebrew/bin/arduino-cli`.
- Arduino-Pico core 6.3.0 under `/Users/jeaimehp/Library/Arduino15/packages/rp2040`.
- Board FQBN: `rp2040:rp2040:rpipico` (Earle Philhower's core).

The existing Adafruit display library in
`/Users/jeaimehp/Documents/Arduino/libraries` contained numbered duplicate
`.cpp` files, causing multiple-definition linker errors. A local copy in
`dependencies/` excludes those duplicates. **Use `--library` to explicitly
select the local display library.** Passing only `--libraries` selected the
broken global copy during the earlier build.

## Rebuild and flash

```sh
cd /Users/jeaimehp/Documents/Arduino/SquirrelPet
arduino-cli compile --fqbn rp2040:rp2040:rpipico --library /Users/jeaimehp/Documents/Arduino/SquirrelPet/dependencies/Adafruit_ST7735_and_ST7789_Library --output-dir /Users/jeaimehp/Documents/Arduino/SquirrelPet/build /Users/jeaimehp/Documents/Arduino/SquirrelPet
arduino-cli upload --fqbn rp2040:rp2040:rpipico --port /dev/cu.usbmodem124301 --input-dir /Users/jeaimehp/Documents/Arduino/SquirrelPet/build /Users/jeaimehp/Documents/Arduino/SquirrelPet
```

Uploading replaces the RP2040 program. The user authorized flashing this
squirrel-pet program. Follow any later user instructions for new changes.
In the prior sandbox, serial configuration required an escalated command;
use the tool's approval mechanism if access is blocked.

The successful uploader automatically reset the serial port, found
`/Volumes/RPI-RP2`, wrote `NEW.UF2`, and restarted the board. If automatic reset
fails, the user may need to hold BOOT, press/release RESET, then release BOOT
to enter the RP2040 bootloader. Do not target an unrelated bootloader device.

## Verification and remaining work

- Compilation succeeded: 69,832 bytes of flash and 9,568 bytes of static RAM,
  plus the framebuffer allocated at runtime.
- Upload succeeded: 174,080-byte UF2 written to the RP2040 bootloader.
- Board reappeared at `/dev/cu.usbmodem124301` after upload.
- **Physical screen appearance and button behavior have not been confirmed.**
  The agent cannot see the screen through the USB serial connection.
- No unresolved compiler errors remain. No automated tests were added.
- The next useful check is the user's observation of the display and controls.
  If the screen is blank or rotated incorrectly, inspect power/backlight,
  ST7789 initialization, and rotation before changing hardware assumptions.
- Preserve the user's changes; inspect `git status` before editing. Add clear
  commit messages and update notes when making further changes.
