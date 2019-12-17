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

/**************************************************************************************************
 * This code and all its related files in src folder are written by Sarthak Jain, Hardik Senjaliya
 * and Vatsal Sheth for the course of Low Power Embedded Design Techniques final project, Bhishma.
 * Due reference is given to the Silicon Labs btmesh-empty example codes, the framework of which was
 * used for designing our code.
 *
 */

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
#include "src/STPM34.h"

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
#define TIMER_ID_CLIENT_GET (44)

#define GET_AC_READINGS			(110)
//#define UINT16_MAX				(65536)

#define SERVER_LPN1_ADDRESS	(0x02)
#define SERVER_LPN2_ADDRESS	(0x09)
#define CLIENT_SERVER_ADDRESS (0x04)

//#define APPKEY_INDEX (2)


/*Global Variables*/
static uint16 _elem_index = 0xffff; /* For indexing elements of the node (this example has only one element)*/
static uint16 _my_address = 0; /* Address of the Primary Element of the Node */
static uint8 switch_pos = 0; /*current position of switch*/
static uint8 trid = 0; /* transaction identifier */
static uint8 request_count; /* number of on/off requests to be sent */

uint8_t server_lpn_address;
uint8_t appkey;
uint16_t a;

static PACKSTRUCT(struct lightbulb_state
{
	// On/Off Server state
	uint8_t onoff_current;
	uint8_t onoff_target;

	// Transition Time Server state
	uint8_t transtime;

	// On Power Up Server state
	uint8_t onpowerup;

	// Primary Generic Level
	uint16_t pri_level_current;
	uint16_t pri_level_target;

}) lightbulb_state;

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

const gecko_configuration_t config =
{
		.sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
		.bluetooth.max_connections = MAX_CONNECTIONS,
		.bluetooth.max_advertisers = MAX_ADVERTISERS,
		.bluetooth.heap = bluetooth_stack_heap,
		.bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
		.bluetooth.sleep_clock_accuracy = 100,
		.gattdb = &bg_gattdb_data,
		.btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.pa.config_enable = 1, // Enable high power PA
		.pa.input = GECKO_RADIO_PA_INPUT_VBAT,// Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.max_timers = 16,
};

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
void mesh_native_bgapi_init(void);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

#ifndef testing
char passkeyOOB[20];
uint16_t resnew;
char pslot_status[20];
static uint8 conn_handle = 0xFF; /* handle of the last opened LE connection */
#endif


void initiate_factory_reset(void)
{
	printf("factory reset\r\n");
#ifndef USING_PCB
	LCD_write("\n***\nFACTORY RESET\n***", LCD_ROW_CONNECTION);
#endif
	/* if connection is open then close it before rebooting */
	if (conn_handle != 0xFF)
	{
		gecko_cmd_le_connection_close(conn_handle);
	}

	/* perform a factory reset by erasing PS storage. This removes all the keys and other settings
	 that have been configured for this node */
	gecko_cmd_flash_ps_erase_all();

//   reboot after a small delay
	gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}


//#ifndef USING_PCB
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

	printf("Data sent: %d \r\n", target.level.level);

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
	if (e == bg_err_success)
	{
		e = mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                                        	element_index,
											mesh_generic_state_level);
	}

	if (e)
	{
		printf("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n", e);
	}
	else
	{
//		printf("request sent"/*, trid*/);
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

	if (request->kind != mesh_generic_request_level)
	{
		return;
	}

	printf("pri_level_request: level=%d, transition=%lu, delay=%u\r\n",
			request->level, transition_ms, delay_ms);

	if (lightbulb_state.pri_level_current == request->level)
	{
		printf("Request for current state received; no op\n");
	}
	else
	{
		printf("Setting pri_level to <%d>\r\n", request->level);

		lightness = request->level + 32768;

		if (transition_ms == 0 && delay_ms == 0)
		{ // Immediate change
			lightbulb_state.pri_level_current = request->level;
			lightbulb_state.pri_level_target = request->level;

		}
		else if (delay_ms > 0)
		{
			// a delay has been specified for the light change. Start a soft timer
			// that will trigger the change after the given delay
			// Current state remains as is for now
			lightbulb_state.pri_level_target = request->level;
//			gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(delay_ms), TIMER_ID_DELAYED_PRI_LEVEL, 1);
			// store transition parameter for later use
		}
		else
		{
			// no delay but transition time has been set.
			lightbulb_state.pri_level_target = request->level;

			// lightbulb current state will be updated when transition is complete
//      	gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(transition_ms), TIMER_ID_PRI_LEVEL_TRANSITION, 1);
		}
	}

	if (request_flags & MESH_REQUEST_FLAG_RESPONSE_REQUIRED)
	{
		pri_level_response(element_index, client_addr, appkey_index);
	}
	else
	{
		pri_level_update(element_index);
	}
}

static void pri_level_change(uint16_t model_id,
                             uint16_t element_index,
                             const struct mesh_generic_state *current,
                             const struct mesh_generic_state *target,
                             uint32_t remaining_ms)
{
	if (lightbulb_state.pri_level_current != current->level.level)
	{
		printf("pri_level_change: from %d to %d\r\n", lightbulb_state.pri_level_current, current->level.level);
		lightbulb_state.pri_level_current = current->level.level;
	}
	else
	{
		printf("pri_level update -same value (%d)\r\n", lightbulb_state.pri_level_current);
	}
}


/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr)
{
	char name[20];
	// create unique device name using the bytes of the Bluetooth address
	sprintf(name, "%x:%x:%x:%x:%x:%x", pAddr->addr[5], pAddr->addr[4], pAddr->addr[3],
									   pAddr->addr[2], pAddr->addr[1], pAddr->addr[0]);

	printf("Device name: '%s'\r\n", name);

	resnew = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0,
														 strlen(name), (uint8 *) name)->result;
	if (resnew)
	{
		printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", resnew);
	}
}


///**
// * Initialization of the models supported by this node. This function registers callbacks for
// * each of the three supported models.
// */
static void init_models(void)
{
	mesh_lib_generic_server_register_handler(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
                                           	 0,
											 pri_level_request,
											 pri_level_change);
}


/**
 * Initialize LPN functionality with configuration and friendship establishment.
 */
void lpn_init(void)
{
	mesh_lib_init(malloc, free, 8);
	uint16 res;
	// Initialize LPN functionality.
	res = gecko_cmd_mesh_lpn_init()->result;
	if (res)
	{
		printf("LPN init failed (0x%x)\r\n", res);
		return;
	}
	else printf("LPN initialized\r\n");

	res = gecko_cmd_mesh_lpn_configure(2, 10 * 1000)->result;
	if (res)
	{
		printf("LPN conf failed (0x%x)\r\n", res);
		return;
	}

	printf("trying to find friend...\r\n");
	res = gecko_cmd_mesh_lpn_establish_friendship(0)->result;

	if (res != 0)
	{
		printf("ret.code %x\r\n", res);
	}
	init_models();
}


int main()
{
	// Initialize device
	initMcu();
	// Initialize board
	initBoard();
	// Initialize application
	initApp();

	// Initialization of self-defined functions
	cmu_init();
	stpm34_init();
//	gpio_init();
//	clock_output();

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
//	gecko_bgapi_class_mesh_proxy_init();
//	gecko_bgapi_class_mesh_proxy_server_init();
//	gecko_bgapi_class_mesh_proxy_client_init();
	gecko_bgapi_class_mesh_generic_client_init();
#ifdef servercode
	gecko_bgapi_class_mesh_generic_server_init();
#endif
	gecko_bgapi_class_mesh_lpn_init();
	gecko_bgapi_class_mesh_friend_init();

	mesh_native_bgapi_init();
	gecko_initCoexHAL();

	RETARGET_SerialInit();
	RETARGET_SerialCrLf(true);

	while (1)
	{
		struct gecko_cmd_packet *evt = gecko_wait_event();
		bool pass = mesh_bgapi_listener(evt);
		if (pass)
		{
			handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
		}
	}
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
	switch (evt_id)
	{
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
			break;

		case gecko_evt_hardware_soft_timer_id:
			switch (evt->data.evt_hardware_soft_timer.handle)
			{
				case TIMER_ID_FACTORY_RESET:
					gecko_cmd_system_reset(0);
					break;

				case TIMER_ID_RESTART:
					gecko_cmd_system_reset(0);
					break;

				case TIMER_ID_PROVISIONING:
//					LED_set_state(LED_STATE_PROV);
					break;

				case TIMER_ID_RETRANS:
			// stop retransmission timer if it was the last attempt
					if (request_count == 0)
					{
						gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_RETRANS, 0);
					}
					break;

				case GET_AC_READINGS:
					lightbulb_state.pri_level_target = Get_Current();
					pri_level_update_and_publish(_elem_index);
					lightbulb_state.pri_level_target = Get_Voltage();
					pri_level_update_and_publish(_elem_index);
					lightbulb_state.pri_level_target = Get_Power();
					pri_level_update_and_publish(_elem_index);
					break;

				default:
					break;
			}
			break;

		case gecko_evt_mesh_node_initialized_id:
			printf("Node Initialized\n\r");
			appkey = 2;
			// Initialize node as generic server
			resnew = gecko_cmd_mesh_generic_server_init()->result;
			if (resnew == 0)
			{
				printf("Initialized as Server model\n\r");
			}
#ifdef servercode
			resnew = gecko_cmd_mesh_generic_server_init()->result;
			if (resnew)
			{
				printf("Server initialization failed\r\n");
			}
			else
				printf("Controller Initialization failed 0x%x\n\r", resnew);
#endif
			struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *) &(evt->data);

			if (pData->provisioned)
			{
				printf("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);

				_my_address = pData->address;
				_elem_index = 0; // index of primary element is zero. This example has only one element.

				lpn_init();
			}
			else
			{
				printf("node is unprovisioned\r\n");

				// The Node is now initialized, start unprovisioned Beaconing using PB-Adv Bearer
				resnew = gecko_cmd_mesh_node_start_unprov_beaconing(0x03)->result;
				if (resnew == 0)
				{
					printf("Starting unprovisioned beaconing\n\r");
				}
				else
					printf("Beaconing failed 0x%x\n\r", resnew);
			}
			break;

		case gecko_evt_mesh_node_provisioning_started_id:
			printf("Provisioning Started\n\r");
			break;

		case gecko_evt_mesh_node_provisioned_id:
			_elem_index = 0;// index of primary element is zero. This example has only one element.
			printf("Provisioned\n\r");
			lpn_init();
			break;

		case gecko_evt_mesh_node_provisioning_failed_id:
			printf("Failed in event\n\r");
			break;

		case gecko_evt_mesh_lpn_friendship_established_id:
			printf("evt gecko_evt_mesh_lpn_friendship_established, friend_address=%x\r\n",
					evt->data.evt_mesh_lpn_friendship_established.friend_address);
			gecko_cmd_hardware_set_soft_timer(4*32768, GET_AC_READINGS, 0);
			break;

		case gecko_evt_mesh_friend_friendship_terminated_id:
			printf("evt gecko_evt_mesh_friend_friendship_terminated, reason=%x\r\n",
					evt->data.evt_mesh_friend_friendship_terminated.reason);
			break;

		case gecko_evt_mesh_node_display_output_oob_id:
		{
			struct gecko_msg_mesh_node_display_output_oob_evt_t *dOOB =
				(struct gecko_msg_mesh_node_display_output_oob_evt_t *) &(evt->data);
			printf("gecko_msg_mesh_node_display_output_oob_evt_t : action%d, size%d\r\n",
					dOOB->output_action, dOOB->output_size);
			printf("%2.2x ", dOOB->data.data[dOOB->data.len - 1]);
			sprintf(passkeyOOB, "KEY:%2.2x", dOOB->data.data[dOOB->data.len - 1]);
			printf("\r\n");
		}
		break;

		case gecko_evt_mesh_node_key_added_id:
			printf("got new %s key with index %x\r\n",
					evt->data.evt_mesh_node_key_added.type == 0 ?
					"network" : "application",
					evt->data.evt_mesh_node_key_added.index);
			appkey = evt->data.evt_mesh_node_key_added.type == 0 ? 0 : evt->data.evt_mesh_node_key_added.index;
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


		case gecko_evt_le_connection_opened_id:
			printf("Connected\n\r");
			break;

		case gecko_evt_system_external_signal_id:
			printf("Button pressed\n\r");
			if (evt->data.evt_system_external_signal.extsignals
					& EXT_SIGNAL_PB0_PRESS)
			{
				//			handle_button_press(0);
			}

			if (evt->data.evt_system_external_signal.extsignals
					& EXT_SIGNAL_PB1_PRESS)
			{
				//			handle_button_press(1);
			}
			break;

		case gecko_evt_le_connection_closed_id:
			/* Check if need to boot to dfu mode */
			if (boot_to_dfu)
			{
			/* Enter to DFU OTA mode */
				gecko_cmd_system_reset(2);
			}
			printf("evt:conn closed, reason 0x%x\r\n",
					evt->data.evt_le_connection_closed.reason);
			break;

		case gecko_evt_gatt_server_user_write_request_id:
			if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control)
			{
				/* Set flag to enter to OTA mode */
				boot_to_dfu = 1;
				/* Send response to Write Request */
				gecko_cmd_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
															gattdb_ota_control, bg_err_success);

			/* Close connection to enter to DFU OTA mode */
				gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
			}
			break;
		default:
			break;
	}
}
