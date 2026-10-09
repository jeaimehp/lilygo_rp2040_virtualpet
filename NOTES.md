# Project notes

## October 9, 2026 — Physical device photo

- Added the user's photo to `assets/photos/acorn-on-device.jpeg` and the README.
- The photo confirms that the grooming scene and care meters display on the
  physical LCD. Button behavior has not been verified by the photo.

## October 9, 2026 — GitHub repository

- Created private repository https://github.com/jeaimehp/lilygo_rp2040_virtualpet.
- Pushed local commits on `main` and set tracking to `origin/main`.

## October 9, 2026 — README media

- Added six native 240 × 135 PNG screenshots and six looping GIFs to `assets/`.
- Rendered the actual sketch with the vendored Adafruit GFX implementation;
  hardware calls are stubbed on the desktop, with no replacement drawing code.
- Verified PNG and decoded GIF pixels against the rendered framebuffer colors.
- Visually reviewed all six scenes together; firmware was unchanged.
- Media depicts firmware output, not a physical LCD capture.
- `tools/render_assets.py` regenerates all media; requires clang++ and Pillow.

## October 9, 2026 — More activity views

- Added front-facing eating and grooming poses, a curled sleeping pose in a
  nighttime nest, and a digging pose with animated dirt.
- Added a picnic blanket, trees, moon and stars, and a moving ball-chase scene.
- Idle views rotate every eight seconds; button activities last five seconds.
- Compilation succeeded: 71,512 bytes of flash, 9,568 bytes of static RAM.
- Flash succeeded: 177,152-byte UF2 uploaded; the Pico returned on port 124301.
- Physical appearance of the new scenes still needs user verification.
- USB name now reports `Pico`, with the same serial DE616C825776382A.

## October 9, 2026 — Initial squirrel pet

- Hardware: LILYGO T-PicoC3, RP2040 side; USB serial DE616C825776382A.
- Display: ST7789V, 135 × 240 pixels, used in landscape orientation.
- Acorn blinks, swishes its tail, eats, plays, and sleeps.
- GPIO 6 feeds; GPIO 7 plays; both buttons toggle sleep.
- Food, joy, and energy change over time; state resets after a restart.
- Display pin mapping follows LILYGO's official T-PicoC3 documentation.

## October 9, 2026 — Build and flash

- Installed Arduino CLI 1.5.1 and Arduino-Pico core 6.3.0.
- Existing display library had numbered duplicate source files that caused
  linker errors. Kept a local dependency copy excluding those duplicates.
- Build succeeded: 69,832 bytes of program storage and 9,568 bytes of static
  RAM, plus the 64,800-byte framebuffer allocated at runtime.
- Uploaded 174,080 bytes of UF2 firmware through the RP2040 bootloader.
- Upload completed successfully; the board returned at
  `/dev/cu.usbmodem124301`.
- Screen appearance and button behavior still require physical verification.
- Build products are excluded from Git; source, dependencies, and setup notes
  are retained. Third-party dependencies retain their supplied licenses.
