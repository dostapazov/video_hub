/*
 * app.c
 *
 *  Created on: 6 сент. 2018 г.
 *      Author: user
 */
#include "app.h"
#include "raspi_link.h"
#include "tcp_server.h"

#include "timers.h"
#include "crc.h"
#include "dio.h"

#ifdef RELEASE
extern IWDG_HandleTypeDef hiwdg;
#endif

static const DEV_INFO_t dev_info={DINF_SIGNATURE,VID,PID,HARD_REV,HARD_MOD, SOFT_REV, SOFT_MOD};

//uint32_t debud_tx_num=0, debud_rx_num=0;
//
//static
//
uint32_t led_blink_time = 0;
uint8_t blink_cnt=0, blink_found=0;
uint32_t wd_check_time = 0;
uint32_t btn_reset_time = 0;
uint32_t reset_check_time = 0;
uint32_t start_check_time = 0;


uint32_t cam_switch_time = 0;
uint32_t cam_num = 1;

uint8_t led_idx = 0;
uint8_t prev_reset_btn = GPIO_PIN_SET;
//uint8_t work_mode;
uint8_t raspi_ready = 0;


uint32_t err=2;
static uint8_t        pc_link_pack_rx[PC_BUF_SIZE] = {0};
static pc_link_pack_t pc_link_pack_tx = {0}; //буфер хрнения принимаемого пакета
static uint16_t       pc_link_point_rx; //указатель на текущий принимаемый байт
static uint32_t       pc_link_rx_pack_time;	//время на приём пакета, после которого ощищается и сбрасывается приёмный буфер
static uint32_t       pc_link_tx_pack_time;	//время передачи пакета
static uint8_t 		  channel=0; //активный канал

BTN_PIN_t rp_heartbits[_RASPI_COUNT_]; //моргалки малинок


/**
  * @brief  отправка пакета
  *
  */
static uint16_t pc_link_pack_transmit(uint8_t type)
{
if (tcp_socket!=NULL)
	{
	int cnt;
	pc_link_pack_tx.cam.header.mode.byte  = PC_LINK_TX_FLAG;
	pc_link_pack_tx.cam.header.session = PC_LINK_SESSION_ID;
	pc_link_pack_tx.cam.header.number  = ++tcp_socket->tx_number;
	pc_link_pack_tx.cam.header.version=PC_LINK_VERSION;
	if (type==2)
		{
		pc_link_pack_tx.cam.header.type=2;
		pc_link_pack_tx.cam.header.size=sizeof(pc_link_pack_tx.cam.data);

		//pc_link_pack_tx.fld.data.f.err=0;
		for (cnt=0; cnt<4; cnt++)
			{
			if (cam_st_out[cnt]==0) pc_link_pack_tx.cam.data.out[cnt]=0;
			if (cam_st_out[cnt]==1) pc_link_pack_tx.cam.data.out[cnt]=1;
			if (cam_st_out[cnt]==2) pc_link_pack_tx.cam.data.out[cnt]=2;
			if (cam_st_out[cnt]==3) pc_link_pack_tx.cam.data.out[cnt]=4;
			if (cam_st_out[cnt]==4) pc_link_pack_tx.cam.data.out[cnt]=8;
			}

		pc_link_pack_tx.cam.crc32=crc32_ether((uint8_t*)&pc_link_pack_tx, sizeof(pc_link_pack_tx.cam)-sizeof(pc_link_pack_tx.cam.crc32), 1);
		tcp_server_send_data(tcp_socket, (uint8_t*)&pc_link_pack_tx, sizeof(pc_link_pack_tx.cam));
		}
	if (type==3)
		{
		pc_link_pack_t * kadr = (pc_link_pack_t *)pc_link_pack_rx;

		pc_link_pack_tx.time.header.type=3;
		pc_link_pack_tx.time.header.size=sizeof(pc_link_pack_tx.time)-sizeof(pc_link_pack_tx.time.header)-sizeof(pc_link_pack_tx.time.crc32);
		pc_link_pack_tx.time.msec=kadr->time.msec;
		pc_link_pack_tx.time.sec=kadr->time.sec;
		pc_link_pack_tx.time.min=kadr->time.min;
		pc_link_pack_tx.time.h=kadr->time.h;
		pc_link_pack_tx.time.d=kadr->time.d;
		pc_link_pack_tx.time.m=kadr->time.m;
		pc_link_pack_tx.time.y=kadr->time.y;
		pc_link_pack_tx.time.crc32=crc32_ether((uint8_t*)&pc_link_pack_tx, sizeof(pc_link_pack_tx.time)-sizeof(pc_link_pack_tx.time.crc32), 1);
		tcp_server_send_data(tcp_socket, (uint8_t*)&pc_link_pack_tx, sizeof(pc_link_pack_tx.time));
		}
	}
 	return(1);
}

/**
  * @brief  Проверка на корректность и обработка принятого пакета
  *
  */
static uint8_t pc_link_pack_process(void)
{
uint16_t cnt;
pc_link_pack_t * kadr = (pc_link_pack_t *)pc_link_pack_rx;
uint32_t calc_crc   =  0;

	if (kadr->time.header.type==3)
		{
		calc_crc=crc32_ether (pc_link_pack_rx, sizeof(kadr->time)-sizeof(kadr->time.crc32), 1);
		if(calc_crc == kadr->time.crc32)
			{
			if (kadr->time.header.size>=8)
				{
				raspi_link_set_time(kadr->time.sec,kadr->time.min,kadr->time.h,kadr->time.d,kadr->time.m,kadr->time.y);
				}
			pc_link_pack_transmit(3);
			return(sizeof(kadr->time));
			}
		}
	else
		{
		calc_crc=crc32_ether (pc_link_pack_rx, sizeof(kadr->cam)-sizeof(kadr->cam.crc32), 1);
		if(calc_crc == kadr->cam.crc32)
			{

			// 	crc.dword=crc32_ether(&pc_link_pack_rx.byte[0], (pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)), 1);
			// 	for (cnt=0; cnt<4; cnt++)
			// 		if (crc.byte[cnt]!=pc_link_pack_rx.byte[pc_link_pack_rx.fld.header.size+sizeof(pc_link_pack_rx.fld.header)+cnt])
			// 			{
			// 			return(0);
			// 			}

			if (kadr->cam.header.type==0x55)
				{
				memcpy(&pc_link_pack_tx.cam.data, &dev_info, sizeof(dev_info));
				}
			if (kadr->cam.header.type==2)
				{
				if (kadr->cam.header.mode.bit.active)
					{
					if (kadr->cam.header.mode.bit.channel==1) channel=1;
					if ((kadr->cam.header.mode.bit.channel==2)&&(channel==0)) channel=2;

					if (kadr->cam.header.mode.bit.channel==channel)
						{
						for (cnt=0; cnt<4; cnt++)
							{
							if (kadr->cam.data.out[cnt]==0) raspi_link_set_cam(cnt+1, 0);
							if (kadr->cam.data.out[cnt]==1) raspi_link_set_cam(cnt+1, 1);
							if (kadr->cam.data.out[cnt]==2) raspi_link_set_cam(cnt+1, 2);
							if (kadr->cam.data.out[cnt]==4) raspi_link_set_cam(cnt+1, 3);
							if (kadr->cam.data.out[cnt]==8) raspi_link_set_cam(cnt+1, 4);
							//if (cam_st_out[cnt]!=cam_set_out[cnt])  rp_heartbits[cnt].wait_time = timers_get_finish_time(_RASPI_START_DURATION_);
							}
						}
					}
				else
					{
					if ((kadr->cam.header.mode.bit.channel==1)&&(channel==1))
						channel=0;
					}
				}
			return(sizeof(kadr->cam));
			}
		}
	return 0;
}


/**
 * store new rx data into rx buffer
 * @return how mach bytes added into buffer
 */
uint16_t __tcp_rx_data_add(uint8_t * rx_buf,uint16_t bsz,uint8_t * data,uint16_t *data_len, uint16_t *rx_point)
{
   uint16_t bytes_added =  MIN(data_len[0],bsz-rx_point[0]);
   if(bytes_added)
   {
     memcpy(rx_buf+rx_point[0],data,bytes_added);
     *data_len -= bytes_added;
     *rx_point += bytes_added;
   }
   return bytes_added;
}

int __tcp_rx_sync_kadr(uint8_t * rx_buf,uint16_t * rx_bytes)
{
    int found = 0;
	uint8_t * ptr = rx_buf;
   do{
	   pc_link_pack_t * kadr = (pc_link_pack_t *)rx_buf;
	  //if(kadr->header.start == PC_LINK_RX_FLAG && kadr->header.session == PC_LINK_SESSION_ID)
	   if(kadr->cam.header.session == PC_LINK_SESSION_ID)
		  found = 1;
	  else
	  {
	     *rx_buf = 0;
	     ptr=0;
//		 ptr = memchr(rx_buf,PC_LINK_RX_FLAG,rx_bytes[0]);
//	     if(!ptr)
	    	 *rx_bytes = 0;
//	     else
//	     {
//	       uint16_t delta = ptr-rx_buf;
//	       *rx_bytes -= delta;
//	       memmove(rx_buf,ptr,rx_bytes[0]);
//	     }

	  }
     }while(ptr && !found);
   return found;
}


int tcp_handle_kadrs(uint8_t * rx_buf,uint16_t * rx_bytes)
{uint32_t dt;
  int kadrs_handled = 0;
  uint32_t sz=0;

  while(rx_bytes[0] && __tcp_rx_sync_kadr(rx_buf,rx_bytes))
  {
	sz=pc_link_pack_process();
	if(sz)
		{
		  ++kadrs_handled;
		  *rx_bytes -= sz;
		  memmove(rx_buf,rx_buf+sz,rx_bytes[0]);
		  pc_link_rx_pack_time=timers_get_finish_time(PC_LINK_MAX_RX_PACK_TIME); //установить таймайт на приём всего пакета

		  dt=PC_LINK_TX_INTERVAL-timers_get_time_left(pc_link_tx_pack_time); //посчтать сколько прошло времени  с момента последней отправки
		  if (dt<PC_LINK_TX_MIN_INTERVAL) //если прошло меньше чем минимально допустимо
			  pc_link_tx_pack_time=timers_get_finish_time(PC_LINK_TX_MIN_INTERVAL-dt);
		  else							  //иначе отправит паекет немедленно
		  	  pc_link_tx_pack_time=timers_get_finish_time(0);
		}
	    else
	    *rx_buf = 0;
  }



  return kadrs_handled;
}


/**
  * @brief  Обработчик принятых пакетов по Ethernet: предопределённая функция вызывается из MX_LWIP_Process()
  *
  * @param  err_t: код ошибки на канале Ethernet
  */

err_t tcp_handle_data(uint8_t* data, uint16_t len, tcp_server_socket *tss)
{
  tcp_socket = tss;

  while(len)
  {
	 data += __tcp_rx_data_add(pc_link_pack_rx,sizeof(pc_link_pack_rx),data,&len,&pc_link_point_rx);
	 tcp_handle_kadrs(pc_link_pack_rx,&pc_link_point_rx);
  }
  pc_link_rx_pack_time=timers_get_finish_time(PC_LINK_MAX_RX_PACK_TIME); //установить таймайт на приём всего пакета
  return ERR_OK;
}

//err_t tcp_handle_data(uint8_t* data, uint16_t len, tcp_server_socket *tss)
//{uint16_t pack_size=0;
//uint8_t all_pack_process=0;
//
//	tcp_socket=tss;
//	if ((pc_link_point_rx!=0)&&(timers_get_time_left(pc_link_rx_pack_time)==0)) //если прошлый пакет принимался слишком долго
//				pc_link_point_rx=0;
//	pc_link_rx_pack_time=timers_get_finish_time(PC_LINK_MAX_RX_PACK_TIME); //установить таймайт на приём всего пакета
//
//	if (len>(sizeof(pc_link_pack_rx)-pc_link_point_rx)) //проверить что в приёмном буфере достаточно места
//		len=(sizeof(pc_link_pack_rx)-pc_link_point_rx); //ограничить размер копируемых данных размером свободного места
//	memcpy(&pc_link_pack_rx.byte[0]+pc_link_point_rx, data, len); //сохранить принятый пакет
//	pc_link_point_rx+=len;
//
//	do //в буфере может быть больше одного пакета, перебираем все пакеты
//		{
//		all_pack_process=0; //для начала считаем что все пакеты обработаны
//		if (pc_link_point_rx>=sizeof(pc_link_pack_rx.fld.header)) //Если заголовок пакета уже принят
//			{
//			pack_size=sizeof(pc_link_pack_rx.fld.header)+pc_link_pack_rx.fld.header.size+4;
//			if (pc_link_point_rx>=pack_size) //если принят весь пакет
//				{
//				 if(pc_link_pack_process())
//				 {
//				 memcpy(&pc_link_pack_rx.byte[0],&pc_link_pack_rx.byte[pack_size],len-pack_size); //удалить из буфера обработанную часть
//      			 pc_link_point_rx-=pack_size; //передвинуть указатель
//     			   if (pc_link_point_rx!=0) all_pack_process=1; //есть ли в буфере необработанные данные
//				 }
//				 else
//			     {
//					 pc_link_point_rx = 0;
//					 all_pack_process = 0;
//				 }
//
//				}
//			}
//		}
//	while (all_pack_process);
//
//	return(0);
//}

/**
  * @brief  Проверка наличия сигнала работы от raspberry
  *
  */
void raspi_check_st(void)
{uint8_t dio_st, cnt;

	//dio_out_write(0xFF);
	dio_st=dio_key_press(); //прочитать сосотяние сигналов от малинок, которые были в 1, а потом стали 0
	for (cnt=0; cnt<_RASPI_COUNT_; cnt++)
		{
		if ((dio_st>>cnt)&1)
			{
			//if (timers_diff_tick(HAL_GetTick(), rp_heartbits[cnt].pulse_time)<_RASPI_WD_INTERVAL_)
			//	{
				rp_heartbits[cnt].wait_time = timers_get_finish_time(_RASPI_WD_INTERVAL_);
			//	pc_link_pack_tx.fld.data.f.err&=(~((1<<cnt)<<4));
			//	}
			//else
			//	{
			//	rp_heartbits[cnt].wait_time = timers_get_finish_time(_RASPI_START_DURATION_);
			//	pc_link_pack_tx.fld.data.f.err|=((1<<cnt)<<4);
			//	}
				pc_link_pack_tx.cam.data.err&=(~(1<<cnt));
			rp_heartbits[cnt].pulse_time=HAL_GetTick();
			}
		if (dio_out_bit_read(cnt)) //прочитать состояние сигнала "сброс" и если на малинка сейчас не выставляется сигнал сброса
			{
			if (timers_get_time_left(rp_heartbits[cnt].wait_time)==0)
				{
				dio_out_bit_write(cnt, GPIO_PIN_RESET);
				rp_heartbits[cnt].rst_time = timers_get_finish_time(_RESET_DURATION_);
				rp_heartbits[cnt].wait_time = timers_get_finish_time(_RASPI_START_DURATION_);
				pc_link_pack_tx.cam.data.err |=(1<<cnt);
				}
			}
		else //если на малинку выставляется сигнал сброса
			{
			if (timers_get_time_left(rp_heartbits[cnt].rst_time)==0) //проверить не пора ли убрать сигнал сброса
				dio_out_bit_write(cnt, GPIO_PIN_SET);
			}
		}
	dio_key_reset(); //очистить обработанные сигналы
}

/**
  * @brief  Антидребезг принажатии ресет и перезагрузка системы
  *
  */
void but_rst_ctrl(void)
{uint32_t press_timeout;

	//press_timeout=timers_get_finish_time(1000);
	//while (timers_get_time_left(press_timeout)!=0)
	press_timeout=0;
	HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_RED, GPIO_PIN_SET);
	while (press_timeout<200000)
		{
		if (HAL_GPIO_ReadPin(BUT_RST)==GPIO_PIN_SET)
			{
			HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_RED, GPIO_PIN_RESET);
			return;
			}
		press_timeout++;
#ifdef RELEASE
		HAL_IWDG_Refresh(&hiwdg);
#endif
		}
	//asm ("cpsid i");
	//HAL_Delay(_RESET_DURATION_);
	press_timeout=0;
	while (press_timeout<100000)
		{
		HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);
		dio_out_write(0);
		press_timeout++;
#ifdef RELEASE
		HAL_IWDG_Refresh(&hiwdg);
#endif
		}
	dio_out_write(0xFF);
	HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_RED, GPIO_PIN_RESET);
	NVIC_SystemReset();
}

void app_init()
{

	HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_RED, GPIO_PIN_SET);
	extern struct netif gnetif;
	ethernetif_init(&gnetif);

	raspi_link_init(&huart1);
	for (int i = 0; i < _RASPI_COUNT_; i++)
		{
		rp_heartbits[i].wait_time = timers_get_finish_time(_RASPI_START_DURATION_); /*HAL_GetTick() + _RASPI_START_DURATION_ + _RASPI_WD_INTERVAL_*/
		rp_heartbits[i].rst_time = 0;
		rp_heartbits[i].pulse_time = 0;

		}
	prev_reset_btn = HAL_GPIO_ReadPin(BUT_RST);

	cam_switch_time = HAL_GetTick() + _RASPI_START_DURATION_ + _CAM_SWITCH_INTERVAL_;

	HAL_GPIO_WritePin(LED_RED, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);

	//set_work_mode(WM_STARTUP);
#ifdef RELEASE
	//HAL_IWDG_Start(&hiwdg);
	__HAL_IWDG_START(&hiwdg);
#endif
}

void app_step()
{
//------------------------------WDT-------------------------------------------
#ifdef RELEASE
		HAL_IWDG_Refresh(&hiwdg);
#endif
	//uint32_t now = HAL_GetTick();
	//btn_reset_check();
	if (HAL_GPIO_ReadPin(BUT_RST)==GPIO_PIN_RESET)
		but_rst_ctrl();
//------------------------------отладка----------------------------------------

	//debud_tx_num = pc_link_pack_tx.header.number;
	//debud_rx_num = ((pc_link_pack_t *)pc_link_pack_rx)->header.number;

//--------------------------обмен по Ethernet----------------------------------
	//MX_LWIP_Process();
    tcp_server_svc();

//------------------------работа с raspberry----------------------------------
	raspi_link_step();
	raspi_check_st();

	if(pc_link_point_rx && timers_get_time_left(pc_link_rx_pack_time)==0)
	{
	  pc_link_point_rx = 0;
	}

	if (timers_get_time_left(pc_link_tx_pack_time)==0)
	 {
		pc_link_pack_transmit(2);
		pc_link_tx_pack_time=timers_get_finish_time(PC_LINK_TX_INTERVAL);
	 }


//---------------------------индикатор работы-----------------------
	if (timers_get_time_left(led_blink_time)==0)
		{
		if (pc_link_pack_tx.cam.data.err==0)
			{
			HAL_GPIO_WritePin(LED_RED, GPIO_PIN_RESET);
			HAL_GPIO_TogglePin(LED_GREEN);
			led_blink_time=timers_get_finish_time(LED_BLINK_FAST);
			blink_cnt=0;
			}
		else
			{
			HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);
			if (HAL_GPIO_ReadPin(LED_RED)==GPIO_PIN_SET) blink_cnt++;
			HAL_GPIO_TogglePin(LED_RED);
			for (blink_found=0; blink_found<_RASPI_COUNT_; blink_found++)
				{
				if ((pc_link_pack_tx.cam.data.err>>blink_found)&1) break;
				}
			blink_found++;

			if (blink_cnt>=blink_found)
				{
				blink_cnt=0;
				led_blink_time=timers_get_finish_time(LED_BLINK_TIME);
				}
			else
				led_blink_time=timers_get_finish_time(LED_BLINK_FAST);
			}
		}
	//-------------------------------------------------------------------
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin==BUT_RST_Pin)
		{
		but_rst_ctrl();
		}
}


