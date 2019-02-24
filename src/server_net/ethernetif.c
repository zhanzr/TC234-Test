/**
 ******************************************************************************
 * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/ethernetif.c
 * @author  MCD Application Team
 * @brief   This file implements Ethernet network interface drivers for lwIP
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include <string.h>

#include "enc28j60.h"

#include "FreeRTOS.h"

#include "uart_int.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT                 ( portMAX_DELAY )
/* Stack size of the interface thread */
#define INTERFACE_THREAD_STACK_SIZE            ( 350 )

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

uint16_t g_low_input_len;
volatile uint8_t net_buf[MAX_FRAMELEN];

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];

uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void ethernetif_input( void const * argument );
TaskHandle_t g_input_handler;

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
                       Ethernet MSP Routines
 *******************************************************************************/

/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH)
 *******************************************************************************/
/**
 * @brief In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */

	/* Set netif link flag */
	netif->flags |= NETIF_FLAG_LINK_UP;

	/* set netif MAC hardware address length */
	netif->hwaddr_len = ETH_HWADDR_LEN;

	/* set netif MAC hardware address */
	netif->hwaddr[0] =  MAC_ADDR0;
	netif->hwaddr[1] =  MAC_ADDR1;
	netif->hwaddr[2] =  MAC_ADDR2;
	netif->hwaddr[3] =  MAC_ADDR3;
	netif->hwaddr[4] =  MAC_ADDR4;
	netif->hwaddr[5] =  MAC_ADDR5;

	/* set netif maximum transfer unit */
	netif->mtu = 1500;

	/* Accept broadcast address and ARP traffic */
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	/* create the task that handles the ETH_MAC */
	xTaskCreate((TaskFunction_t )ethernetif_input,
			(const char*    )"ethernetif_input",
			(uint16_t       )768,
			(void*          )netif,
			(UBaseType_t    )tskIDLE_PRIORITY + 3,
			(TaskHandle_t*  )&g_input_handler);
}


/**
 * @brief This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
	err_t errval;
	struct pbuf *q;
	uint32_t framelength = 0;
	uint32_t bufferoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t payloadoffset = 0;

	bufferoffset = 0;

	enc28j60PacketSend(p->len, (uint8_t*)((uint8_t*)q->payload));

	errval = ERR_OK;

	return errval;
}

/**
 * @brief Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf * low_level_input(struct netif *netif) {
	struct pbuf *p = NULL, *q = NULL;
	uint16_t len = 0;
	uint8_t *buffer;
	uint32_t bufferoffset = 0;
	uint32_t payloadoffset = 0;
	uint32_t byteslefttocopy = 0;
	uint32_t i=0;

	/* Obtain the size of the packet and put it into the "len" variable. */
	len = g_low_input_len;

	if (len > 0) {
		/* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	}

	if (p != NULL) {
		memcpy( (uint8_t*)((uint8_t*)p->payload), (uint8_t*)net_buf, g_low_input_len);
	}

	return p;
}

extern struct netif gnetif;
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);
/**
 * @brief This function is the ethernetif_input task, it is processed when a packet
 * is ready to be read from the interface. It uses the function low_level_input()
 * that should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input( void const * argument ) {
	struct pbuf *p;
	struct netif *netif = (struct netif *) argument;

	for( ;; ) {
		g_low_input_len = enc28j60PacketReceive(MAX_FRAMELEN, net_buf);

		if(g_low_input_len!=0) {
			do{
				p = low_level_input( netif );

				if (p != NULL) {
					printf("%s %u, %08X, %08X\n",
							__func__, g_low_input_len, (uint32_t)netif, (uint32_t)&gnetif);
					flush_stdout();

					printf("%08X, %08X\n",
							(uint32_t)netif->input, (uint32_t)tcpip_input);
					flush_stdout();
					if (netif->input( p, netif) != ERR_OK ) {
						printf("%s %d\n", __func__, __LINE__);
						flush_stdout();
						pbuf_free(p);
					} else {
						printf("%s %d\n", __func__, __LINE__);
						flush_stdout();
					}
				}
			}while(p!=NULL);
		} else {
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
	}
}

/**
 * @brief Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;

	/* We directly use etharp_output() here to save a function call.
	 * You can instead declare your own function an call etharp_output()
	 * from it if you have to do some checks before sending (e.g. if link
	 * is available...) */
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;

	/* initialize the hardware */
	low_level_init(netif);

	return ERR_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
