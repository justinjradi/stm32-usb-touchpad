/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
typedef struct __attribute__((packed))
{
	uint8_t confidence:1;
	uint8_t tip_switch:1;
	uint8_t contact_ID:2;
	uint8_t padding:4;
	uint16_t x;
	uint16_t y;
} Contact;	// 5 bytes

typedef struct __attribute__((packed))
{
	uint8_t report_ID;
	Contact reported_contacts[TP_MAX_CONTACTS];
	uint16_t scan_time;
	uint8_t contact_count;
	uint8_t button_1 : 1;
	uint8_t button_2 : 1;
	uint8_t button_3 : 1;
	uint8_t padding : 5;
} TouchpadReport;		// 10 bytes

typedef struct __attribute__((packed))
{
	uint8_t report_ID;
	uint8_t button_1 : 1;
	uint8_t button_2 : 1;
	uint8_t padding : 6;
	uint16_t x;
	uint16_t y;
} MouseReport;	// 4 bytes

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
void input_test(void)
{
	TouchpadReport touchpadReport = {0};
	touchpadReport.report_ID = REPORTID_TOUCHPAD;
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &touchpadReport, sizeof (touchpadReport));
	printf("");
}

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CUSTOM_HID) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &USBD_CustomHID_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

