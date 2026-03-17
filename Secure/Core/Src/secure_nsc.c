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
#include "identify_chip.h"
#include <stdio.h>
#include <string.h>
#include <arm_cmse.h>  // for cmse_nsfptr_create, cmse_is_nsfptr
#include <sys/_intsup.h>
/** @addtogroup STM32U5xx_HAL_Examples

  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Global variables ----------------------------------------------------------*/
void *pSecureFaultCallback = NULL;   /* Pointer to secure fault callback in Non-secure */
void *pSecureErrorCallback = NULL;   /* Pointer to secure error callback in Non-secure */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Secure registration of non-secure callback.
  * @param  CallbackId  callback identifier
  * @param  func        pointer to non-secure function
  * @retval None
  */
CMSE_NS_ENTRY void SECURE_RegisterCallback(SECURE_CallbackIDTypeDef CallbackId, void *func)
{
  if(func != NULL)
  {
    switch(CallbackId)
    {
      case SECURE_FAULT_CB_ID:           /* SecureFault Interrupt occurred */
      pSecureFaultCallback = func;
      break;
      case GTZC_ERROR_CB_ID:             /* GTZC Interrupt occurred */
      pSecureErrorCallback = func;
      break;
      default:
      /* unknown */
      break;
    }
  }
}

/**
  * @}
  */
CMSE_NS_ENTRY int authenticate(void *callback)
{
  char * expected_pin = "1234";
  char input_pin[5] = {0};  // 4 digits + null terminator

  funcptr_NS ns_cb = NULL;

  if (callback != NULL) {
    // Force LSB=1 (non-secure state) and cast to NS-callable type
    ns_cb = (funcptr_NS)cmse_nsfptr_create(callback);

    // Optional but recommended runtime validation
    if (!cmse_is_nsfptr(ns_cb)) {
      return -1;
    }
  }

  for (int i = 0; i < 4; ++i) {
    char c = KeyPad_WaitForKeyGetChar(0);
    printf("Key Pressed: %c\n", c);
    input_pin[i] = c;

    if (ns_cb != NULL) {
      ns_cb();  // Non-secure callback invocation
    }
  }

  // mock tropic call
  identify_chip();

  if (strcmp(input_pin, expected_pin) == 0) {
    return 0;  // Authentication successful
  } else {
    return -1;  // Authentication failed
  }


  return 0;
}

/**
  * @}
  */
/* USER CODE END Non_Secure_CallLib */

