/*
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	======== USBCDCD.c ========
 */

/* driverlib Header files */
#include <stdint.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

/* usblib Header files */
#include <usblib/usb-ids.h>
#include <usblib/usblib.h>
#include <usblib/usbcdc.h>
#include <usblib/device/usbdevice.h>
#include <usblib/device/usbdcdc.h>

/* Example/Board Header files */
#include "USBCDCD.h"

/* Defines */
#define USBBUFFERSIZE   256

#ifndef NULL
#define NULL                    ((void *)0)
#endif

#ifndef USB_VID_TI
#define USB_VID_TI      USB_VID_STELLARIS
#endif

/* Typedefs */
typedef volatile enum {
    USBCDCD_STATE_IDLE = 0,
    USBCDCD_STATE_INIT,
    USBCDCD_STATE_UNCONFIGURED
} USBCDCD_USBState;

/* Static variables and handles */
static volatile USBCDCD_USBState state = USBCDCD_STATE_UNCONFIGURED;

static uint8_t receiveBuffer[USBBUFFERSIZE];
static uint8_t receiveBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];
static uint8_t transmitBuffer[USBBUFFERSIZE];
static uint8_t transmitBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer txBuffer;
const tUSBBuffer rxBuffer;
static tCDCSerInstance serialInstance;

/* Function prototypes */
static unsigned long cbRxHandler(void *cbData, unsigned long event,
                         unsigned long eventMsg, void *eventMsgPtr);
static unsigned long cbSerialHandler(void *cbData, unsigned long event,
                             unsigned long eventMsg, void *eventMsgPtr);
static unsigned long cbTxHandler(void *cbData, unsigned long event,
                         unsigned long eventMsg, void *eventMsgPtr);

static uint32_t rxData(uint8_t *pStr, uint32_t length);
static uint32_t txData(const uint8_t *pStr, uint32_t length);

/* The languages supported by this device. */
const uint8_t langDescriptor[] =
{
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

/* The manufacturer string. */
const uint8_t manufacturerString[] =
{
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
    't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};

/* The product string. */
const uint8_t productString[] =
{
    2 + (16 * 2),
    USB_DTYPE_STRING,
    'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0,
    'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0
};

/* The serial number string. */
const uint8_t serialNumberString[] =
{
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};

/* The interface description string. */
const uint8_t controlInterfaceString[] =
{
    2 + (21 * 2),
    USB_DTYPE_STRING,
    'A', 0, 'C', 0, 'M', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,
    'r', 0, 'o', 0, 'l', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
    'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

/* The configuration description string. */
const uint8_t configString[] =
{
    2 + (26 * 2),
    USB_DTYPE_STRING,
    'S', 0, 'e', 0, 'l', 0, 'f', 0, ' ', 0, 'P', 0, 'o', 0, 'w', 0,
    'e', 0, 'r', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0,
    'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0,
    'o', 0, 'n', 0
};

/* The descriptor string table. */
const uint8_t * const stringDescriptors[] =
{
    langDescriptor,
    manufacturerString,
    productString,
    serialNumberString,
    controlInterfaceString,
    configString
};

#define STRINGDESCRIPTORSCOUNT (sizeof(stringDescriptors) / \
                                sizeof(uint8_t *))

const tUSBDCDCDevice serialDevice =
{
    USB_VID_TI,
    USB_PID_SERIAL,
    0,
    USB_CONF_ATTR_SELF_PWR,

    cbSerialHandler,
    NULL,

    USBBufferEventCallback,
    (void *)&rxBuffer,

    USBBufferEventCallback,
    (void *)&txBuffer,

    stringDescriptors,
    STRINGDESCRIPTORSCOUNT,

    &serialInstance
};

const tUSBBuffer rxBuffer =
{
    0,                      /* This is a receive buffer. */
    cbRxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketRead,          /* pfnTransfer */
    USBDCDCRxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    receiveBuffer,              /* pcBuffer */
    USBBUFFERSIZE,              /* ulBufferSize */
    receiveBufferWorkspace      /* pvWorkspace */
};

const tUSBBuffer txBuffer =
{
    1,                       /* This is a transmit buffer. */
    cbTxHandler,                /* pfnCallback */
    (void *)&serialDevice,      /* Callback data is our device pointer. */
    USBDCDCPacketWrite,         /* pfnTransfer */
    USBDCDCTxPacketAvailable,   /* pfnAvailable */
    (void *)&serialDevice,      /* pvHandle */
    transmitBuffer,             /* pcBuffer */
    USBBUFFERSIZE,              /* ulBufferSize */
    transmitBufferWorkspace     /* pvWorkspace */
};

static tLineCoding g_sLineCoding=
{
    115200,                     /* 115200 baud rate. */
    1,                          /* 1 Stop Bit. */
    0,                          /* No Parity. */
    8                           /* 8 Bits of data. */
};

/*
 *  ======== cbRxHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
static unsigned long cbRxHandler(void *cbData, unsigned long event,
                         unsigned long eventMsg, void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_RX_AVAILABLE:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
            break;

        case USB_EVENT_DATA_REMAINING:
            break;

        case USB_EVENT_REQUEST_BUFFER:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbSerialHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
static unsigned long cbSerialHandler(void *cbData, unsigned long event,
                             unsigned long eventMsg, void *eventMsgPtr)
{
    tLineCoding *psLineCoding;

    /* Determine what event has happened */
    switch (event) {
        case USB_EVENT_CONNECTED:
            state = USBCDCD_STATE_INIT;
            //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            break;

        case USB_EVENT_DISCONNECTED:
            state = USBCDCD_STATE_UNCONFIGURED;
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            break;

        case USBD_CDC_EVENT_GET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /* Copy the current line coding information into the structure. */
            psLineCoding->ulRate = g_sLineCoding.ulRate;
            psLineCoding->ucStop = g_sLineCoding.ucStop;
            psLineCoding->ucParity = g_sLineCoding.ucParity;
            psLineCoding->ucDatabits = g_sLineCoding.ucDatabits;
            break;

        case USBD_CDC_EVENT_SET_LINE_CODING:
            /* Create a pointer to the line coding information. */
            psLineCoding = (tLineCoding *)eventMsgPtr;

            /*
             * Copy the line coding information into the current line coding
             * structure.
             */
            g_sLineCoding.ulRate = psLineCoding->ulRate;
            g_sLineCoding.ucStop = psLineCoding->ucStop;
            g_sLineCoding.ucParity = psLineCoding->ucParity;
            g_sLineCoding.ucDatabits = psLineCoding->ucDatabits;
            break;

        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            break;

        case USBD_CDC_EVENT_SEND_BREAK:
            break;

        case USBD_CDC_EVENT_CLEAR_BREAK:
            break;

        case USB_EVENT_SUSPEND:
            break;

        case USB_EVENT_RESUME:
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== cbTxHandler ========
 *  Callback handler for the USB stack.
 *
 *  Callback handler call by the USB stack to notify us on what has happened in
 *  regards to the keyboard.
 *
 *  @param(cbData)          A callback pointer provided by the client.
 *
 *  @param(event)           Identifies the event that occurred in regards to
 *                          this device.
 *
 *  @param(eventMsgData)    A data value associated with a particular event.
 *
 *  @param(eventMsgPtr)     A data pointer associated with a particular event.
 *
 */
static unsigned long cbTxHandler(void *cbData, unsigned long event,
                         unsigned long eventMsg, void *eventMsgPtr)
{
    switch (event) {
        case USB_EVENT_TX_COMPLETE:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            break;

        default:
            break;
    }

    return (0);
}

/*
 *  ======== rxData ========
 */
static uint32_t rxData(uint8_t *pStr, uint32_t length)
{
    uint32_t read = 0;

    if (length) {
    	read = USBBufferRead(&rxBuffer, pStr, length);
    }

    return (read);
}

/*
 *  ======== txData ========
 */
static uint32_t txData(const uint8_t *pStr, uint32_t length)
{
    unsigned long buffAvailSize;
    uint32_t bufferedCount = 0;
    uint32_t sendCount = 0;
    uint8_t *sendPtr;

    while (bufferedCount != length) {
        /* Determine the buffer size available */
        buffAvailSize = USBBufferSpaceAvailable(&txBuffer);

        /* Determine how much needs to be sent */
        if ((length - bufferedCount) > buffAvailSize) {
            sendCount = buffAvailSize;
        }
        else {
            sendCount = length - bufferedCount;
        }

        /* Adjust the pointer to the data */
        sendPtr = (uint8_t *)pStr + bufferedCount;

        /* Place the contents into the USB BUffer */
        bufferedCount += USBBufferWrite(&txBuffer, sendPtr, sendCount);

#if 0
        /* Pend until some data was sent through the USB*/
        if (!Semaphore_pend(semTxSerial, timeout)) {
            break;
        }
#endif
    }

    return (bufferedCount);
}

/*
 *  ======== USBCDCD_init ========
 */
uint8_t USBCDCD_init(void)
{
    /* State specific variables */
    state = USBCDCD_STATE_UNCONFIGURED;

    /* Set the USB stack mode to Device mode with VBUS monitoring */
    USBStackModeSet(0, USB_MODE_DEVICE, 0);

    USBBufferInit(&txBuffer);
    USBBufferInit(&rxBuffer);

    /*
     * Pass our device information to the USB HID device class driver,
     * initialize the USB controller and connect the device to the bus.
     */
    if (!USBDCDCInit(0, &serialDevice)) {
        return -1;
    }
    return 0;
}

/*
 *  ======== USBCDCD_receiveData ========
 */
uint32_t USBCDCD_receiveData(uint8_t *pStr, uint32_t length)
{
    uint32_t retValue = 0;

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            USBCDCD_waitForConnect();
            break;

        case USBCDCD_STATE_INIT:
            state = USBCDCD_STATE_IDLE;
            retValue = rxData(pStr, length);
            break;

        case USBCDCD_STATE_IDLE:
            retValue = rxData(pStr, length);
            break;

        default:
            break;
    }

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

    return (retValue);
}

/*
 *  ======== USBCDCD_sendData ========
 */
uint32_t USBCDCD_sendData(const uint8_t *pStr, uint32_t length)
{
    uint32_t retValue = 0;

    switch (state) {
        case USBCDCD_STATE_UNCONFIGURED:
            //USBCDCD_waitForConnect();
            break;

        case USBCDCD_STATE_INIT:
            state = USBCDCD_STATE_IDLE;
            retValue = txData(pStr, length);
            break;

        case USBCDCD_STATE_IDLE:
            retValue = txData(pStr, length);
            break;

        default:
            break;
    }
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);

    return (retValue);
}

/*
 *  ======== USBCDCD_waitForConnect ========
 */
void USBCDCD_waitForConnect(void)
{
    while (state == USBCDCD_STATE_UNCONFIGURED);
}
