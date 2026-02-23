#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libopenthread/platform/openthread-system.h>
#include <libopenthread/platform/plat.h>
#include <openthread/coap.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/udp.h>

#include <libtock/tock.h>
#include <libtock/crypto/isle.h>
#include <libtock-sync/services/alarm.h>
#include <libtock/kernel/ipc.h>
#include <libtock/interface/button.h>
#include <libtock/interface/led.h>
#include <libtock/peripherals/gpio.h>
#include <libtock/services/alarm.h>

#include "coap.h"

#define MAX_PAYLOAD_LEN ((uint32_t) 79)
#define EXP_DEST_ADDR "fd74:42e:17e:e1ae:fe5a:60cc:c7de:7a6f"

uint8_t _g_payload_buffer[MAX_PAYLOAD_LEN];
static bool g_connected;
static otInstance* g_ot_instance;
static uint16_t __g_coap_message_id = 0;
static uint32_t __g_coap_token = 0;

// helper utility demonstrating network config setup
static void setNetworkConfiguration(otInstance* aInstance);

static otUdpSocket sUdpSocket;

void initUdp(otInstance* aInstance);

void handleUdpRecvTemperature(void* aContext, otMessage* aMessage,
                              const otMessageInfo* aMessageInfo);
void handle_coap_message(
	void* context,
	otMessage* msg,
	const otMessageInfo* msg_info);

void sendUdpTemperature(otInstance* aInstance);

void announce_ip_address(void);

static void __send_test_packet(void);
static void __recv_test_packet(void);
static void __recv_test_callback(int, int, int, void*);

#define BUTTON_COUNT ((uint8_t) 4)

static void __on_button_press(
	__attribute__ ((unused)) returncode_t rc,
	int button_no,
	bool pressed);

// callback for Thread state change events
static void stateChangeCallback(uint32_t flags, void* context);

// helper utility to print ip address
static void print_ip_addr(otInstance* instance);

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
	for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
		libtock_button_notify_on_press(i, __on_button_press);
	}

	// Set up GPIO.
	libtock_gpio_enable_output(0);

  // Initialize OpenThread instance.
  otSysInit(argc, argv);
  otInstance* instance;
  instance = otInstanceInitSingle();
  g_ot_instance = instance;
  assert(instance);

  // set child timeout to 60 seconds.
  otThreadSetChildTimeout(instance, 60);

  // Set callback to be notified when thread state changes.
  otSetStateChangedCallback(instance, stateChangeCallback, instance);

  ///////////////////////////////////////////////////
  // THREAD NETWORK SETUP HERE

  // Configure network.
  setNetworkConfiguration(instance);

  // Init UDP interface.
  initUdp(instance);

  // Enable network interface.
  while (otIp6SetEnabled(instance, true) != OT_ERROR_NONE) {
    printf("Failed to start Thread network interface!\n");
    libtocksync_alarm_delay_ms(100);
  }

  // Print IPv6 address.
  print_ip_addr(instance);

  // Start Thread network.
  while (otThreadSetEnabled(instance, true) != OT_ERROR_NONE) {
    printf("Failed to start Thread stack!\n");
    libtocksync_alarm_delay_ms(100);
  }

  uint32_t counter_freq;
  uint32_t last_announce;
  libtock_alarm_command_get_frequency(&counter_freq);
  libtock_alarm_command_read(&last_announce);
  printf("counter freq.: %ld\n", counter_freq);

  //
  ////////////////////////////////////////////////////

  // OpenThread main loop.
  for (;;) {
    // Execute any pending OpenThread related work.
    otTaskletsProcess(instance);

    // Execute any platform related work (e.g. check
    // radio buffer for new packets).
    otSysProcessDrivers(instance);

    // If there is not pending platform or OpenThread
    // related work -- yield.
	const bool is_tasklet_pending = otTaskletsArePending(instance);
	const bool is_openthread_pending = openthread_platform_pending_work();
	const bool is_work_pending = is_tasklet_pending || is_openthread_pending;
    if (!is_work_pending) {
		/* uint32_t now; */
		/* libtock_alarm_command_read(&now); */
		/* if (g_connected && (now - last_announce) > (counter_freq * 10)) { */
		/* 	libtock_alarm_command_read(&last_announce); */
		/* 	printf("Announcing address.\n"); */
		/* 	announce_ip_address(); */
		/* } else { */
		/* 	yield(); */
		/* } */
		yield();
    }
  }

  return 0;
}

// Helper method that configures the OpenThread network dataset
// for the desired tutorial configuration.
// We set the following dataset parameters:
//  -- Channel:    26
//  -- PanId:      0xabcd
//  -- Networkkey: 00112233445566778899aabbccddeeff
/*
Active Timestamp: 0
Channel: 26
Channel Mask: 0x07fff800
Ext PAN ID: b7e699aefab75ba4
Mesh Local Prefix: fd74:42e:17e:e1ae::/64
Network Key: 84fb54cc428a595911a263d5caed2456
Network Name: WIoT
PAN ID: 0x4501
PSKc: b6815b13690e51189a736edcc18e5653
Security Policy: 672 onrcp 0
 */
void setNetworkConfiguration(otInstance* aInstance) {
  otOperationalDataset aDataset;

  memset(&aDataset, 0, sizeof(otOperationalDataset));

  /* Set Channel to 26 */
  /* aDataset.mChannel = 20; */
  aDataset.mChannel = 26;
  aDataset.mComponents.mIsChannelPresent = true;
  aDataset.mChannelMask = 0x07fff800;
  aDataset.mComponents.mIsChannelMaskPresent = true;

  /* Set Pan ID to abcd */
  /* aDataset.mPanId = (otPanId)0xabcd; */
  aDataset.mPanId = (otPanId) 0x4501;
  aDataset.mComponents.mIsPanIdPresent = true;
  uint8_t extpanid[] = {
	  0x45, 0x01, 0xc5, 0xee, 0x45, 0x01, 0xc5, 0xee
  };
  memcpy(
	  aDataset.mExtendedPanId.m8,
	  extpanid,
	  sizeof(extpanid));
  aDataset.mComponents.mIsExtendedPanIdPresent = true;

  uint8_t pskc[] = {
	  0x22, 0xd2, 0x48, 0x61, 0xd6, 0x2a, 0xd7, 0x39, 0xbc, 0x63, 0x54, 0x24, 0xb3, 0xe2, 0x43, 0xf8
  };
  memcpy(
	  aDataset.mPskc.m8,
	  pskc,
	  sizeof(pskc));
  aDataset.mComponents.mIsPskcPresent = true;

  /* Set network key to 00112233445566778899aabbccddeeff */
  /* uint8_t key[OT_NETWORK_KEY_SIZE] = { */
  /* 	  0x00, 0x11, 0x22, 0x33, */
  /* 	  0x44, 0x55, 0x66, 0x77, */
  /* 	  0x88, 0x99, 0xaa, 0xbb, */
  /* 	  0xcc, 0xdd, 0xee, 0xff */
  /* }; */
  uint8_t key[OT_NETWORK_KEY_SIZE] = {
	  0x84, 0xFB, 0x54, 0xCC,
	  0x42, 0x8A, 0x59, 0x59,
	  0x11, 0xA2, 0x63, 0xD5,
	  0xCA, 0xED, 0x24, 0x56
  };
  memcpy(aDataset.mNetworkKey.m8, key, sizeof(aDataset.mNetworkKey));
  aDataset.mComponents.mIsNetworkKeyPresent = true;

  aDataset.mNetworkName.m8[0] = 'W';
  aDataset.mNetworkName.m8[0] = 'I';
  aDataset.mNetworkName.m8[0] = 'o';
  aDataset.mNetworkName.m8[0] = 'T';
  aDataset.mNetworkName.m8[0] = 0x00;
  aDataset.mComponents.mIsNetworkNamePresent = true;

  otError error = otDatasetSetActive(aInstance, &aDataset);
  assert(error == 0);
}

// Helper method that registers a stateChangeCallback to print
// when state changes occur (useful for debugging).
static void stateChangeCallback(uint32_t flags, void* context) {
	otError oerr;
	otInstance* instance = (otInstance*)context;
	const uint8_t stable_addr_upper[] = { 0xfd, 0x74, 0x04, 0x2e, 0x01, 0x7e, 0xe1, 0xae };

	if (!(flags & OT_CHANGED_THREAD_ROLE)) {
		return;
	}

	g_connected = false;

	switch (otThreadGetDeviceRole(instance)) {
    case OT_DEVICE_ROLE_DISABLED:
		printf("[State Change] - Disabled.\n");
		break;
    case OT_DEVICE_ROLE_DETACHED:
		printf("[State Change] - Detached.\n");
		break;
    case OT_DEVICE_ROLE_CHILD:
		g_connected = true;
		printf("[State Change] - Child.\n");
		printf("Successfully attached to Thread network as a child.\n");
		print_ip_addr(instance);
		break;
    case OT_DEVICE_ROLE_ROUTER:
		printf("[State Change] - Router.\n");
		break;
    case OT_DEVICE_ROLE_LEADER:
		printf("[State Change] - Leader.\n");
		break;
    default:
		break;
	}

	if (g_connected) {
		libtock_led_on(0);
	} else {
		libtock_led_off(0);
	}

	return;
}

// Helper method to print the given Thread node's registered
// ipv6 address.
static void print_ip_addr(otInstance* instance) {
  char addr_string[64];
  const otNetifAddress* unicastAddrs = otIp6GetUnicastAddresses(instance);

  printf("[THREAD] Device IPv6 Addresses: ");
  for (const otNetifAddress* addr = unicastAddrs; addr; addr = addr->mNext) {
    const otIp6Address ip6_addr = addr->mAddress;
    otIp6AddressToString(&ip6_addr, addr_string, sizeof(addr_string));
    printf("%s\n", addr_string);
  }
}


void handleUdpRecvTemperature(void* aContext, otMessage* aMessage,
                              const otMessageInfo* aMessageInfo) {
  OT_UNUSED_VARIABLE(aContext);
  OT_UNUSED_VARIABLE(aMessageInfo);
  char buf[2];

  const otIp6Address sender_addr = aMessageInfo->mPeerAddr;
  otIp6AddressToString(&sender_addr, buf, sizeof(buf));

  otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
  printf("Received UDP Packet: %d\r\n", buf[0]);
}


void handle_coap_message(
	void* context,
	otMessage* request_msg,
	const otMessageInfo* msg_info)
{
	otInstance* const ot_instance = (otInstance*) context;

	// Flash the LED.
	libtock_led_on(2);
    libtocksync_alarm_delay_ms(50);
	libtock_led_off(2);

	// Get the message.
	const uint16_t payload_len = otMessageRead(
		request_msg,
		otMessageGetOffset(request_msg),
		_g_payload_buffer,
		MAX_PAYLOAD_LEN);

	const otIp6Address sender_addr = msg_info->mPeerAddr;
	char sender_addr_str[OT_IP6_ADDRESS_STRING_SIZE];
	otIp6AddressToString(
		&sender_addr,
		sender_addr_str,
		OT_IP6_ADDRESS_STRING_SIZE);
	printf("Received packet from %s.\n", sender_addr_str);
	return;

	// Determine that it is COAP and the kind of COAP message it is,
	// and respond appropriately.

	// Read the header with in-place checks.
	if (payload_len < 4) { return; }
	if ((_g_payload_buffer[0] & 0x03) != 0x01) { return; }

	otError error = OT_ERROR_NONE;
	otMessage* response_msg;
	otMessageInfo response_msg_info;

	memset(&response_msg_info, 0, sizeof(response_msg_info));

	response_msg_info.mPeerAddr = sender_addr;
	response_msg_info.mPeerPort = COAP_PORT_NO;

	/* response_msg = otUdpNewMessage(ot_instance, NULL); */
	response_msg = otCoapNewMessage(ot_instance, NULL);
	if (response_msg == NULL) {
		printf("Error creating CoAP response\n");
		return;
	}

	error = otCoapMessageInitResponse(
		response_msg,
		request_msg,
		OT_COAP_TYPE_NON_CONFIRMABLE,
		OT_COAP_CODE_NOT_IMPLEMENTED);
	if (error != OT_ERROR_NONE) {
		printf("Error initializing CoAP response\n");
		return;
	}

	error = otUdpSend(
		ot_instance,
		&sUdpSocket,
		response_msg,
		&response_msg_info);
	if (error != OT_ERROR_NONE && response_msg != NULL) {
		printf("Error sending udp packet\n");
		otMessageFree(response_msg);
	}

	return;
}

void initUdp(otInstance* aInstance) {
  otSockAddr listenSockAddr;

  memset(&sUdpSocket, 0, sizeof(sUdpSocket));
  memset(&listenSockAddr, 0, sizeof(listenSockAddr));

  listenSockAddr.mPort = 5683;

  otUdpOpen(aInstance, &sUdpSocket, handle_coap_message, aInstance);
  otUdpBind(aInstance, &sUdpSocket, &listenSockAddr, OT_NETIF_THREAD);
}

const uint8_t coap_payload[] = {
	0b01000001,
	0b01000101,
	// Message ID
    0xFA,
	0xFE,
	// Token
	 0x00, 0x01, 0x2A, 0x5E,
	// Payload marker
	0xFF,
	// Payload
	0x11, 0x22, 0xA7, 0xB3,
};

void sendUdpTemperature(otInstance* aInstance) {

  otError error = OT_ERROR_NONE;
  otMessage*   message;
  otMessageInfo messageInfo;
  otIp6Address destinationAddr;

  memset(&messageInfo, 0, sizeof(messageInfo));

  otIp6AddressFromString(EXP_DEST_ADDR, &destinationAddr);
  messageInfo.mPeerAddr = destinationAddr;
  messageInfo.mPeerPort = 1212;

  message = otUdpNewMessage(aInstance, NULL);
  if (message == NULL) {
    printf("Error creating udp message\n");
    return;
  }

  error = otMessageAppend(message, &coap_payload, sizeof(coap_payload));
  if (error != OT_ERROR_NONE && message != NULL) {
    printf("Error appending to udp message\n");
    otMessageFree(message);
    return;
  }

  libtock_gpio_set(0);
  error = otUdpSend(aInstance, &sUdpSocket, message, &messageInfo);
  libtock_gpio_clear(0);

  if (error != OT_ERROR_NONE && message != NULL) {
    printf("Error sending udp packet\n");
    otMessageFree(message);
  }
}

static void __on_button_press(
	__attribute__ ((unused)) returncode_t rc,
	int button_no,
	bool pressed)
{

	if (pressed) {
		printf("button %d pressed\n", button_no);

		switch(button_no) {
		case 0:
			__send_test_packet();
			/* __recv_test_packet(); */
			break;
		case 1:
			sendUdpTemperature(g_ot_instance);
			break;
		/* case 1: */
		/* 	otInstanceErasePersistentInfo(g_ot_instance); */
		/* 	g_connected = false; */
		/* 	while (true) {  } */
		/* 	break; */
		case 3:
			/* printf("Total active time: %ld ms\n", */
			/* 	   (uint32_t) (((float) bbb) / ((float) 32.768))); */
			break;
		default:
			break;
		}
	}
}

/* static uint16_t __g_coap_message_id = 0x128F; */
/* static uint32_t __g_coap_token = 0x5228ABCD; */

static void __send_test_packet(void)
{
	otError error = OT_ERROR_NONE;
	otMessage* msg;
	otMessageInfo msg_info;
	otIp6Address dst_addr;

	if (!g_connected) {
		printf("Cannot send. Not connected yet...\n");
		return;
	}

	printf("Initiating send.\n");;

	printf("Original CoAP message size: %d B\n", sizeof(coap_payload));

	if (sizeof(coap_payload) > MAX_PAYLOAD_LEN) {
		printf("Payload too large to send (%d B > %ld B).\n",
			   sizeof(coap_payload),
			   MAX_PAYLOAD_LEN);
		return;
	}

	/* if (!g_connected) { */
	/* 	return; */
	/* } */

	memset(&msg_info, 0, sizeof(msg_info));

	otIp6AddressFromString(
		EXP_DEST_ADDR,
		&dst_addr);
	msg_info.mPeerAddr = dst_addr;
	msg_info.mPeerPort = 5683;

	msg = otUdpNewMessage(
		g_ot_instance,
		NULL);
	if (msg == NULL) {
		printf("Error creating UDP message.\n");
		return;
	}

	error = otMessageAppend(msg, coap_payload, sizeof(coap_payload));
	if (error != OT_ERROR_NONE) {
		printf("Error building message.\n");
		otMessageFree(msg);
		return;
	}

	/* libtock_gpio_toggle(0); */
	error = otUdpSend(
		g_ot_instance,
		&sUdpSocket,
		msg,
		&msg_info);
	if (error != OT_ERROR_NONE) {
		printf("Error sending UDP packet: %d\n", error);
		otMessageFree(msg);
		return;
	} else {
		__g_coap_message_id++;
		__g_coap_token++;
	}
	/* libtock_gpio_toggle(0); */

	return;
}

uint8_t __recv_ct_buffer[32];
uint8_t __recv_pt_buffer[40];
uint8_t __recv_piv_buffer[4];
uint8_t __recv_src_buffer[8];
bool __recv_waiting;

void __recv_test_packet(void)
{
	// Share the buffers with the OS.
	libtock_isle_allow_ro_set_in_buffer(
		__recv_ct_buffer,
		sizeof(__recv_ct_buffer));
	libtock_isle_allow_rw_set_out_buffer(
		__recv_pt_buffer,
		sizeof(__recv_pt_buffer));
	libtock_isle_allow_ro_set_piv_buffer(
		__recv_piv_buffer);
	libtock_isle_allow_ro_set_srchost_buffer(
		__recv_src_buffer);

	// Set the callback.
	libtock_isle_subscribe_out_message_ready(
		__recv_test_callback);

	// Call for decryption.
	returncode_t rc = libtock_isle_command_decrypt(
		0xFEFEFEFEABABABAB);
	printf("OS decrypt_recv: %d\n", rc);

	// Wait for the OS to report being finished.
	__recv_waiting = true;
	while (__recv_waiting) {
		yield();
	}

	// Get buffers back from the OS.
	libtock_isle_allow_ro_set_in_buffer(NULL, 0);
	libtock_isle_allow_rw_set_out_buffer(NULL, 0);
	libtock_isle_allow_ro_set_piv_buffer(NULL);
	libtock_isle_allow_ro_set_srchost_buffer(NULL);

	return;
}

void __recv_test_callback(
	int result,
	int message_len,
	int arg3,
	void* data)
{
	__recv_waiting = false;
	return;
}

void announce_ip_address(void)
{
	otError error = OT_ERROR_NONE;
	otMessage*   message;
	otMessageInfo messageInfo;
	otIp6Address destinationAddr;

	char addr_string[64];
	const otNetifAddress* unicastAddrs = otIp6GetUnicastAddresses(g_ot_instance);

	printf("[THREAD] Device IPv6 Addresses: ");
	for (const otNetifAddress* addr = unicastAddrs; addr; addr = addr->mNext) {
		uint8_t addr_str_len = 0;
		const otIp6Address ip6_addr = addr->mAddress;
		otIp6AddressToString(&ip6_addr, addr_string, sizeof(addr_string));
		while (addr_string[addr_str_len++] != '\0');

		memset(&messageInfo, 0, sizeof(messageInfo));

		otIp6AddressFromString("ff03::02", &destinationAddr);
		messageInfo.mPeerAddr = destinationAddr;
		messageInfo.mPeerPort = 1212;
		message = otUdpNewMessage(g_ot_instance, NULL);
		if (message == NULL) {
			printf("Error creating udp message\n");
			return;
		}

		error = otMessageAppend(message, &addr_string, addr_str_len);
		if (error != OT_ERROR_NONE && message != NULL) {
			printf("Error appending to udp message\n");
			otMessageFree(message);
			return;
		}

		error = otUdpSend(g_ot_instance, &sUdpSocket, message, &messageInfo);
		if (error != OT_ERROR_NONE && message != NULL) {
			printf("Error sending udp packet\n");
			otMessageFree(message);
		}
	}

	// Flash the LED.
	libtock_led_on(1);
    libtocksync_alarm_delay_ms(50);
	libtock_led_off(1);

	return;
}
