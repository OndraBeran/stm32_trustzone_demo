#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "libtropic.h"
#include "libtropic_mbedtls_v4.h"
#include "libtropic_port_stm32u5xx.h"
#include "psa/crypto.h"

extern RNG_HandleTypeDef hrng;

int identify_chip(void) {
    printf("==============================================\n");
    printf("==== TROPIC01 Chip Identification Example ====\n");
    printf("==============================================\n");

    /* Cryptographic function provider initialization.

        In production, this would typically be done only once,
        usually at the start of the application or before
        the first use of cryptographic functions but no later than
        the first occurrence of any Libtropic function */
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        fprintf(stderr, "PSA Crypto initialization failed, status=%ld (psa_status_t)\n", status);
        return -1;
    }

    /* Libtropic handle.

        It is declared here (on stack) for
        simplicity. In production, you put it on heap if needed. */
    lt_handle_t lt_handle = {0};

    /* Device structure.

        Modify this according to your environment. Default values
        are compatible with RPi and our RPi shield.

        The device structure has to be zero initialized!
        STM32 HAL depends on zero init values. */
    lt_dev_stm32u5xx_t device = {0};

    device.spi_instance = SPI1;
    device.baudrate_prescaler = SPI_BAUDRATEPRESCALER_16;

    /* Enable clock of the GPIO bank where our custom chip select output is present. */
    // LT_SPI_CS_CLK_ENABLE(); /* Defined in main.h. */
    device.spi_cs_gpio_bank = GPIOC;
    device.spi_cs_gpio_pin = GPIO_PIN_9;

    /* IMPORTANT: Do not forget to initialize RNG peripheral
        at the beginning of your application using HAL_RNG_Init()! */
    device.rng_handle = &hrng;

#ifdef LT_USE_INT_PIN
    /* Enable clock of the GPIO bank where interrupt input is present. */
    LT_INT_CLK_ENABLE(); /* Defined in main.h. */
    device.int_gpio_bank = LT_INT_BANK;
    device.int_gpio_pin = LT_INT_PIN;
#endif

    lt_handle.l2.device = &device;

    /* Crypto abstraction layer (CAL) context. */
    lt_ctx_mbedtls_v4_t crypto_ctx;
    lt_handle.l3.crypto_ctx = &crypto_ctx;

    printf("Initializing handle...");
    lt_ret_t ret = lt_init(&lt_handle);
    if (LT_OK != ret) {
        fprintf(stderr, "\nFailed to initialize handle, ret=%s\n", lt_ret_verbose(ret));
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("OK\n");

    /* First, we check versions of both updateable firmwares. To do that, we need TROPIC01 to **not**
       be in the Start-up Mode. If there are valid firmwares, TROPIC01 will begin to execute them
       automatically on boot. */
    printf("Sending reboot request...");
    ret = lt_reboot(&lt_handle, TR01_REBOOT);
    if (ret != LT_OK) {
        fprintf(stderr, "\nlt_reboot() failed, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("OK\n");

    printf("Reading data from chip...\n");

    uint8_t fw_ver[4];
    ret = lt_get_info_riscv_fw_ver(&lt_handle, fw_ver);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to get RISC-V FW version, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("  RISC-V FW version: %02X.%02X.%02X (%02X)\n", fw_ver[3], fw_ver[2], fw_ver[1], fw_ver[0]);

    ret = lt_get_info_spect_fw_ver(&lt_handle, fw_ver);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to get SPECT FW version, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("  SPECT FW version: %02X.%02X.%02X (.%02X)\n", fw_ver[3],
           fw_ver[2], fw_ver[1], fw_ver[0]);

    /* We need to do the maintenance reboot to check bootloader version and FW bank headers in the
     * Startup Mode. */
    printf("Sending maintenance reboot request...");
    ret = lt_reboot(&lt_handle, TR01_MAINTENANCE_REBOOT);
    if (ret != LT_OK) {
        fprintf(stderr, "\nlt_reboot() failed, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("OK\n");

    printf("Reading data from chip...\n");

    /* When TROPIC01 is in Start-up Mode, we can get RISC-V bootloader version the same way as we got
     * RISC-V FW version.
     */
    ret = lt_get_info_riscv_fw_ver(&lt_handle, fw_ver);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to get RISC-V bootloader version, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("  RISC-V bootloader version: %02X.%02X.%02X (.%02X)\n",
           fw_ver[3] & 0x7f, fw_ver[2], fw_ver[1], fw_ver[0]);

    printf("Firmware bank headers:\n");
    ret = lt_print_fw_header(&lt_handle, TR01_FW_BANK_FW1, printf);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to print TR01_FW_BANK_FW1 header, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    ret = lt_print_fw_header(&lt_handle, TR01_FW_BANK_FW2, printf);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to print TR01_FW_BANK_FW2 header, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    ret = lt_print_fw_header(&lt_handle, TR01_FW_BANK_SPECT1, printf);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to print TR01_FW_BANK_SPECT1 header, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    ret = lt_print_fw_header(&lt_handle, TR01_FW_BANK_SPECT2, printf);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to print TR01_FW_BANK_SPECT2 header, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }

    struct lt_chip_id_t chip_id = {0};

    printf("Chip ID data:\n");
    ret = lt_get_info_chip_id(&lt_handle, &chip_id);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to get chip ID, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }

    printf("---------------------------------------------------------\n");
    ret = lt_print_chip_id(&chip_id, printf);
    if (ret != LT_OK) {
        fprintf(stderr, "Failed to print chip ID, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("---------------------------------------------------------\n");

    printf("Sending reboot request...");
    ret = lt_reboot(&lt_handle, TR01_REBOOT);
    if (ret != LT_OK) {
        fprintf(stderr, "\nlt_reboot() failed, ret=%s\n", lt_ret_verbose(ret));
        lt_deinit(&lt_handle);
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("OK!\n");

    printf("Deinitializing handle...");
    ret = lt_deinit(&lt_handle);
    if (LT_OK != ret) {
        fprintf(stderr, "\nFailed to deinitialize handle, ret=%s\n", lt_ret_verbose(ret));
        mbedtls_psa_crypto_free();
        return -1;
    }
    printf("OK\n");

    /* Cryptographic function provider deinitialization.

        In production, this would be done only once, typically
        during termination of the application. */
    mbedtls_psa_crypto_free();

    return 0;
}