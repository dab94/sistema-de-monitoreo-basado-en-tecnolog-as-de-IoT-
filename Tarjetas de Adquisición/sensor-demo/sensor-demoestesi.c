/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "contiki.h"
#include "contiki-net.h"
#include "rpl/rpl-private.h"
#include "mqtt.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"


/*POSIBLE QUITAR A FUTURO */
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"


#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/leds.h"

/*POSIBLE QUITAR A FUTURO */
#include "dev/button-sensor.h"
#include "dev/radio-sensor.h"
#include "dev/sensor-common.h"
#include "st-lib.h"


#ifdef X_NUCLEO_IKS01A1
#include "dev/temperature-sensor.h"
#include "dev/humidity-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/magneto-sensor.h"
#include "dev/acceleration-sensor.h"
#include "dev/gyroscope-sensor.h"
#endif /*X_NUCLEO_IKS01A1*/

/*IPV6 POSIBLE QUITAR LUEGO DE HABLAR CON TAMURA*/
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
/*#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr) */
#endif

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"


#include <string.h>

#include "constantes.h"

process_event_t sPressure;
process_event_t sHumidity;
process_event_t sTemp;
/*---------------------------------------------------------------------------*/
/*
 * MQTT broker address
 */
static const char *broker_ip = MQTT_DEMO_BROKER_IP_ADDR;
/*---------------------------------------------------------------------------*/
/*
 * A timeout used when waiting for something to happen (e.g. to connect or to
 * disconnect)
 */
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Provide visible feedback via LEDS during various states */
/* When connecting to broker */


#define CONNECTING_LED_DURATION    (CLOCK_SECOND >> 2)

/* Each time we try to publish */
#define PUBLISH_LED_ON_DURATION    (CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER              0xFF
#define RECONNECT_INTERVAL         (CLOCK_SECOND * 2)

/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS         RETRY_FOREVER
#define CONNECTION_STABLE_TIME     (CLOCK_SECOND * 5)
/*---------------------------------------------------------------------------*/
static struct timer connection_life;
static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;

#define STATE_INIT                    0
#define STATE_REGISTERED              1
#define STATE_CONNECTING              2
#define STATE_CONNECTED               3
#define STATE_PUBLISHING              4
#define STATE_DISCONNECTED            5
#define STATE_NEWCONFIG               6
#define STATE_CONFIG_ERROR         0xFE
#define STATE_ERROR                0xFF
/*---------------------------------------------------------------------------*/
#define CONFIG_EVENT_TYPE_ID_LEN     32
#define CONFIG_CMD_TYPE_LEN           8
#define CONFIG_IP_ADDR_STR_LEN       64
/*---------------------------------------------------------------------------*/
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION         (NET_CONNECT_PERIODIC >> 1)
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_demo_process);
PROCESS_NAME(lvl_process);
PROCESS_NAME(buzzer_process);
PROCESS_NAME(Humidity_process);
PROCESS_NAME(temp_process);

AUTOSTART_PROCESSES(&mqtt_demo_process,&lvl_process,&buzzer_process,&Humidity_process,&temp_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */


char *vibracion = "Vibracion";
char *flujo = "Flujo";
char *nivel = "Nivel";
char *humedad = "Humedad Relativa";
char *temp = "Temperatura";

typedef struct mqtt_client_config {
  //char event_type_id[CONFIG_EVENT_TYPE_ID_LEN];
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  char cmd_type[CONFIG_CMD_TYPE_LEN];
  clock_time_t pub_interval;
  uint16_t broker_port;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topic.
 * Make sure they are large enough to hold the entire respective string
 */

//REVISAR EL PROJECT_CONF.H
static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
static struct mqtt_connection conn;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static struct ctimer ct;
static struct etimer et_Humidity;
static struct etimer et_temp;
static struct etimer et_lvl;
static char *buf_ptr;
static uint16_t seq_nr_value = 0;
/*---------------------------------------------------------------------------*/
static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
PROCESS(mqtt_demo_process, "MQTT Demo");
PROCESS (lvl_process, "Water Pressure process");
PROCESS (buzzer_process, "Buzzer process");
PROCESS (Humidity_process, "Water Humidity process");
PROCESS (temp_process, "Temperature process");
/*---------------------------------------------------------------------------*/
int
ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr)
{
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}
/*---------------------------------------------------------------------------*/
static void
publish_led_off(void *d)
{
  leds_off(LEDS_GREEN);
}
/*---------------------------------------------------------------------------*/
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  printf("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len,
      chunk_len);

  /* If we don't like the length, ignore */
  if(topic_len != 17 || chunk_len != 1) {
    printf("Incorrect topic or chunk len. Ignored\n");
    return;
  }
  
  
//REVISAR EN CASO DE ERROR
  if(strncmp(&topic[13], "leds", 4) == 0) {
    if(chunk[0] == '1') {
      leds_on(LEDS_RED);
      printf("Turning LED RED on!\n");
    } else if(chunk[0] == '0') {
      leds_off(LEDS_RED);
      printf("Turning LED RED off!\n");
    }
    return;
  }
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    printf("APP - Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    printf("APP - MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_demo_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      printf("APP - Application received a publish on topic '%s'. Payload "
          "size is %i bytes. Content:\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
                msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
    printf("APP - Application is subscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    printf("APP - Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    printf("APP - Publishing complete.\n");
    break;
  }
  default:
    printf("APP - Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
construct_pub_topic(void)
{
  int len = snprintf(pub_topic, BUFFER_SIZE, "/v1.6/devices/%s","nodo1");
  
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Pub Topic too large: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_sub_topic(void)
{
  int len = snprintf(sub_topic, BUFFER_SIZE, "zolertia/cmd/%s",
                     conf.cmd_type);
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Sub Topic too large: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  printf("Subscription topic %s\n", sub_topic);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "d:%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config(void)
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_sub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_pub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  /* Reset the counter */
  seq_nr_value = 0;

  state = STATE_INIT;

  /*
   * Schedule next timer event ASAP
   * If we entered an error state then we won't do anything when it fires.
   * Since the error at this stage is a config error, we will only exit this
   * error state if we get a new config.
   */
  etimer_set(&publish_periodic_timer, 0);

  return;
}
/*---------------------------------------------------------------------------*/
static int
init_config()
{
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));
 /* memcpy(conf.event_type_id, DEFAULT_EVENT_TYPE_ID,
         strlen(DEFAULT_EVENT_TYPE_ID));
  memcpy(conf.event_type_id1, DEFAULT_EVENT_TYPE_ID,
         strlen(DEFAULT_EVENT_TYPE_ID1));*/
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.cmd_type, DEFAULT_SUBSCRIBE_CMD_TYPE, 4);

  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
subscribe(void)
{
  /* Publish MQTT topic in IBM quickstart format */
  mqtt_status_t status;

  status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);

  printf("APP - Subscribing to %s\n", sub_topic);
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    printf("APP - Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void publish(uint16_t value1,char *event_name1,uint16_t value2,char *event_name2,uint16_t value3,char *event_name3)
{
  int len;
  //uint16_t aux;
  int remaining = APP_BUFFER_SIZE;

  seq_nr_value++;
  buf_ptr = app_buffer;

 /* len = snprintf(buf_ptr, remaining,
                 "{"
                 "\"Level\":{"
                 "\"myName\":\"%s\","
                 "\"Seq no\":%d,"
                 "\"Uptime (sec)\":%lu",
                 BOARD_STRING, seq_nr_value, clock_seconds());*/

/*  if(len < 0 || len >= remaining) {
    printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
    return;
  }*/

  len = snprintf(buf_ptr, remaining, "{\"%s\":%u,\"%s\":%u,\"%s\":%u}",event_name1,value1,event_name2,value2,event_name3,value3);


  if(len < 0 || len >= remaining) {
    printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
    return;
  }
  
  remaining -= len;
  buf_ptr += len;

  mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
               strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);

  printf("APP - Publish to %s: %s\n", pub_topic, app_buffer);
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
               conf.pub_interval * 3);

  state = STATE_CONNECTING;
}

/*---------------------------------------------------------------------------*/
static void print_local_addresses(void)
{
  int i;
  uint8_t state;

  printf("This IP Address:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) 
  {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) 
    {
      printf(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
state_machine(uint16_t value1,char *event_name1,uint16_t value2,char *event_name2,uint16_t value3,char *event_name3)
{
  switch(state) {
  case STATE_INIT:
    /* If we have just been configured register MQTT connection */
    mqtt_register(&conn, &mqtt_demo_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    printf("Init\n");

    /* Notice there is no "break" here, it will continue to the
     * STATE_REGISTERED
     */
  case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) 
    {
      /* Registered and with a public IP. Connect */
      printf("Registered. Connect attempt %u\n", connect_attempt);
      connect_to_broker();
    } 
    else 
    {
      leds_on(LEDS_GREEN);
      ctimer_set(&ct, NO_NET_LED_DURATION, publish_led_off, NULL);
    }
    etimer_set(&publish_periodic_timer, NET_CONNECT_PERIODIC);
    return;
    break;

  case STATE_CONNECTING:
    leds_on(LEDS_GREEN);
    ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    printf("Connecting (%u)\n", connect_attempt);
    break;

  case STATE_CONNECTED:
    /* Notice there's no "break" here, it will continue to subscribe */

  case STATE_PUBLISHING:
    /* If the timer expired, the connection is stable. */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
      connect_attempt = 0;
    }

    if(mqtt_ready(&conn) && conn.out_buffer_sent) 
    {
      /* Connected. Publish */
      if(state == STATE_CONNECTED) 
      {
        subscribe();
        state = STATE_PUBLISHING;

      } else 
      {
        leds_on(LEDS_GREEN);
        printf("Publishing\n");
        ctimer_set(&ct, PUBLISH_LED_ON_DURATION, publish_led_off, NULL);
        publish(value1,event_name1,value2,event_name2,value3,event_name3);
      }
      etimer_set(&publish_periodic_timer, conf.pub_interval);

      /* Return here so we don't end up rescheduling the timer */
      return;

    } 
    else 
    {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd).
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us.
       */
      printf("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
    }
    break;

  case STATE_DISCONNECTED:
    printf("Disconnected\n");
    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      printf("Disconnected. Attempt %u in %lu ticks\n", connect_attempt, interval);

      etimer_set(&publish_periodic_timer, interval);

      state = STATE_REGISTERED;
      return;

    } else {
      /* Max reconnect attempts reached. Enter error state */
      state = STATE_ERROR;
      printf("Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;

  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    printf("Bad configuration.\n");
    return;

  case STATE_ERROR:
  default:
    leds_on(LEDS_GREEN);
    printf("Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_demo_process, ev, data)
{
  PROCESS_BEGIN();
static uint16_t preHumidity,pretemp,prePressure;
preHumidity = 0;
pretemp = 0;
prePressure = 0;
  //printf("MQTT Demo Process\n");

  if(init_config() != 1) {
    PROCESS_EXIT();
  }

   static uint16_t datum;

  /* These are variables to store strings for each sending process  */
  
  //static char *sender_p;
  
  update_config();

  while(1)
  {
    PROCESS_YIELD();

    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
    ev == PROCESS_EVENT_POLL || ev == sHumidity || ev == sTemp || ev == sPressure)
    {
        datum = *((uint16_t *)data);
        if (ev == sHumidity)
        {
            preHumidity = datum;
        }
        else if (ev == sTemp)
        {
            pretemp = datum;
        }
        else if (ev == sPressure)
        {
            prePressure =  datum;
        }
        state_machine(prePressure,"Pressure",pretemp,"Temp",preHumidity,"Humidity");
        
    }
  }

  PROCESS_END();
}


PROCESS_THREAD (lvl_process, ev, data)
{
  static uint8_t  thresholdY,
                  t_lvl;
  static uint16_t lvl_counter;
  

  /* Every process start with this macro, we tell the system this is the start
   * of the thread
   */
  PROCESS_BEGIN ();
  /* Create a pointer to the data, as we are expecting a string we use "char" */
  //static char *parent;
  //parent = (char * )data;

  //printf ("Water level process started by %s\n", parent);

  /* We need to allocate a numeric process ID to our process */
  sPressure = process_alloc_event ();
  t_lvl = TEMP_SPERIOD;
  lvl_counter = NORMAL_Pressure;
  etimer_set (&et_lvl, t_lvl * CLOCK_SECOND);
  
  SENSORS_ACTIVATE(pressure_sensor);
  SENSORS_ACTIVATE(button_sensor);
  /* Send turn off buzzer */
  process_post ( &buzzer_process, sPressure, &thresholdY );
  //leds_on (LEDS_GREEN);   /* normal Pressure */

  /* And now we wait for the button_sensor process to inform us about the water
   * Pressure. If the user button is clicked,the water Pressure is decreased; if the user button is pressed for more than one second, the water Pressure is increased
   */

  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL (etimer_expired(&et_lvl));

    /* Read the temperature sensor */
    if(ev == sensors_event && data == &button_sensor) {
        lvl_counter = pressure_sensor.value(0);
        printf("Presure:\t%d.%d C\n", lvl_counter / 10, ABS_VALUE(lvl_counter) % 10);

        process_post ( &mqtt_demo_process, sPressure, &lvl_counter );
        etimer_reset (&et_lvl);
        }
  }

  /* This is the end of the process, we tell the system we are done.  Even if
   * we won't reach this due to the "while(...)" we need to include it
   */
  PROCESS_END ();
}

PROCESS_THREAD(buzzer_process, ev, data)
{
  PROCESS_BEGIN();
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD (temp_process, ev, data)
{
  /* This is a variable to store the sample period */
  static uint8_t t_temp;

  /* This is a variable to store the temperature */
  static uint16_t temp;

  /*  The sensors are already started at boot */

  PROCESS_BEGIN ();

  /* Create a pointer to the data, as we are expecting a string we use "char" */
  //static char *parent;
  //parent = (char * )data;

  print_local_addresses();
  
  //printf ("Temperature process started by %s\n", parent);

  /* We need to allocate a numeric process ID to our process */
  sTemp = process_alloc_event ();

  t_temp = TEMP_SPERIOD; /* temperature sampling period */
  /* Spin the timer */
  etimer_set (&et_temp, t_temp * CLOCK_SECOND);
  
  SENSORS_ACTIVATE(temperature_sensor);

  while (1)
  {

    PROCESS_WAIT_EVENT_UNTIL (etimer_expired(&et_temp));

    /* Read the temperature sensor */
    temp = temperature_sensor.value(0);
    printf("Temperature:\t%d.%d C\n", temp / 10, ABS_VALUE(temp) % 10);

    /* Send Pressure value to head node */
    process_post ( &mqtt_demo_process, sTemp, &temp );
    /* Reset timer */
    etimer_reset (&et_temp);
  }

  PROCESS_END();
}

PROCESS_THREAD (Humidity_process, ev, data)
{
  /* This is a variable to store the sample period */
  static uint8_t  t_Humidity;
  /* These are variables to store the samples */
//  static uint16_t adc1;
  static uint16_t adc3;

  PROCESS_BEGIN ();

  /* Create a pointer to the data, as we are expecting a string we use "char" */
  //static char *parent;
  //parent = (char * )data;

  //printf ("Water Humidity process started by %s\n", parent);

  /* We need to allocate a numeric process ID to our process */
  sHumidity = process_alloc_event ();
  
  SENSORS_ACTIVATE(humidity_sensor);
  /* Wait before starting the process */
  t_Humidity = Humidity_SPERIOD; /* Humidity sampling period */
  etimer_set (&et_Humidity, t_Humidity * CLOCK_SECOND);

  while (1)
  {
    /* This protothread waits until timeout
     */
    PROCESS_WAIT_EVENT_UNTIL (etimer_expired(&et_Humidity));

    /* Read from ADC1 */
/*    adc1 = adc_zoul.value(ZOUL_SENSORS_ADC1);
    printf ("ADC1 (water Humidity) = %u mV\n", adc1);*/
    adc3= humidity_sensor.value(0);
    printf("Humidity:\t%d.%d rH\n", adc3 / 10, ABS_VALUE(adc3) % 10);

/* Send Pressure value to head node */
//    process_post ( &send2br_process, sHumidity, &adc1 );
    process_post ( &mqtt_demo_process, sHumidity, &adc3 );

    /* Reset timer */
    etimer_reset (&et_Humidity);
  }

  /* This is the end of the process, we tell the system we are done.  Even if
   * we won't reach this due to the "while(...)" we need to include it
   */
  PROCESS_END();
}
