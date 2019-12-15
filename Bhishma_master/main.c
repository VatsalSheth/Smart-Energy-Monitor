/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs BT Mesh Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko BT Mesh C application.
 * The application starts unprovisioned Beaconing after boot
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* C Standard Library headers */
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "lcd_driver.h"

#include "src/gpio.h"
#include "src/rtcc.h"
#include "src/cmu.h"
#include "src/clk_output.h"

/* Bluetooth stack headers */
#include "bg_types.h"

#include "gatt_db.h"
//#include "aat.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lighting_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include <em_gpio.h>
#include <gpiointerrupt.h>

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#include "retargetserial.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

// bluetooth stack heap
#define MAX_CONNECTIONS 2

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)
		+ BTMESH_HEAP_SIZE + 1760];

#define EXT_SIGNAL_PB0_PRESS (0x01)
#define EXT_SIGNAL_PB1_PRESS (0x04)

// If using PCB and not development board
#define USING_PCB

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define TIMER_ID_RESTART    (78)
#define TIMER_ID_FACTORY_RESET  (77)
#define TIMER_ID_PROVISIONING   (66)
#define TIMER_ID_RETRANS    (10)
#define TIMER_ID_FRIEND_FIND (20)
#define TIMER_ID_SAVE_STATE (60)
#define TIMER_ID_SENSOR (81)
#define TIMER_30_SEC (30)
#define TIMER_15_MIN (15)
#define TIMER_ID_FIRE_LED0 (13)
#define TIMER_ID_FIRE_LED1 (23)
#define TIMER_ID_FIRE_RESET (33)
#define TIMER_ID_CLIENT_GET (44)
#define TIMER_ID_DISPLAY_TIME (55)
#define TIMER_TIME_DISP (99)
#define APP_TASK (1)
#define UPD_CAL (2)

/**
 *  State of the LEDs is updated by calling LED_set_state().
 *  The new state is passed as parameter, possible values are defined below.
 */
#define LED_STATE_OFF    (0)   /* light off (both LEDs turned off)   */
#define LED_STATE_ON     (1)   /* light on (both LEDs turned on)     */
#define LED_STATE_PROV   (3)   /* provisioning (LEDs blinking)       */

#define SERVER_LPN1_ADDRESS	(0x02)
#define SERVER_LPN2_ADDRESS	(0x09)
#define CLIENT_SERVER_ADDRESS (0x04)

//#define APPKEY_INDEX (2)

#define SEC_THRESH_1 (30)
#define SEC_THRESH_2 (0)
#define MIN_THRESH_1 (0)
#define MIN_THRESH_2 (15)
#define HOUR_THRESH_1 (22)
#define HOUR_THRESH_2 (8)

#define SET_YEAR (2018)
#define SET_MONTH (12)
#define SET_DATE (13)
#define SET_DAY (4)
#define SET_HOUR (14)
#define SET_MINUTE (11)
#define SET_SEC (00)
#define SET_MSEC (000)
#define RESPONSE_REQUIRED (0)

#define servercode

/*Global Variables*/
static uint16 _elem_index = 0xffff; /* For indexing elements of the node (this example has only one element)*/
static uint16 _my_address = 0; /* Address of the Primary Element of the Node */
static uint8 switch_pos = 0; /*current position of switch*/
static uint8 trid = 0; /* transaction identifier */
static uint8 request_count; /* number of on/off requests to be sent */
static uint8 lightness_percent = 0; /* lightness level percentage */
uint8 time_rst_done;	//	for checking if time reset is done

uint8_t server_lpn_address;
uint8_t appkey;

#ifdef servercode
static PACKSTRUCT(struct lightbulb_state {
  // On/Off Server state
  uint8_t onoff_current;
  uint8_t onoff_target;

  // Transition Time Server state
  uint8_t transtime;

  // On Power Up Server state
  uint8_t onpowerup;

  // Primary Generic Level
  int16_t pri_level_current;
  int16_t pri_level_target;

}) lightbulb_state;
#endif
//
// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

const gecko_configuration_t config = { .bluetooth.max_connections =
		MAX_CONNECTIONS, .bluetooth.max_advertisers = MAX_ADVERTISERS,
		.bluetooth.heap = bluetooth_stack_heap, .bluetooth.heap_size =
				sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
		.bluetooth.sleep_clock_accuracy = 100, .gattdb = &bg_gattdb_data,
		.btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.pa.config_enable = 1, // Enable high power PA
		.pa.input = GECKO_RADIO_PA_INPUT_VBAT,// Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.max_timers = 16, };

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
void mesh_native_bgapi_init(void);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

#ifndef testing
uint16_t resnew;
char passkeyOOB[20];
char pslot_status[20];
char time_disp[20];
char fire_LCD[20];
static uint8 conn_handle = 0xFF; /* handle of the last opened LE connection */
#endif

void initiate_time_reset(void) {
	printf("time reset\r\n");
#ifndef USING_PCB
	LCD_write("\n***\nTIME RESET\n***", LCD_ROW_CONNECTION);
#endif
	setDateAndTime(SET_YEAR, SET_MONTH, SET_DATE, SET_DAY, SET_HOUR, SET_MINUTE,
			SET_SEC, SET_MSEC);
}

void initiate_factory_reset(void) {
	printf("factory reset\r\n");
#ifndef USING_PCB
	LCD_write("\n***\nFACTORY RESET\n***", LCD_ROW_CONNECTION);
#endif
	/* if connection is open then close it before rebooting */
	if (conn_handle != 0xFF) {
		gecko_cmd_le_connection_close(conn_handle);
	}

	/* perform a factory reset by erasing PS storage. This removes all the keys and other settings
	 that have been configured for this node */
	gecko_cmd_flash_ps_erase_all();

	setDateAndTime(SET_YEAR, SET_MONTH, SET_DATE, SET_DAY, SET_HOUR, SET_MINUTE,
			SET_SEC, SET_MSEC);
//   reboot after a small delay
	gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}

void LED_set_state(int state) {
	switch (state) {
	case LED_STATE_OFF:
		GPIO_PinOutClear(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutClear(BSP_LED1_PORT, BSP_LED1_PIN);
		break;
	case LED_STATE_ON:
		GPIO_PinOutSet(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutSet(BSP_LED1_PORT, BSP_LED1_PIN);
		break;
	case LED_STATE_PROV:
		GPIO_PinOutToggle(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutToggle(BSP_LED1_PORT, BSP_LED1_PIN);
		break;

	default:
		break;
	}
}


#ifdef servercode
static errorcode_t pri_level_response(uint16_t element_index,
                                      uint16_t client_addr,
                                      uint16_t appkey_index)
{
  struct mesh_generic_state current, target;

  current.kind = mesh_generic_state_level;
  current.level.level = lightbulb_state.pri_level_current;

  target.kind = mesh_generic_state_level;
  target.level.level = lightbulb_state.pri_level_target;

  return mesh_lib_generic_server_response(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                                          element_index,
                                          client_addr,
                                          appkey_index,
                                          &current,
                                          &target,
                                          0,
                                          0x00);
}

static errorcode_t pri_level_update(uint16_t element_index)
{
  struct mesh_generic_state current, target;

  current.kind = mesh_generic_state_level;
  current.level.level = lightbulb_state.pri_level_current;

  target.kind = mesh_generic_state_level;
  target.level.level = lightbulb_state.pri_level_target;

  return mesh_lib_generic_server_update(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                                        element_index,
                                        &current,
                                        &target,
                                        0);
}

static errorcode_t pri_level_update_and_publish(uint16_t element_index)
{
  errorcode_t e;

  e = pri_level_update(element_index);
  if (e == bg_err_success) {
    e = mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                                        element_index,
                                        mesh_generic_state_level);
  }

  return e;
}

static void pri_level_request(uint16_t model_id,
                              uint16_t element_index,
                              uint16_t client_addr,
                              uint16_t server_addr,
                              uint16_t appkey_index,
                              const struct mesh_generic_request *request,
                              uint32_t transition_ms,
                              uint16_t delay_ms,
                              uint8_t request_flags)
{
  // for simplicity, this demo assumes that all level requests use set level.
  // other type of requests are ignored

  uint16_t lightness;

  if (request->kind != mesh_generic_request_level) {
    return;
  }

  printf("pri_level_request: level=%d, transition=%lu, delay=%u\r\n",
         request->level, transition_ms, delay_ms);

  if (lightbulb_state.pri_level_current == request->level) {
    printf("Request for current state received; no op\n");
  } else {
    printf("Setting pri_level to <%d>\r\n", request->level);

    lightness = request->level + 32768;

    if (transition_ms == 0 && delay_ms == 0) { // Immediate change
      lightbulb_state.pri_level_current = request->level;
      lightbulb_state.pri_level_target = request->level;
//      lightbulb_state.lightness_current = lightness;
//      lightbulb_state.lightness_target = lightness;

      // update LED Temperature
//      LEDS_SetLevel(lightness, 0);
    } else if (delay_ms > 0) {
      // a delay has been specified for the light change. Start a soft timer
      // that will trigger the change after the given delay
      // Current state remains as is for now
      lightbulb_state.pri_level_target = request->level;
//      lightbulb_state.lightness_target = lightness;
//      gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(delay_ms), TIMER_ID_DELAYED_PRI_LEVEL, 1);
      // store transition parameter for later use
//      delayed_pri_level_trans = transition_ms;
    } else {
      // no delay but transition time has been set.
      lightbulb_state.pri_level_target = request->level;
//      lightbulb_state.lightness_target = lightness;
//      LEDS_SetLevel(lightness, transition_ms);

      // lightbulb current state will be updated when transition is complete
//      gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(transition_ms), TIMER_ID_PRI_LEVEL_TRANSITION, 1);
    }
//    lightbulb_state_changed();
  }

  if (request_flags & MESH_REQUEST_FLAG_RESPONSE_REQUIRED) {
    pri_level_response(element_index, client_addr, appkey_index);
  } else {
    pri_level_update(element_index);
  }
}

static void pri_level_change(uint16_t model_id,
                             uint16_t element_index,
                             const struct mesh_generic_state *current,
                             const struct mesh_generic_state *target,
                             uint32_t remaining_ms)
{
  if (lightbulb_state.pri_level_current != current->level.level) {
    printf("pri_level_change: from %d to %d\r\n", lightbulb_state.pri_level_current, current->level.level);
    lightbulb_state.pri_level_current = current->level.level;
//    lightbulb_state_changed();
  } else {
    printf("pri_level update -same value (%d)\r\n", lightbulb_state.pri_level_current);
  }
}
#endif

/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr) {
	char name[20];
	// create unique device name using the bytes of the Bluetooth address
	sprintf(name, "%x:%x:%x:%x:%x:%x", pAddr->addr[5], pAddr->addr[4], pAddr->addr[3],
									   pAddr->addr[2], pAddr->addr[1], pAddr->addr[0]);

	printf("Device name: '%s'\r\n", name);

	resnew = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0,
			strlen(name), (uint8 *) name)->result;
	if (resnew) {
		printf(
				"gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n",
				resnew);
	}

	// show device name on the LCD
#ifndef USING_PCB
	LCD_write(name, 2);
#endif
}

/**
 * This function publishes one on/off request to change the state of light(s) in the group.
 * Global variable switch_pos holds the latest desired light state, possible values are
 * switch_pos = 1 -> PB1 was pressed, turn lights on
 * switch_pos = 0 -> PB0 was pressed, turn lights off
 *
 * This application sends multiple requests for each button press to improve reliability.
 * Parameter retrans indicates whether this is the first request or a re-transmission.
 * The transaction ID is not incremented in case of a re-transmission.
 */
void send_onoff_request(int retrans) {
	uint16 resp;
	uint16 delay;
	struct mesh_generic_request req;
	const uint32 transtime = 0; /* using zero transition time by default */

	req.kind = mesh_generic_request_on_off;
	req.on_off =
			switch_pos ?
					MESH_GENERIC_ON_OFF_STATE_ON :
					MESH_GENERIC_ON_OFF_STATE_OFF;

	// increment transaction ID for each request, unless it's a retransmission
	if (retrans == 0) {
		trid++;
	}

	/* delay for the request is calculated so that the last request will have a zero delay and each
	 * of the previous request have delay that increases in 50 ms steps. For example, when using three
	 * on/off requests per button press the delays are set as 100, 50, 0 ms
	 */
	delay = 0;

	resp = gecko_cmd_mesh_generic_client_publish(
	MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID, _elem_index, trid, transtime, // transition time in ms
			delay, 0,     // flags
			mesh_generic_request_on_off,     // type
			1,     // param len
			&req.on_off     /// parameters data
			)->result;

	if (resp) {
		printf("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n",
				resp);
	} else {
		printf("request sent, trid = %u, delay = %d\r\n", trid, delay);
	}

	/* keep track of how many requests has been sent */
	if (request_count > 0) {
		request_count--;
	}
}

/**
 * Handling of long button presses. This function called from the main loop when application receives
 * event gecko_evt_system_external_signal_id.
 *
 * parameter button defines which button was pressed, possible values
 * are 0 = PB0, 1 = PB1.
 *
 * This function is called from application context (not ISR) so it is safe to call BGAPI functions
 */
void handle_button_press(int button) {
	// PB0 -> switch off, PB1 -> switch on
	switch_pos = button;

	/* long press turns light ON or OFF, using Generic OnOff model */
	printf("PB%d -> turn light(s) ", button);
	if (switch_pos) {
		printf("on\r\n");
		lightness_percent = 100;
	} else {
		printf("off\r\n");
		lightness_percent = 0;
	}

	request_count = 3; // request is sent 3 times to improve reliability

	/* send the first request */
	send_onoff_request(0);

	/* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
	gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(50),
			TIMER_ID_RETRANS, 0);
}

///**
// * Initialization of the models supported by this node. This function registers callbacks for
// * each of the three supported models.
// */
void init_models(void) {
	mesh_lib_generic_server_register_handler(MESH_GENERIC_LEVEL_CLIENT_MODEL_ID,
											 0,
											 pri_level_request,
											 pri_level_change);
}

/**
 * This is a callback function that is invoked each time a GPIO interrupt in one of the pushbutton
 * inputs occurs. Pin number is passed as parameter.
 *
 * Note: this function is called from ISR context and therefore it is not possible to call any BGAPI
 * functions directly. The button state change is signaled to the application using gecko_external_signal()
 * that will generate an event gecko_evt_system_external_signal_id which is then handled in the main loop.
 */
void gpioint(uint8_t pin) {
	if (pin == BSP_BUTTON0_PIN) {
		gecko_external_signal(0x1);
	} else if (pin == BSP_BUTTON1_PIN) {
		gecko_external_signal(0x4);
	}
}

/**
 * Enable button interrupts for PB0, PB1. Both GPIOs are configured to trigger an interrupt on the
 * rising edge (button released).
 */
void enable_button_interrupts(void) {
	GPIOINT_Init();

	GPIO_ExtIntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, BSP_BUTTON0_PIN, true,
			false, true);
	GPIO_ExtIntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, BSP_BUTTON1_PIN, true,
			false, true);

	/* register the callback function that is invoked when interrupt occurs */
	GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, gpioint);
	GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, gpioint);
}

void friend_node_init(void) {
	mesh_lib_init(malloc, free, 8);
	resnew = gecko_cmd_mesh_friend_init()->result;
	if (resnew == 0) {
		printf("Initialized as friend\n\r");
		LCD_write("Friend Initialized", 4);
	} else
		printf("Friend initialization failed 0x%x\n\r", resnew);
	LCD_write("", 6);
	enable_button_interrupts();
	init_models();
}

PACKSTRUCT( struct date_time_struct {
	uint16 year;
	uint8 month;
	uint8 day;
	uint8 hour;
	uint8 minute;
	uint8 second;
})
;

PACKSTRUCT( struct day_date_time_struct {
	struct date_time_struct date_time;
	uint8 day_of_week;
})
;

PACKSTRUCT( struct exact_time_struct {
	struct day_date_time_struct day_date_time;
	uint8 frac_256;
})
;

PACKSTRUCT( struct current_time_characteristic {
	struct exact_time_struct exact_time;
	uint8 adjust_reason;
})
;

PACKSTRUCT( struct local_time_information_characteristic {
	int8 time_zone;
	uint8 dst_offset;
})
;

int main() {
	// Initialize device
	initMcu();
	// Initialize board
	initBoard();
	// Initialize application
	initApp();

	// Initialization of self-defined functions
	cmu_init();
	gpio_init();
//	rtcSetup();
	clock_output();
#ifndef USING_PCB
	LCD_init("BT Mesh Demo");
#endif
	gecko_stack_init(&config);
	gecko_bgapi_class_dfu_init();
	gecko_bgapi_class_system_init();
	gecko_bgapi_class_le_gap_init();
	gecko_bgapi_class_le_connection_init();
	gecko_bgapi_class_gatt_init();
	gecko_bgapi_class_gatt_server_init();
	gecko_bgapi_class_endpoint_init();
	gecko_bgapi_class_hardware_init();
	gecko_bgapi_class_flash_init();
	gecko_bgapi_class_test_init();
	gecko_bgapi_class_sm_init();

	gecko_bgapi_class_mesh_node_init();
//  gecko_bgapi_class_mesh_prov_init();
	gecko_bgapi_class_mesh_proxy_init();
	gecko_bgapi_class_mesh_proxy_server_init();
	gecko_bgapi_class_mesh_proxy_client_init();
	gecko_bgapi_class_mesh_generic_client_init();
#ifdef servercode
	gecko_bgapi_class_mesh_generic_server_init();
#endif
	//  gecko_bgapi_class_mesh_lpn_init();
	gecko_bgapi_class_mesh_friend_init();

	mesh_native_bgapi_init();
	gecko_initCoexHAL();

	RETARGET_SerialInit();
	RETARGET_SerialCrLf(true);

//	setTimeZone(4);
//	setDst(0);
//	time_rst_done = 0;

	while (1) {
		struct gecko_cmd_packet *evt = gecko_wait_event();
		bool pass = mesh_bgapi_listener(evt);
		if (pass) {
			handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
		}
	}
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt) {
	switch (evt_id) {
	case gecko_evt_dfu_boot_id:
		gecko_cmd_le_gap_set_mode(2, 2);
		break;

	case gecko_evt_system_boot_id:
		{
			struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();
			set_device_name(&pAddr->address);
		}

		// Initialize Mesh stack in Node operation mode, wait for initialized event
		gecko_cmd_mesh_node_init_oob(0x00, 0x03, 0x03, 0x08, 0x00, 0x04, 0x01);

#ifdef last_ditch
		gecko_cmd_hardware_set_soft_timer(32768, APP_TASK, 0); // 1sec continuous running for displaying time
		gecko_cmd_hardware_set_soft_timer(32768 * 60, UPD_CAL, 0); // 1min continuous running for updating calendar
		gecko_cmd_hardware_set_soft_timer(32768 * 3, TIMER_ID_SENSOR, 0);//3 seconds continuous running for polling sensor every 3 seconds(>< threshold)
#endif

		break;

	case gecko_evt_hardware_soft_timer_id:
		switch (evt->data.evt_hardware_soft_timer.handle) {
		case TIMER_ID_FACTORY_RESET:
			gecko_cmd_system_reset(0);
			break;

		case TIMER_ID_RESTART:
			gecko_cmd_system_reset(0);
			break;

		case TIMER_ID_PROVISIONING:
			LED_set_state(LED_STATE_PROV);
			break;

		case TIMER_ID_RETRANS:
			send_onoff_request(1); // param 1 indicates that this is a retransmission
			// stop retransmission timer if it was the last attempt
			if (request_count == 0) {
				gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_RETRANS, 0);
			}
			break;

#ifdef last_ditch
		case APP_TASK:
			applicationTask();
			break;

		case UPD_CAL:
			updateCalendar();
			break;

		case TIMER_30_SEC: {
			uint8_t seconds_2_send;
			seconds_2_send = retrieve_sec();
			uint16 resp;
			struct mesh_generic_request req;
			const uint32 transtime = 0; /* using zero transition time by default */

			req.kind = mesh_generic_request_on_off;
			/*Uncomment to increase threshold of toggling LPN LED to every 30 seconds
			 * Else uncommented part allws toggling of LPN LED every 15 seconds
			 req.on_off = seconds_2_send > SEC_THRESH_1 ? MESH_GENERIC_ON_OFF_STATE_ON : MESH_GENERIC_ON_OFF_STATE_OFF;
			 if((seconds_2_send > SEC_THRESH_1 && seconds_2_send < SEC_THRESH_1 + 5) || ((seconds_2_send > SEC_THRESH_2 && seconds_2_send < SEC_THRESH_2 + 5)))
			 */
			req.on_off =
					((seconds_2_send > 0) && (seconds_2_send < 5))
							|| ((seconds_2_send > 30) && (seconds_2_send < 35)) ?
							MESH_GENERIC_ON_OFF_STATE_ON :
							MESH_GENERIC_ON_OFF_STATE_OFF;
			if ((seconds_2_send > 0 && seconds_2_send < 5)
					|| ((seconds_2_send > 15 && seconds_2_send < 20))
					|| ((seconds_2_send > 30 && seconds_2_send < 35))
					|| ((seconds_2_send > 45 && seconds_2_send < 50))) {
				resp = gecko_cmd_mesh_generic_client_set(
				MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID, _elem_index,
						server_lpn_address, appkey, trid, transtime, // transition time in ms
						0,
						RESPONSE_REQUIRED,     // flags
						mesh_generic_request_on_off,     // type
						1,     // param len
						&req.on_off     /// parameters data
						)->result;

				gecko_cmd_hardware_set_soft_timer(32768 * 5,
						TIMER_ID_DISPLAY_TIME, 1);
				if (resp) {
					printf(
							"gecko_cmd_mesh_generic_client_set state failed,code %x\r\n",
							resp);
				} else {
					printf("request sent as value %d\r\n", req.on_off);
				}
			}
		}
			break;

		case TIMER_15_MIN: {
			uint8_t hours_2_send;
			uint8_t minutes_2_send;
			hours_2_send = retrieve_hours();
			minutes_2_send = retrieve_minutes();
			uint16 resp;
			struct mesh_generic_request req;
			const uint32 transtime = 0; /* using zero transition time by default */

			req.kind = mesh_generic_request_on_off;
			req.on_off =
					(hours_2_send > HOUR_THRESH_1)
							&& (hours_2_send < HOUR_THRESH_1 + 1) ?
							MESH_GENERIC_ON_OFF_STATE_ON :
							MESH_GENERIC_ON_OFF_STATE_OFF;
			if (((hours_2_send > HOUR_THRESH_1)
					&& (hours_2_send < HOUR_THRESH_1 + 1)
					&& (minutes_2_send > MIN_THRESH_1)
					&& (minutes_2_send < MIN_THRESH_2))
					|| (((hours_2_send > HOUR_THRESH_2)
							&& (hours_2_send < HOUR_THRESH_2 + 1)
							&& (minutes_2_send > MIN_THRESH_1)
							&& (minutes_2_send < MIN_THRESH_2)))) {
				resp = gecko_cmd_mesh_generic_client_set(
				MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID, _elem_index,
						server_lpn_address, appkey, trid, transtime, // transition time in ms
						0,
						RESPONSE_REQUIRED,     // flags
						mesh_generic_request_on_off,     // type
						1,     // param len
						&req.on_off     /// parameters data
						)->result;

				if (resp) {
					printf(
							"gecko_cmd_mesh_generic_client_set state failed,code %x\r\n",
							resp);
				} else {
					printf("request sent as value %d\r\n", req.on_off);
				}
			}
		}
			break;

#endif
		case TIMER_ID_SENSOR:
				LCD_write(fire_LCD, 6);
				uint16 resp;
				struct mesh_generic_request req;
				req.on_off = MESH_GENERIC_ON_OFF_STATE_ON;

				resp = gecko_cmd_mesh_generic_client_set(
				MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID, _elem_index,
				CLIENT_SERVER_ADDRESS, appkey, trid, 0, // transition time in ms
						0, 0,     // flags
						mesh_generic_request_on_off,     // type
						1,     // param len
						&req.on_off     /// parameters data
						)->result;

				if (resp) {
					printf(
							"Fiery gecko_cmd_mesh_generic_client_set state failed,code %x\r\n",
							resp);
				} else {
					printf("Fiery request sent as value %d\r\n", req.on_off);
				}
			break;

		case TIMER_ID_CLIENT_GET: {
			uint8 resp;
			resp =
					gecko_cmd_mesh_generic_client_get(
					MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID, _elem_index,
							server_lpn_address, appkey,
							mesh_generic_request_on_off)->result;
			if (resp) {
				printf("gecko_cmd_mesh_generic_client_get failed,code %x\r\n",
						resp);
			} else
				printf("Client Get sent\r\n");
		}
			break;

		case TIMER_ID_FIRE_LED0:
			GPIO_PinOutToggle(BSP_LED0_PORT, BSP_LED0_PIN);
			gecko_cmd_hardware_set_soft_timer(32768 / 2, TIMER_ID_FIRE_LED1, 0);
			break;

		case TIMER_ID_FIRE_LED1:
			GPIO_PinOutToggle(BSP_LED1_PORT, BSP_LED1_PIN);
			break;

		case TIMER_TIME_DISP: {
			uint8_t hours_2_send;
			uint8_t minutes_2_send;
			hours_2_send = retrieve_hours();
			minutes_2_send = retrieve_minutes();
			sprintf(time_disp, "TIME:%d:%d", hours_2_send, minutes_2_send);
			LCD_write(time_disp, 5);
		}
			break;

		case TIMER_ID_DISPLAY_TIME:
			LCD_write("", 6);
			break;

		case TIMER_ID_FIRE_RESET:
			gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_FIRE_LED0, 1);
			gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_FIRE_LED1, 1);
			LCD_write("", 6);
			LED_set_state(LED_STATE_OFF);
			break;

		default:
			break;
		}
		break;

	case gecko_evt_mesh_node_initialized_id:
		printf("Node Initialized\n\r");
		appkey = 2;
		// Initialize node as generic client
		resnew = gecko_cmd_mesh_generic_client_init()->result;
		if (resnew == 0) {
			printf("Initialized as Client model\n\r");
		}
#ifndef USING_PCB
			LCD_write("f2:6e - Client Init", 2);
			LCD_write("", 6);
#endif
#ifdef servercode
			resnew = gecko_cmd_mesh_generic_server_init()->result;
			if (resnew) {
				printf("Server initialization failed\r\n");
			}
		 else
			printf("Controller Initialization failed 0x%x\n\r", resnew);
#endif
		struct gecko_msg_mesh_node_initialized_evt_t *pData =
				(struct gecko_msg_mesh_node_initialized_evt_t *) &(evt->data);

		if (pData->provisioned) {
			printf("node is provisioned. address:%x, ivi:%ld\r\n",
					pData->address, pData->ivi);

			_my_address = pData->address;
			_elem_index = 0; // index of primary element is zero. This example has only one element.

			friend_node_init();
			LCD_write("provisioned", 3);
		} else {
			printf("node is unprovisioned\r\n");
			LCD_write("unprovisioned", 3);

			// The Node is now initialized, start unprovisioned Beaconing using PB-Adv Bearer
			resnew = gecko_cmd_mesh_node_start_unprov_beaconing(0x03)->result;
			if (resnew == 0) {
				printf("Starting unprovisioned beaconing\n\r");
			} else
				printf("Beaconing failed 0x%x\n\r", resnew);
		}

		gecko_cmd_hardware_set_soft_timer(32768 * 60, TIMER_TIME_DISP, 0);
		break;

	case gecko_evt_mesh_node_provisioning_started_id:
		printf("Provisioning Started\n\r");
//      led_init();
		// start timer for blinking LEDs to indicate which node is being provisioned
		gecko_cmd_hardware_set_soft_timer(32768 / 4, TIMER_ID_PROVISIONING, 0);
		break;

	case gecko_evt_mesh_node_provisioned_id:
		_elem_index = 0;// index of primary element is zero. This example has only one element.
		printf("Provisioned\n\r");
		LCD_write("Provisioned", 3);

		// stop LED blinking when provisioning complete
		gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_PROVISIONING, 0);
		LED_set_state(LED_STATE_OFF);

		friend_node_init();
		break;

	case gecko_evt_mesh_node_provisioning_failed_id:
		printf("Failed in event\n\r");
		break;

	case gecko_evt_mesh_friend_friendship_established_id:
		printf(
				"evt gecko_evt_mesh_friend_friendship_established, lpn_address=%x\r\n",
				evt->data.evt_mesh_friend_friendship_established.lpn_address);
		LCD_write("BEFRIENDED LPN", 4);
		server_lpn_address =
				evt->data.evt_mesh_friend_friendship_established.lpn_address;

		gecko_cmd_hardware_set_soft_timer(32768 * 5, TIMER_30_SEC, 0);//30 seconds continuous running for checking time >< 30 seconds
		gecko_cmd_hardware_set_soft_timer(32768 * 60 * 15, TIMER_15_MIN, 0); // 15 minutes continuous running for checking time (8am/10pm)

		break;

	case gecko_evt_mesh_friend_friendship_terminated_id:
		printf("evt gecko_evt_mesh_friend_friendship_terminated, reason=%x\r\n",
				evt->data.evt_mesh_friend_friendship_terminated.reason);
		LCD_write("NO LPN", 4);

		gecko_cmd_hardware_set_soft_timer(0, TIMER_30_SEC, 1);//stop sending value every 30 seconds if friendship established
		gecko_cmd_hardware_set_soft_timer(0, TIMER_15_MIN, 1);

		break;

	case gecko_evt_mesh_node_display_output_oob_id: {
		struct gecko_msg_mesh_node_display_output_oob_evt_t *dOOB =
				(struct gecko_msg_mesh_node_display_output_oob_evt_t *) &(evt->data);
		printf(
				"gecko_msg_mesh_node_display_output_oob_evt_t : action%d, size%d\r\n",
				dOOB->output_action, dOOB->output_size);
		printf("%2.2x ", dOOB->data.data[dOOB->data.len - 1]);
		sprintf(passkeyOOB, "KEY:%2.2x", dOOB->data.data[dOOB->data.len - 1]);
		LCD_write(passkeyOOB, 6);
		printf("\r\n");
	}
		break;

	case gecko_evt_mesh_node_key_added_id:
		printf("got new %s key with index %x\r\n",
				evt->data.evt_mesh_node_key_added.type == 0 ?
						"network" : "application",
				evt->data.evt_mesh_node_key_added.index);
		appkey =
				evt->data.evt_mesh_node_key_added.type == 0 ?
						0 : evt->data.evt_mesh_node_key_added.index;
		break;

	case gecko_evt_mesh_node_model_config_changed_id:
		printf("model config changed\r\n");

		break;

#ifdef servercode
	case gecko_evt_mesh_generic_server_client_request_id:
		printf("evt gecko_evt_mesh_generic_server_client_request_id\r\n");
		mesh_lib_generic_server_event_handler(evt);
		break;

	case gecko_evt_mesh_generic_server_state_changed_id:

		// uncomment following line to get debug prints for each server state changed event
		//server_state_changed(&(evt->data.evt_mesh_generic_server_state_changed));

		// pass the server state changed event to mesh lib handler that will invoke
		// the callback functions registered by application
		mesh_lib_generic_server_event_handler(evt);
		break;
#endif

	case gecko_evt_mesh_generic_client_server_status_id: {
		struct gecko_msg_mesh_generic_client_server_status_evt_t* lpn_pstatus =
				(struct gecko_msg_mesh_generic_client_server_status_evt_t*) &(evt->data.evt_mesh_generic_client_server_status);
//		uint8_t msg_len = lpn_pstatus->parameters.len;
		uint16_t msg_data = lpn_pstatus->parameters.data[1];
		msg_data = (msg_data << 8) | lpn_pstatus->parameters.data[0];
//		maintain.para_slot_len = lpn_pstatus->parameters.len;
//		maintain.para_slot_data[maintain.para_slot_len - 1] =
//				lpn_pstatus->parameters.data[lpn_pstatus->parameters.len - 1];

		printf("Server status received: %d \n\r", msg_data);

//		ps_save();

//		ps_load();

		if (evt->data.evt_mesh_generic_client_server_status.server_address
				== SERVER_LPN1_ADDRESS) {
			for (int i = 0; i < maintain.para_slot_len; i++)
				printf("Parking slot 1 status: %d\r\n",
						maintain.para_slot_data[i]);
			sprintf(pslot_status, "%s",
					maintain.para_slot_data[maintain.para_slot_len - 1] ?
							"Slot 1 full" : "Slot 1 empty");
			LCD_write(pslot_status, 7);
		}
		if (evt->data.evt_mesh_generic_client_server_status.server_address
				== SERVER_LPN2_ADDRESS) {
			for (int i = 0; i < maintain.para_slot_len; i++)
				printf("Parking slot 2 status: %d\r\n",
						maintain.para_slot_data[i]);
			sprintf(pslot_status, "%s",
					maintain.para_slot_data[maintain.para_slot_len - 1] ?
							"Slot 2 full" : "Slot 2 empty");
			LCD_write(pslot_status, 8);
		}
	}
		break;

	case gecko_evt_le_connection_opened_id:
		printf("Connected\n\r");
		break;

	case gecko_evt_system_external_signal_id: {
		printf("Button pressed\n\r");
		if (evt->data.evt_system_external_signal.extsignals
				& EXT_SIGNAL_PB0_PRESS) {
			handle_button_press(0);
		}

		if (evt->data.evt_system_external_signal.extsignals
				& EXT_SIGNAL_PB1_PRESS) {
			handle_button_press(1);
		}
	}
		break;

	case gecko_evt_le_connection_closed_id:
		/* Check if need to boot to dfu mode */
		if (boot_to_dfu) {
			/* Enter to DFU OTA mode */
			gecko_cmd_system_reset(2);
		}
		printf("evt:conn closed, reason 0x%x\r\n",
				evt->data.evt_le_connection_closed.reason);
		break;

	case gecko_evt_gatt_server_user_write_request_id:
		if (evt->data.evt_gatt_server_user_write_request.characteristic
				== gattdb_ota_control) {
			/* Set flag to enter to OTA mode */
			boot_to_dfu = 1;
			/* Send response to Write Request */
			gecko_cmd_gatt_server_send_user_write_response(
					evt->data.evt_gatt_server_user_write_request.connection,
					gattdb_ota_control, bg_err_success);

			/* Close connection to enter to DFU OTA mode */
			gecko_cmd_le_connection_close(
					evt->data.evt_gatt_server_user_write_request.connection);
		}
		break;
	default:
		break;
	}
}
