/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

static UX_SLAVE_CLASS_CDC_ACM *cdc_acm = UX_NULL;

static UCHAR usb_test_message[] =
    "STM32 USB CDC works\r\n";

static ULONG usb_tx_actual_length = 0U;
static ULONG usb_last_tx_tick = 0U;
static UINT usb_tx_active = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_CDC_ACM_Activate
  *         This function is called when insertion of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Activate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Activate */

  cdc_acm = (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance;

  usb_tx_active = 0U;
  usb_tx_actual_length = 0U;
  usb_last_tx_tick = HAL_GetTick();
  /* USER CODE END USBD_CDC_ACM_Activate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_Deactivate
  *         This function is called when extraction of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Deactivate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Deactivate */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);

  cdc_acm = UX_NULL;
  usb_tx_active = 0U;
  /* USER CODE END USBD_CDC_ACM_Deactivate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_ParameterChange
  *         This function is invoked to manage the CDC ACM class requests.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_ParameterChange(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_ParameterChange */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);
  /* USER CODE END USBD_CDC_ACM_ParameterChange */

  return;
}

/* USER CODE BEGIN 1 */

VOID USB_CDC_Process(VOID)
{
  UINT status;

  /* CDC device is not connected/configured yet */
  if (cdc_acm == UX_NULL)
  {
    usb_tx_active = 0U;
    return;
  }

  /* Start a new message once per second */
  if (usb_tx_active == 0U)
  {
    if ((HAL_GetTick() - usb_last_tx_tick) < 1000U)
    {
      return;
    }

    usb_tx_actual_length = 0U;
    usb_tx_active = 1U;
  }

  /*
   * Standalone USBX transmission.
   *
   * This function must be called repeatedly until the transfer
   * reaches UX_STATE_NEXT.
   */
  status = ux_device_class_cdc_acm_write_run(
      cdc_acm,
      usb_test_message,
      sizeof(usb_test_message) - 1U,
      &usb_tx_actual_length);

  if (status == UX_STATE_NEXT)
  {
    /* Transmission complete */
    usb_tx_active = 0U;
    usb_last_tx_tick = HAL_GetTick();
  }
  else if ((status == UX_STATE_EXIT) ||
           (status == UX_STATE_ERROR))
  {
    /* Transfer failed or device disappeared */
    usb_tx_active = 0U;
    usb_last_tx_tick = HAL_GetTick();
  }
}

/* USER CODE END 1 */
