#include "boards.h"
#include "board.h"
#include "tusb.h"
#include "uf2/configkeys.h"

__attribute__((used, section(".bootloaderConfig")))
const uint32_t bootloaderConfig[] =
{
  /* CF2 START */
  CFG_MAGIC0, CFG_MAGIC1,                       // magic
  5, 100,                                       // used entries, total entries

  204, 0x100000,                                // FLASH_BYTES = 0x100000
  205, 0x40000,                                 // RAM_BYTES = 0x40000
  208, (USB_DESC_VID << 16) | USB_DESC_UF2_PID, // BOOTLOADER_BOARD_ID = USB VID+PID, used for verification when updating bootloader via uf2
  209, 0xada52840,                              // UF2_FAMILY = 0xada52840
  210, 0x20,                                    // PINS_PORT_SIZE = PA_32

  0, 0, 0, 0, 0, 0, 0, 0
  /* CF2 END */
};

static const uint8_t nrf_pins[] = {
  [ 0] = _PINNUM(0, 8),
  [ 1] = _PINNUM(0, 6),
  [ 2] = _PINNUM(0, 17),
  [ 3] = _PINNUM(0, 20),
  [ 4] = _PINNUM(0, 22),
  [ 5] = _PINNUM(0, 24),
  [ 6] = _PINNUM(1, 0),
  [ 7] = _PINNUM(0, 11),
  [ 8] = _PINNUM(1, 4),
  [ 9] = _PINNUM(1, 6),
  [10] = _PINNUM(0, 9),
  [16] = _PINNUM(0, 10),
  [14] = _PINNUM(1, 11),
  [15] = _PINNUM(1, 13),
  [18] = _PINNUM(1, 15), [19] = _PINNUM(0, 2),
  [20] = _PINNUM(0, 29),
  [21] = _PINNUM(0, 31),
};

static const uint8_t rows[] = { 19, 8, 9, 21, 7, 20 };
#if defined(LEFT_HALF)
static const uint8_t cols[] = { 1, 0, 2, 4, 5, 6 };
#elif defined(RIGHT_HALF)
static const uint8_t cols[] = { 6, 5, 4, 2, 0, 1 };
#else
#error "Define LEFT_HALF or RIGHT_HALF"
#endif

void board_init2(void)
{
  for (uint8_t col = 0; col < sizeof(cols); ++col) {
    nrf_gpio_cfg_output(nrf_pins[cols[col]]);
    nrf_gpio_pin_write(nrf_pins[cols[col]], 0);
  }
  for (uint8_t row = 0; row < sizeof(rows); ++row) {
    nrf_gpio_cfg_input(nrf_pins[rows[row]], NRF_GPIO_PIN_PULLDOWN);
  }
}

/*
        map = <
          RC(0,0) RC(0,1) RC(0,2) RC(0,3) RC(0,4) RC(0,5)
          RC(1,0) RC(1,1) RC(1,2) RC(1,3) RC(1,4) RC(1,5)
          RC(2,0) RC(2,1) RC(2,2) RC(2,3) RC(2,4) RC(2,5)
          RC(3,0) RC(3,1) RC(3,2) RC(3,3) RC(3,4) RC(3,5)
          RC(4,0) RC(4,1)                 RC(4,4) RC(4,5)
          // DPAD
          // Left Down    Middle   Up      Right
          RC(5,5) RC(5,3) RC(5,2)  RC(5,1) RC(5,4)
        >;
*/

bool exit_bootloader(void)
{
  static uint8_t matrix[sizeof(rows)][sizeof(cols)];
  for (uint8_t col = 0; col < sizeof(cols); ++col) {
    nrf_gpio_pin_write(nrf_pins[cols[col]], 1);
    for (uint8_t row = 0; row < sizeof(rows); ++row) {
      int pin = nrf_gpio_pin_read(nrf_pins[rows[row]]) ? 1 : -1;
      int new = matrix[row][col] + pin;
      if (new < 0x00) new = 0x00;
      if (new > 0xFF) new = 0xFF;
      matrix[row][col] = new;
    }
    nrf_gpio_pin_write(nrf_pins[cols[col]], 0);
  }

  bool magic =
    (matrix[2][1] == 0xFF) && (matrix[2][2] == 0) &&
    (matrix[2][3] == 0) && (matrix[2][4] == 0xFF);

  return magic;
}
