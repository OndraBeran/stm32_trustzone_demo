/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Secure/Src/secure_nsc.c
  * @author  MCD Application Team
  * @brief   This file contains the non-secure callable APIs (secure world)
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN Non_Secure_CallLib */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "secure_nsc.h"
#include "KeyPad.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arm_cmse.h>  // for cmse_nsfptr_create, cmse_is_nsfptr
#include <sys/_intsup.h>
#include "libtropic_common.h"
#include "pin_verification.h"
/** @addtogroup STM32U5xx_HAL_Examples

  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Global variables ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void protected_function(void) {
  printf("This is a protected function that can only be called from the secure world.\n");
}

CMSE_NS_ENTRY uint8_t remaining_tries_nsc() {
  return remaining_tries();
}

CMSE_NS_ENTRY int authenticate(void *callback)
{
  uint8_t input_pin[4] = {0};

  funcptr_NS ns_callback = NULL;

  // If a callback function is provided, create a non-secure callable function pointer
  if (callback != NULL) {
    ns_callback = (funcptr_NS)cmse_nsfptr_create(callback);
  }

  for (int i = 0; i < 4; ++i) {
    char c = KeyPad_WaitForKeyGetChar(0);
    printf("Key Pressed: %c\n", c);
    input_pin[i] = c - '0';  // Convert char digit to integer

    // Call the non-secure callback to print an asterisk for each entered digit
    if (ns_callback != NULL) {
      ns_callback();
    }
  }

  int check_result = check_pin(input_pin, sizeof(input_pin));

  if (check_result == 0) {
    protected_function();  // Call a secure function on successful authentication
  }

  return check_result;
}

/**
  * @}
  */
/* USER CODE END Non_Secure_CallLib */

