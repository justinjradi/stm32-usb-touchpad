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
	uint8_t contact_ID:4;
	uint8_t padding:2;
	uint16_t x;
	uint16_t y;
} Contact;	// 5 bytes

typedef struct __attribute__((packed))
{
	uint8_t report_ID;
	Contact reported_contacts[TP_MAX_CONTACTS];
	uint16_t scan_time;
	uint8_t contact_count;
	uint8_t left_click : 1;
	uint8_t middle_click : 1;
	uint8_t right_click : 1;
	uint8_t padding : 5;
} TouchpadReport;		// 5 + 5*5 = 30 bytes

typedef struct __attribute__((packed))
{
	uint8_t report_ID;
	uint8_t left_click : 1;
	uint8_t right_click : 1;
	uint8_t padding : 6;
	uint16_t x;
	uint16_t y;
} MouseReport;	// 4 bytes

Contact contacts_by_ID[TP_MAX_CONTACTS];
uint8_t button_states[3];
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
void tp_clear(void)
{
	for (int id = 0; id < TP_MAX_CONTACTS; id++)
	{
		memset(&contacts_by_ID[id], 0, sizeof (Contact));
	}
}

void tp_set_contact(uint8_t contact_ID, uint16_t x, uint16_t y)
{
	contacts_by_ID[contact_ID].confidence = 1;
	contacts_by_ID[contact_ID].tip_switch = 1;
	contacts_by_ID[contact_ID].contact_ID = contact_ID;
	contacts_by_ID[contact_ID].x = x;
	contacts_by_ID[contact_ID].y = y;
}

void tp_reset_contact(uint8_t contact_ID)
{
	contacts_by_ID[contact_ID].tip_switch = 0;
}

void tp_set_reset_buttons(uint8_t left_click, uint8_t middle_click, uint8_t right_click)
{
	button_states[0] = left_click;
	button_states[1] = middle_click;
	button_states[2] = right_click;
}

void tp_update(int scan_time_ms)
{
	TouchpadConfiguration tc = get_touchpad_configuration();
	if (tc.input_mode == TP_INPUT_MODE_TOUCHPAD)
	{
		tp_send_touchpad_report(scan_time_ms);
	}
	else	// input mode = mouse
	{
//		for (int id = 0; id < TP_MAX_CONTACTS; id++)
//		{
//			if (contacts_by_ID[id].tip_switch)	// Send report for first set contact
//			{
//				// TODO CONVERT UNITS
//				uint16_t x = contacts_by_ID[id].x;
//				uint16_t x = contacts_by_ID[id].y;
//				uint8_t left_click  = button_states[0];
//				uint8_t right_click = button_states[2];
//				tp_send_mouse_report(x, y, left_click, right_click);
//				return;
//			}
//		}
		tp_send_mouse_report(0, 0, 0, 0);	// No contact is set, so send empty report
	}
}

// Direct usage discouraged!
void tp_send_touchpad_report(uint16_t scan_time)	// in 100μs units
{
	TouchpadReport report;
	report.report_ID = REPORTID_TOUCHPAD;
	report.scan_time = scan_time;
	int count = 0;
	TouchpadConfiguration tc = get_touchpad_configuration();
	if (tc.surface_switch)	// Contacts are to be reported
	{
		for (int id = 0; id < TP_MAX_CONTACTS; id++)
		{
			if (contacts_by_ID[id].tip_switch)
			{
				report.reported_contacts[count] = contacts_by_ID[id];
				count++;
			}
		}
		for (int j = count; j < TP_MAX_CONTACTS; j++)
		{
			memset(&report.reported_contacts[j], 0, sizeof (Contact));
		}
	}
	report.contact_count = count;
	if (tc.button_switch)
	{
		report.left_click = button_states[0];
		report.middle_click = button_states[1];
		report.right_click = button_states[2];
	}
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &report, sizeof (report));
}

// Direct usage discouraged!
void tp_send_mouse_report(uint16_t x, uint16_t y, uint8_t left_click, uint8_t right_click)
{
	MouseReport report;
	report.x = x;
	report.y = y;
	report.left_click = left_click;
	report.right_click = right_click;
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &report, sizeof (report));
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

