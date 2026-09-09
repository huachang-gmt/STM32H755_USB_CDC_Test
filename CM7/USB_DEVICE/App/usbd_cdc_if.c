/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

#define USB_PKT_MAGIC0              0x47U
#define USB_PKT_MAGIC1              0x4DU
#define USB_PKT_VERSION             0x01U

#define USB_PKT_TYPE_COMMAND        0x01U
#define USB_PKT_TYPE_RESPONSE       0x02U
#define USB_PKT_TYPE_ACK            0x03U
#define USB_PKT_TYPE_RETRANSMIT_REQUEST 0x04U

#define USB_PKT_HEADER_SIZE         8U
#define USB_PKT_CRC_SIZE            2U
#define USB_PKT_MAX_PAYLOAD         4096U
#define USB_PKT_MAX_SIZE            \
    (USB_PKT_HEADER_SIZE + USB_PKT_MAX_PAYLOAD + USB_PKT_CRC_SIZE)

/*已經開始收到一個封包，但 100 ms 內仍沒有收完整，就視為接收 timeout*/   
#define USB_PKT_RX_TIMEOUT_MS          50U
/* USER CODE END PRIVATE_DEFINES */


/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

static uint8_t packet_rx_buffer[USB_PKT_MAX_SIZE];
static uint32_t packet_rx_length = 0U;
static uint32_t packet_rx_start_tick = 0U;
static uint16_t packet_rx_sequence = 0U;
static uint8_t packet_rx_waiting_retransmit = 0U;
static uint32_t packet_rx_retransmit_tick = 0U;
/* USER CODE END PV */


/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
static void USB_Packet_ProcessRx(void);

static void USB_Packet_SendResponse(
    uint16_t sequence,
    uint16_t payload_length);
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */

  printf("[USB RX] Len=%lu\r\n", *Len);

  printf("[USB RX] Data HEX:");

  for (uint32_t i = 0U; i < *Len; i++)
  {
    printf(" %02X", Buf[i]);
  }

  printf("\r\n");

  /*
   * USB CDC is a byte stream.
   *
   * One callback does NOT necessarily equal one packet.
   *
   * Append the received bytes into the packet accumulator.
   */
  if ((*Len > 0U) &&
      ((packet_rx_length + *Len) <= USB_PKT_MAX_SIZE))
  {
    if (packet_rx_length == 0U)
    {
        packet_rx_start_tick = HAL_GetTick();
    }

    memcpy(
        &packet_rx_buffer[packet_rx_length],
        Buf,
        *Len
    );

    packet_rx_length += *Len;


    printf(
        "[USB PKT DEBUG] Before ProcessRx: length=%lu waiting=%u seq=%u\r\n",
        packet_rx_length,
        packet_rx_waiting_retransmit,
        packet_rx_sequence
    );


    USB_Packet_ProcessRx();

    if (packet_rx_length == 0U)
    {
        packet_rx_start_tick = 0U;
    }

  }
  else
  {
    printf(
        "[USB PKT] RX accumulator overflow\r\n"
    );

    /*
     * For now reset the accumulator.
     *
     * Later this condition will be handled by the
     * transport reliability/error mechanism.
     */
    packet_rx_length = 0U;
  }

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);

  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

static uint16_t USB_Packet_CalculateCRC16(
    const uint8_t *data,
    uint32_t length)
{
    const uint16_t polynomial = 0xA001U;
    uint16_t crc = 0xFFFFU;

    for (uint32_t i = 0U; i < length; i++)
    {
        crc ^= data[i];

        for (uint32_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ polynomial);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

static uint16_t USB_Packet_ReadUint16LE(
    const uint8_t *data)
{
    return (uint16_t)(
        (uint16_t)data[0] |
        ((uint16_t)data[1] << 8U)
    );
}

static void USB_Packet_SendResponse(
    uint16_t sequence,
    uint16_t payload_length)
{
    static uint8_t packet[128];
    uint32_t packet_length = 0U;

    char response_payload[64];

    const int response_length = snprintf(
        response_payload,
        sizeof(response_payload),
        "RX OK SEQ=%u LEN=%u\r\n",
        sequence,
        payload_length
    );

    if (response_length <= 0)
    {
        return;
    }

    if ((uint32_t)response_length > 64U)
    {
        return;
    }

    /*
     * ------------------------------------------------------------
     * Transport Header
     *
     * Byte 0~1 : Magic
     * Byte 2   : Version
     * Byte 3   : RESPONSE
     * Byte 4~5 : Sequence
     * Byte 6~7 : Payload Length
     * ------------------------------------------------------------
     */

    packet[0] = USB_PKT_MAGIC0;
    packet[1] = USB_PKT_MAGIC1;
    packet[2] = USB_PKT_VERSION;
    packet[3] = USB_PKT_TYPE_RESPONSE;

    packet[4] = (uint8_t)(sequence & 0x00FFU);
    packet[5] = (uint8_t)((sequence >> 8U) & 0x00FFU);

    packet[6] = (uint8_t)(
        (uint16_t)response_length & 0x00FFU
    );

    packet[7] = (uint8_t)(
        ((uint16_t)response_length >> 8U) & 0x00FFU
    );

    /*
     * Payload
     */
    memcpy(
        &packet[USB_PKT_HEADER_SIZE],
        response_payload,
        (uint32_t)response_length
    );

    packet_length =
        USB_PKT_HEADER_SIZE +
        (uint32_t)response_length;

    /*
     * CRC covers Header + Payload.
     */
    const uint16_t crc =
        USB_Packet_CalculateCRC16(
            packet,
            packet_length
        );

    packet[packet_length++] =
        (uint8_t)(crc & 0x00FFU);

    packet[packet_length++] =
        (uint8_t)((crc >> 8U) & 0x00FFU);

    printf(
        "[USB PKT TX] RESPONSE seq=%u payload=%u total=%lu\r\n",
        sequence,
        (uint16_t)response_length,
        packet_length
    );

    printf("[USB PKT TX] HEX:");

    for (uint32_t i = 0U; i < packet_length; i++)
    {
        printf(
            " %02X",
            packet[i]
        );
    }

    printf("\r\n");

    const uint8_t result =
        CDC_Transmit_FS(
            packet,
            (uint16_t)packet_length
        );

    printf(
        "[USB PKT TX] CDC_Transmit_FS result=%u\r\n",
        result
    );
}

//「明天 Stage 8 要重新檢查／重寫」
static void USB_Packet_SendRetransmitRequest(uint16_t sequence)
{
    static uint8_t packet[12];
    uint32_t packet_length = 0U;

    /*
     * RETRANSMIT_REQUEST Packet
     *
     * Byte 0~1 : Magic
     * Byte 2   : Version
     * Byte 3   : RETRANSMIT_REQUEST
     * Byte 4~5 : Sequence
     * Byte 6~7 : Payload Length = 2
     * Byte 8~9 : Requested Sequence
     * Byte 10~11 : CRC16
     */

    packet[0] = USB_PKT_MAGIC0;
    packet[1] = USB_PKT_MAGIC1;
    packet[2] = USB_PKT_VERSION;
    packet[3] = USB_PKT_TYPE_RETRANSMIT_REQUEST;

    /* Transport Header Sequence */
    packet[4] = (uint8_t)(sequence & 0x00FFU);
    packet[5] = (uint8_t)((sequence >> 8U) & 0x00FFU);

    /* Payload Length = 2 */
    packet[6] = 0x02U;
    packet[7] = 0x00U;

    /* Payload = requested sequence */
    packet[8] = (uint8_t)(sequence & 0x00FFU);
    packet[9] = (uint8_t)((sequence >> 8U) & 0x00FFU);

    packet_length = 10U;

    const uint16_t crc =
        USB_Packet_CalculateCRC16(
            packet,
            packet_length
        );

    packet[packet_length++] =
        (uint8_t)(crc & 0x00FFU);

    packet[packet_length++] =
        (uint8_t)((crc >> 8U) & 0x00FFU);

    printf(
        "[USB PKT TX] RETRANSMIT_REQUEST seq=%u total=%lu\r\n",
        sequence,
        packet_length
    );

    printf("[USB PKT TX] HEX:");

    for (uint32_t i = 0U; i < packet_length; i++)
    {
        printf(
            " %02X",
            packet[i]
        );
    }

    printf("\r\n");

    const uint8_t result =
        CDC_Transmit_FS(
            packet,
            (uint16_t)packet_length
        );

    printf(
        "[USB PKT TX] CDC_Transmit_FS result=%u\r\n",
        result
    );
}

static void USB_Packet_ProcessRx(void)
{
    while (true)
    {
        /*
         * ------------------------------------------------------------
         * 1. Need at least 2 bytes to search for packet magic.
         * ------------------------------------------------------------
         */
        if (packet_rx_length < 2U)
        {
            return;
        }

        /*
         * ------------------------------------------------------------
         * 2. Synchronize to MAGIC = 0x47 0x4D.
         *
         * Any bytes before MAGIC are discarded.
         * ------------------------------------------------------------
         */
        uint32_t magic_index = 0U;

        while (magic_index + 1U < packet_rx_length)
        {
            if ((packet_rx_buffer[magic_index] == USB_PKT_MAGIC0) &&
                (packet_rx_buffer[magic_index + 1U] == USB_PKT_MAGIC1))
            {
                break;
            }

            magic_index++;
        }

        /*
         * No complete MAGIC pair found yet.
         *
         * Keep the last byte because it could be 0x47
         * and the next USB callback may contain 0x4D.
         */
        if (magic_index + 1U >= packet_rx_length)
        {
            if (packet_rx_buffer[packet_rx_length - 1U] ==
                USB_PKT_MAGIC0)
            {
                packet_rx_buffer[0] =
                    USB_PKT_MAGIC0;

                packet_rx_length = 1U;
            }
            else
            {
                packet_rx_length = 0U;
            }

            return;
        }

        /*
         * Discard bytes before MAGIC.
         */
        if (magic_index > 0U)
        {
            memmove(
                packet_rx_buffer,
                &packet_rx_buffer[magic_index],
                packet_rx_length - magic_index
            );

            packet_rx_length -= magic_index;
        }

        /*
         * ------------------------------------------------------------
         * 3. Need complete 8-byte header.
         *
         * IMPORTANT:
         * Receiving only part of the header is NOT an error.
         * We simply wait for the next CDC callback.
         * ------------------------------------------------------------
         */
        if (packet_rx_length < USB_PKT_HEADER_SIZE)
        {
            printf(
                "[USB PKT] Fragment: header %lu/%u bytes\r\n",
                packet_rx_length,
                USB_PKT_HEADER_SIZE
            );

            return;
        }

        /*
         * ------------------------------------------------------------
         * 4. Check protocol version.
         * ------------------------------------------------------------
         */
        if (packet_rx_buffer[2] != USB_PKT_VERSION)
        {
            printf(
                "[USB PKT] Invalid version: 0x%02X\r\n",
                packet_rx_buffer[2]
            );

            /*
             * Discard current MAGIC and try to resynchronize.
             */
            memmove(
                packet_rx_buffer,
                &packet_rx_buffer[1],
                packet_rx_length - 1U
            );

            packet_rx_length--;

            continue;
        }

        /*
         * ------------------------------------------------------------
         * 5. Check packet type.
         * ------------------------------------------------------------
         */
        const uint8_t packet_type =
            packet_rx_buffer[3];

        if ((packet_type != USB_PKT_TYPE_COMMAND) &&
            (packet_type != USB_PKT_TYPE_RESPONSE) &&
            (packet_type != USB_PKT_TYPE_ACK) &&
            (packet_type != USB_PKT_TYPE_RETRANSMIT_REQUEST))
        {
            printf(
                "[USB PKT] Invalid type: 0x%02X\r\n",
                packet_type
            );

            /*
             * Discard current MAGIC and resynchronize.
             */
            memmove(
                packet_rx_buffer,
                &packet_rx_buffer[1],
                packet_rx_length - 1U
            );

            packet_rx_length--;

            continue;
        }

        /*
         * ------------------------------------------------------------
         * 6. Read Sequence and Payload Length.
         * ------------------------------------------------------------
         */
        const uint16_t sequence =
            USB_Packet_ReadUint16LE(
                &packet_rx_buffer[4]
            );

        packet_rx_sequence = sequence;


        const uint16_t payload_length =
            USB_Packet_ReadUint16LE(
                &packet_rx_buffer[6]
            );

        /*
         * ------------------------------------------------------------
         * 7. Validate payload length.
         * ------------------------------------------------------------
         */
        if (payload_length > USB_PKT_MAX_PAYLOAD)
        {
            printf(
                "[USB PKT] Invalid payload length: %u\r\n",
                payload_length
            );

            /*
             * Discard current MAGIC and resynchronize.
             */
            memmove(
                packet_rx_buffer,
                &packet_rx_buffer[1],
                packet_rx_length - 1U
            );

            packet_rx_length--;

            continue;
        }

        /*
         * ------------------------------------------------------------
         * 8. Calculate complete packet size.
         * ------------------------------------------------------------
         */
        const uint32_t expected_packet_size =
            USB_PKT_HEADER_SIZE +
            (uint32_t)payload_length +
            USB_PKT_CRC_SIZE;

        /*
         * ------------------------------------------------------------
         * 9. Packet is not complete yet.
         *
         * THIS IS NORMAL.
         *
         * The packet may have been split across multiple
         * CDC callbacks.
         * ------------------------------------------------------------
         */
        if (packet_rx_length < expected_packet_size)
        {
            printf(
                "[USB PKT] Fragment: %lu/%lu bytes, waiting...\r\n",
                packet_rx_length,
                expected_packet_size
            );

            return;
        }

        /*
         * ------------------------------------------------------------
         * 10. Full packet received.
         *     Verify CRC.
         * ------------------------------------------------------------
         */
        const uint16_t received_crc =
            USB_Packet_ReadUint16LE(
                &packet_rx_buffer[
                    USB_PKT_HEADER_SIZE + payload_length
                ]
            );

        const uint16_t calculated_crc =
            USB_Packet_CalculateCRC16(
                packet_rx_buffer,
                USB_PKT_HEADER_SIZE + payload_length
            );

        printf(
            "[USB PKT] Full packet: type=0x%02X seq=%u payload=%u\r\n",
            packet_type,
            sequence,
            payload_length
        );

        printf(
            "[USB PKT] CRC RX=0x%04X CALC=0x%04X\r\n",
            received_crc,
            calculated_crc
        );

        if (received_crc != calculated_crc)
        {
            printf(
                "[USB PKT] CRC ERROR\r\n"
            );

            /*
             * For now:
             * Do NOT implement retransmission yet.
             *
             * That will be the next reliability step.
             */

            memmove(
                packet_rx_buffer,
                &packet_rx_buffer[expected_packet_size],
                packet_rx_length - expected_packet_size
            );

            packet_rx_length -= expected_packet_size;

            continue;
        }

        /*
         * ------------------------------------------------------------
         * 11. CRC OK.
         * ------------------------------------------------------------
         */
        printf(
            "[USB PKT] CRC OK\r\n"
        );

        if (packet_rx_waiting_retransmit != 0U)
        {
            printf(
                "[USB PKT] Retransmitted packet received, leaving WAIT_RETRANSMIT\r\n"
            );

            packet_rx_waiting_retransmit = 0U;
            packet_rx_retransmit_tick = 0U;
        }

        if (packet_type == USB_PKT_TYPE_COMMAND)
        {
            printf(
                "[USB PKT] COMMAND received, seq=%u\r\n",
                sequence
            );

            printf(
                "[USB PKT] Payload HEX:"
            );

            for (uint16_t i = 0U; i < payload_length; i++)
            {
                printf(
                    " %02X",
                    packet_rx_buffer[
                        USB_PKT_HEADER_SIZE + i
                    ]
                );
            }

            printf("\r\n");

            USB_Packet_SendResponse(
                sequence,
                payload_length
            );

        }

        /*
         * ------------------------------------------------------------
         * 12. Remove processed packet.
         *
         * If another complete packet already exists in the
         * accumulator, the while-loop processes it immediately.
         * ------------------------------------------------------------
         */
        memmove(
            packet_rx_buffer,
            &packet_rx_buffer[expected_packet_size],
            packet_rx_length - expected_packet_size
        );

        packet_rx_length -= expected_packet_size;
    }
}

void USB_Packet_CheckRxTimeout(void)
{
    const uint32_t now = HAL_GetTick();

    /* STEP 3:
     * Waiting for the one allowed retransmission.
     */
    if (packet_rx_waiting_retransmit != 0U)
    {
        if ((now - packet_rx_retransmit_tick) >= USB_PKT_RX_TIMEOUT_MS)
        {
            printf(
                "[USB PKT] RETRANSMIT TIMEOUT - no retransmitted packet\r\n"
            );

            printf(
                "[USB PKT DEBUG] Before reset: length=%lu waiting=%u seq=%u\r\n",
                packet_rx_length,
                packet_rx_waiting_retransmit,
                packet_rx_sequence
            );

            packet_rx_length = 0U;
            packet_rx_start_tick = 0U;
            packet_rx_sequence = 0U;
            packet_rx_waiting_retransmit = 0U;
            packet_rx_retransmit_tick = 0U;

            printf(
                "[USB PKT DEBUG] After reset: length=%lu waiting=%u seq=%u\r\n",
                packet_rx_length,
                packet_rx_waiting_retransmit,
                packet_rx_sequence
            );
        }

        return;
    }

    /* Normal packet reception timeout */
    if (packet_rx_length == 0U)
        return;

    if ((now - packet_rx_start_tick) < USB_PKT_RX_TIMEOUT_MS)
        return;

    printf(
        "[USB PKT] ERROR: RX TIMEOUT - incomplete packet, received=%lu bytes\r\n",
        packet_rx_length
    );

    const uint16_t sequence = packet_rx_sequence;

    printf(
        "[USB PKT] Request retransmission seq=%u\r\n",
        sequence
    );

    packet_rx_length = 0U;
    packet_rx_start_tick = 0U;

    packet_rx_waiting_retransmit = 1U;
    packet_rx_retransmit_tick = now;

    USB_Packet_SendRetransmitRequest(sequence);

    printf(
        "[USB PKT] State -> WAIT_RETRANSMIT seq=%u\r\n",
        sequence
    );
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
