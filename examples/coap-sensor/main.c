#include <assert.h>
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

#include <libtock-sync/services/alarm.h>
#include <libtock/kernel/ipc.h>
#include <libtock/services/alarm.h>
#include <libtock/tock.h>

#include "coap.h"

#define MAX_PAYLOAD_LEN (79)

uint8_t _g_payload_buffer[MAX_PAYLOAD_LEN];

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

void sendUdpTemperature(otInstance* aInstance, uint8_t temperature);

// callback for Thread state change events
static void stateChangeCallback(uint32_t flags, void* context);

// helper utility to print ip address
static void print_ip_addr(otInstance* instance);

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
  // Initialize OpenThread instance.
  otSysInit(argc, argv);
  otInstance* instance;
  instance = otInstanceInitSingle();
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
void setNetworkConfiguration(otInstance* aInstance) {
  otOperationalDataset aDataset;

  memset(&aDataset, 0, sizeof(otOperationalDataset));

  /* Set Channel to 26 */
  aDataset.mChannel = 26;
  aDataset.mComponents.mIsChannelPresent = true;

  /* Set Pan ID to abcd */
  aDataset.mPanId = (otPanId)0xabcd;
  aDataset.mComponents.mIsPanIdPresent = true;

  /* Set network key to 00112233445566778899aabbccddeeff */
  uint8_t key[OT_NETWORK_KEY_SIZE] = {
	  0x00, 0x11, 0x22, 0x33,
	  0x44, 0x55, 0x66, 0x77,
	  0x88, 0x99, 0xaa, 0xbb,
	  0xcc, 0xdd, 0xee, 0xff
  };
  memcpy(aDataset.mNetworkKey.m8, key, sizeof(aDataset.mNetworkKey));
  aDataset.mComponents.mIsNetworkKeyPresent = true;

  otError error = otDatasetSetActive(aInstance, &aDataset);
  assert(error == 0);
}

// Helper method that registers a stateChangeCallback to print
// when state changes occur (useful for debugging).
static void stateChangeCallback(uint32_t flags, void* context) {
  otInstance* instance = (otInstance*)context;
  if (!(flags & OT_CHANGED_THREAD_ROLE)) {
    return;
  }

  switch (otThreadGetDeviceRole(instance)) {
    case OT_DEVICE_ROLE_DISABLED:
      printf("[State Change] - Disabled.\n");
      break;
    case OT_DEVICE_ROLE_DETACHED:
      printf("[State Change] - Detached.\n");
      break;
    case OT_DEVICE_ROLE_CHILD:
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

	// Determine that it is COAP and the kind of COAP message it is,
	// and respond appropriately.

	// Read the header with in-place checks.
	if (payload_len < 4) { return; }
	if ((_g_payload_buffer[0] & 0x03) != 0x01) { return; }

	/* const bool hd_confirmable = (_g_payload_buffer[0] & (1 << 2)) != 0; */
	/* const bool hd_nonconfirmable = (_g_payload_buffer[0] & (1 << 3)) != 0; */
	/* const uint8_t hd_token_len = (_g_payload_buffer[0] & 0xF0) >> 4; */
	/* const uint8_t hd_code_class = _g_payload_buffer[1] >> 5; */
	/* const uint8_t hd_code_detail = _g_payload_buffer[1] & 0x1F; */
	/* const uint16_t hd_message_id = *((uint16_t*) _g_payload_buffer + 2); */

	/* printf("===== COAP message\n"); */
	/* printf("CON: %c, NONCON: %c\n", */
	/* 	   hd_confirmable ? 'Y' : 'N', */
	/* 	   hd_nonconfirmable ? 'Y' : 'N'); */
	/* printf("TKL: %i\n", hd_token_len); */
	/* printf("Code: %d.%02d\n", hd_code_class, hd_code_detail); */
	/* printf("Message ID: %04x\n", hd_message_id); */

	/* // Don't respond if non-confirmable. */
	/* if (hd_nonconfirmable) { return; } */

	/* _g_payload_buffer[0] = */
	/* 	// VERSION */
	/* 	(1) */
	/* 	// TYPE (not confirmable, unconfirmable) */
	/* 	| (1 | (1 << 1)) << 2 */
	/* 	// TOKEN LENGTH (same as received) */
	/* 	| hd_token_len << 4; */

	/* // RESPONSE CODE */
	/* _g_payload_buffer[1] = */
	/* 	// CLASS */
	/* 	COAP_RESPONSE_CODE_CLASS_CLIENT_ERROR */
	/* 	| COAP_RESPONSE_CODE_DETAIL_NOT_FOUND; */

	/* // MESSAGE ID */
	/* *((uint16_t*) _g_payload_buffer + 2) = */
	/* 	hd_message_id; */

	/* // TOKEN... leave it as-is in place. */

	/* const uint16_t response_len = */
	/* 	COAP_HEADER_LEN */
	/* 	+ hd_token_len; */

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

void sendUdpTemperature(otInstance* aInstance, uint8_t temperature) {

  otError error = OT_ERROR_NONE;
  otMessage*   message;
  otMessageInfo messageInfo;
  otIp6Address destinationAddr;

  memset(&messageInfo, 0, sizeof(messageInfo));

  otIp6AddressFromString("ff02::02", &destinationAddr);
  messageInfo.mPeerAddr = destinationAddr;
  messageInfo.mPeerPort = 1212;

  message = otUdpNewMessage(aInstance, NULL);
  if (message == NULL) {
    printf("Error creating udp message\n");
    return;
  }

  error = otMessageAppend(message, &temperature, 1);
  if (error != OT_ERROR_NONE && message != NULL) {
    printf("Error appending to udp message\n");
    otMessageFree(message);
    return;
  }

  error = otUdpSend(aInstance, &sUdpSocket, message, &messageInfo);
  if (error != OT_ERROR_NONE && message != NULL) {
    printf("Error sending udp packet\n");
    otMessageFree(message);
  }
}
