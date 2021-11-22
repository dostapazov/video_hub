/*
 * tcp_server.h
 *
 *  Created on: 27 мар. 2018 г.
 *      Author: user
 *      raspi video_hub
 */

#ifndef APP_DEVICES_TCP_SERVER_H_
#define APP_DEVICES_TCP_SERVER_H_
#include "hal_includes.h"
#include "lwip.h"
#include "lwip/tcp.h"
#include "tcp_priv.h"

#define _TCP_SERVER_PORT_	5000

//
//LWIP datatypes
//

/* structure for maintaing connection infos to be passed as argument to LwIP callbacks*/
typedef struct _tcp_server_socket
{
  struct tcp_pcb *pcb;    			/* pointer on the current tcp_pcb */
  struct pbuf *p;         			/* pointer on the received/to be transmitted pbuf */
  uint16_t     tx_number;
  uint16_t     rx_number;
  struct _tcp_server_socket* next;
  struct _tcp_server_socket* prev;
} tcp_server_socket;

extern tcp_server_socket *tcp_socket;

typedef enum
{
	ETHERNET_LINK_ERR,
	ETHERNET_LINK_OK,
	ETHERNET_LINK_UP,
	ETHERNET_LINK_DN,
	ETHERNET_LINK_FIRST,
}eth_link_t;

//
//callback data types
//

typedef err_t (*data_handle_fn)(uint8_t* data, uint16_t len, tcp_server_socket *tss);

//
//public members
//


void    tcp_server_init(uint8_t priority, uint16_t port, data_handle_fn rec_fn);
err_t   tcp_server_send_data      ( tcp_server_socket *es, uint8_t* data, uint32_t len);
void    tcp_server_connection_close(tcp_server_socket *tss);
void    tsp_server_close_all_connections();
void    tcp_server_svc();
uint8_t ethernet_get_link(void);
err_t   tcp_handle_data(uint8_t* data, uint16_t len, tcp_server_socket *tss);


#endif /* APP_DEVICES_TCP_SERVER_H_ */
