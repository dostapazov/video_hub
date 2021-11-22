/*
 * tcp_server.c
 *
 *  Created on: 27 мар. 2018 г.
 *      Author: user
 *      raspi video_hub
 */

#include <string.h>
#include "tcp_server.h"
//
//private LWIP callback forwards
//
err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
err_t tcp_server_recv  (void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void  tcp_server_error (void *arg, err_t err);
err_t tcp_server_poll  (void *arg, struct tcp_pcb *tpcb);
err_t tcp_server_sent  (void *arg, struct tcp_pcb *tpcb, u16_t len);

//
//private forwards
//

err_t tcp_server_send_tss(tcp_server_socket *tss);
tcp_server_socket* socket_allock();
              void socket_free(tcp_server_socket* socket);
uint8_t ethernet_get_link(void);

//
//static members
//
static struct tcp_pcb *tcppcb = NULL;
static uint8_t def_prio = TCP_PRIO_NORMAL;
static data_handle_fn data_handle_function = 0;
static tcp_server_socket* socket_list = 0;

static eth_link_t eth_link=ETHERNET_LINK_FIRST;

//
//public members
//
tcp_server_socket *tcp_socket=NULL;
/**
 * @brief создание TCP сервера
 * @param priority приоритет (TCP_PRIO_MIN, TCP_PRIO_NORMAL, TCP_PRIO_MAX)
 * @param port TCP-порт сервера
 */
void tcp_server_init(uint8_t priority, uint16_t port, data_handle_fn rec_fn)
{
	//выход, если сервер уже создан
	if (tcppcb != NULL) return;

	// Создаем новый экземпляр TCP-сервера
	tcppcb = tcp_alloc(priority);
	if (NULL == tcppcb) return;


	def_prio = priority;
	//memset(&tss_list, 0, sizeof(tss_list));

	err_t err;
	//Указываем на каком порту будет жить сервер и какие IP-адреса будут допущены
	err = tcp_bind(tcppcb, IP_ADDR_ANY, port);
	if (err == ERR_OK)
	{
		//Запускаем "слушатель" TCP. По факту тут сервер и стратует
		tcppcb = tcp_listen(tcppcb);
		err = ERR_MEM;
		if (tcppcb)
		{
			//Указываем callback-функцию обработки входящего подключения
			tcp_accept(tcppcb, tcp_server_accept);
			err = ERR_OK;
		}

	}


	if (ERR_OK != err)
	{
		tcp_pcb_purge(tcppcb);
		memp_free(MEMP_TCP_PCB, tcppcb);
		tcppcb = NULL;
	}
	else
		data_handle_function = rec_fn;
}


/**
 * @brief Послать данные через сокет
 * @param tss Указатель на сокет
 * @param data Буфер с данными
 * @param len Размер передаваемых данных (байт)
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_send_data(tcp_server_socket *tss, uint8_t* data, uint32_t len)
{
	//TODO: сделать разделение больших пакетов (> 1500 байт) на мелкие
	err_t err;
	if(!tss->p || tss->p->len <len)
	{
    	if(tss->p)pbuf_free(tss->p);
    	  tss->p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
	}


    if(tss->p)
    {
      //tss->p->payload = (void*)data;
    	memcpy(tss->p->payload,data,len);
	    err = tcp_server_send_tss(tss);
	}
    else
    	{err = ERR_MEM;}
    if(err)
    	{
    	 err = err+0;
    	}
	return err;
}

/**
 * @brief Закрывает сокет
 * @param tpcb
 * @param tss
 */
void tcp_server_connection_close(tcp_server_socket *tss)
{
	if (tss != NULL)
  		 socket_free(tss);
}


void tsp_server_close_all_connections()
{
  while(socket_list)
	    socket_free(socket_list);
}


//
//private LWIP callback forwards
//

/**
 *
 * @brief callback-функция обработки запроса на соединение TCP. "Дает добро"(или не дает) на подключение нового клиента
 * @param arg
 * @param newpcb
 * @param err
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{

	UNUSED(arg);
	UNUSED(err);

	if (!data_handle_function) return ERR_VAL;

	err_t ret_err;

	tcp_setprio(newpcb, def_prio);

	tcp_server_socket *tss = socket_allock();
	if (tss != NULL)
	{
		tss->pcb = newpcb;
		tss->p = NULL;

		/* Это чтобы структурка "tss" прилетала нам в параметре  "void *arg" */
		tcp_arg(newpcb, tss);
		tcp_recv(newpcb, tcp_server_recv);
		tcp_err(newpcb, tcp_server_error);
		tcp_poll(newpcb, tcp_server_poll, 1);
		//tcp_sent(newpcb, tcp_server_sent);
		ret_err = ERR_OK;
	}
	else
	{
		ret_err = ERR_MEM;
	}
	return ret_err;
}

/**
 * @brief callback-функция обработки принятых данных
 * @param arg
 * @param tpcb
 * @param p
 * @param err
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	//Скучная прелюдия
	uint16_t tot_len = 0;
	err_t res = ERR_OK;
	tcp_server_socket *tss = (tcp_server_socket *)arg;
	if (p)
	{
    if (!data_handle_function) err =  ERR_VAL;
    if (!tss) err = ERR_ARG;
	//Обработка принятых данных

	struct pbuf* ptr = p;
	while ((ptr != NULL) && (err == ERR_OK))
	{
		res = data_handle_function(ptr->payload, ptr->len, tss);
		tot_len+= ptr->len;
		ptr = ptr->next;
	}
	pbuf_free(p);

	if(tot_len)
	  tcp_recved(tpcb,tot_len);
	}
	else
		res =   ERR_BUF;
	return res;
}

/**
 * @brief callback-функция обработки ошибок
 * @param arg
 * @param err Код ошибки @err_enum_t
 */
void tcp_server_error(void *arg, err_t err)
{
	UNUSED(err);
	if (arg != NULL)
		tcp_server_connection_close((tcp_server_socket *)arg);

}

/**
 * @brief callback-функция основного цикла LWIP
 * @param arg
 * @param tpcb
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err = ERR_OK;
	tcp_server_socket *tss = (tcp_server_socket *)arg;
	if (tss != NULL)
	{
	  //	if (tss->p != NULL)
	  //		ret_err = tcp_server_send_tss(tss);
	}
	return ret_err;
}

/**
 * @brief callback-функция обработки подтверждения отправки пакета
 * @param arg
 * @param tpcb
 * @param len
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	UNUSED(len);
	tcp_server_socket *tss = (tcp_server_socket*)arg;

	err_t err = ERR_OK;

	if (tss->p != NULL)
		err = tcp_server_send_tss(tss);

	return err;
}


//
//private members
//

/**
 * @brief Приватная функция отправки данных
 * @param tss
 * @return Код ошибки @err_enum_t
 */
err_t tcp_server_send_tss(tcp_server_socket *tss)
{
	struct tcp_pcb *tpcb = tss->pcb;
	uint16_t tot_len = 0;
	if(tss)
	{
	struct pbuf *ptr = tss->p;

	err_t wr_err = ERR_BUF;
	uint16_t len = ptr->len;
	if (ptr != NULL)
		{
		wr_err = tcp_write(tpcb, ptr->payload, len, TCP_WRITE_FLAG_COPY);
		if (wr_err == ERR_OK)
			{
			tcp_output(tpcb);
			tot_len += ptr->len;
			ptr = ptr->next;
			}
		}
	if(!wr_err && tot_len) tcp_recved(tpcb, tot_len);
	if(ERR_IS_FATAL(wr_err) || wr_err == ERR_CONN)
		tcp_server_connection_close(tss);
	return wr_err;
	}
	return ERR_ARG;
}

/**
 * @brief Создает новый сокет и размещает его в списке сокетов
 * @return
 */
tcp_server_socket* socket_allock()
{
	tcp_server_socket* res = (tcp_server_socket *)mem_malloc(sizeof(tcp_server_socket));
	if (res != NULL)
	{

		memset(res,0,sizeof(*res));
		// add new socket to head of the list
		res->next    = socket_list;
		socket_list  = res;

	}
	return res;
}

/**
 * @brief Удаляет сокет из списка и освобождает память
 * @param socket
 */
void socket_free(tcp_server_socket* socket)
{
   if(socket)
   {
	if(socket == tcp_socket)
		tcp_socket = NULL;
	if (socket->prev == NULL)
		socket_list = socket->next;
	else
		socket->prev->next = socket->next;

	if(socket->pcb)
	  tcp_close(socket->pcb);
	 if(socket->p) pbuf_free(socket->p);
	   mem_free(socket);
   }
}

//----------------------------------------------
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
  return;
}

uint8_t ethernet_get_link(void)
{  uint32_t phyreg;
   extern struct netif gnetif;

	HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &phyreg);
	if (phyreg & PHY_LINKED_STATUS)
		{
		if (eth_link==ETHERNET_LINK_FIRST)
			{
			MX_LWIP_Init();
			eth_link=ETHERNET_LINK_UP;
			}
		else
			{
			if ((eth_link==ETHERNET_LINK_ERR)||(eth_link==ETHERNET_LINK_DN))
				{
				eth_link=ETHERNET_LINK_UP;
				netif_set_link_up(&gnetif);
				//dhcp_start(&gnetif);
				}
			else eth_link=ETHERNET_LINK_OK;
			}
		}
	else
		{
		if (eth_link!=ETHERNET_LINK_FIRST)
			{
			if ((eth_link==ETHERNET_LINK_OK)||(eth_link==ETHERNET_LINK_UP))
				{
				netif_set_link_down(&gnetif);
				eth_link=ETHERNET_LINK_DN;
				}
			else
				{
				eth_link=ETHERNET_LINK_ERR;
				}
			}
		}

	return(eth_link);
}

void tcp_server_svc()
{
	eth_link_t state = ethernet_get_link();
	switch(state)
	{
	case ETHERNET_LINK_OK:
		MX_LWIP_Process();
		break;
	case ETHERNET_LINK_UP:
		 tcp_server_init(TCP_PRIO_MAX, _TCP_SERVER_PORT_, tcp_handle_data);
		 break;
	case ETHERNET_LINK_DN:
		tsp_server_close_all_connections();
		break;
	default:
		break;
	}
}






