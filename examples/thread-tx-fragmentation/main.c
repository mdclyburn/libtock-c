#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <libtock-sync/services/alarm.h>
#include <libtock/interface/button.h>
#include <libtock/interface/led.h>
#include <libtock/kernel/ipc.h>
#include <libtock/services/alarm.h>
#include <libtock/tock.h>

#include <libopenthread/platform/openthread-system.h>
#include <libopenthread/platform/plat.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/udp.h>

// helper utility demonstrating network config setup
static void setNetworkConfiguration(otInstance* aInstance);

static otUdpSocket sUdpSocket;

void initUdp(otInstance* aInstance);

static void print_ip_addr(otInstance* instance);

// callback for Thread state change events
static void stateChangeCallback(uint32_t flags, void* context);

#define BORDER_ROUTER_IP6_STR "fe80::404c:c223:cc7e:c8a7"

bool g_connected = false;
uint32_t g_last_pressed_debounce_ms = 0;
bool g_send_data = false;

void button_cb_send_data(
	__attribute__ ((unused)) returncode_t rtc,
	__attribute__ ((unused)) int button_no,
	bool pressed);

#define MAX_UDP_PAYLOAD_BYTES (79)
#define PAYLOAD_SIZE (MAX_UDP_PAYLOAD_BYTES)

uint8_t g_tx_payload[PAYLOAD_SIZE];
uint8_t g_tx_count;

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

	// Perform as much network transmission setup as possible.
	otError error = OT_ERROR_NONE;
	otMessageInfo messageInfo;
	otIp6Address destinationAddr;

	memset(&messageInfo, 0, sizeof(messageInfo));
	memset(g_tx_payload, 0xDE, PAYLOAD_SIZE);

	error = otIp6AddressFromString(BORDER_ROUTER_IP6_STR, &destinationAddr);
	if (error != OT_ERROR_NONE) {
		printf("Failed to convert destination address.\n");
		while (true) {  }
	}
	messageInfo.mPeerAddr = destinationAddr;
	messageInfo.mPeerPort = 25501;

	// Set up button callback.
	libtock_button_notify_on_press(0, button_cb_send_data);

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
		if (!otTaskletsArePending(instance) &&
			!openthread_platform_pending_work()) {

			// Offload to fReeLoaders when button is pressed.
			if (g_send_data && g_connected) {
				otMessage* message = NULL;
				message = otUdpNewMessage(instance, NULL);
				if (message == NULL) {
					printf("Error creating UDP message.\n");
					continue;
				}

				error = otMessageAppend(message, g_tx_payload, PAYLOAD_SIZE);
				if (error != OT_ERROR_NONE) {
					printf("Error appending to message.\n");
					otMessageFree(message);
					continue;
				}

				error = otUdpSend(instance, &sUdpSocket, message, &messageInfo);
				if (error != OT_ERROR_NONE && message != NULL) {
					printf("error sending packet: %d\n", error);
					otMessageFree(message);
				}
				printf("sent\n");
				libtocksync_alarm_delay_ms(100);
				g_tx_count += 1;
				if (g_tx_count >= 100) {
					g_send_data = false;
				}
			} else {
				yield();
			}
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
		g_connected = false;
		break;
    case OT_DEVICE_ROLE_CHILD:
		printf("[State Change] - Child.\n");
		printf("Successfully attached to Thread network as a child.\n");
		g_connected = true;
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
}

void button_cb_send_data(
	__attribute__ ((unused)) returncode_t rtc,
	__attribute__ ((unused)) int button_no,
	bool pressed)
{
	if (!g_connected || !pressed) { return; }

	uint32_t t_now;
	uint32_t t_now_ms;
	libtock_alarm_command_read(&t_now);
	t_now_ms = libtock_alarm_ticks_to_ms(t_now);

	uint32_t d_last_pressed_ms = t_now_ms - g_last_pressed_debounce_ms;
	if (d_last_pressed_ms < 300) { return; }
	g_last_pressed_debounce_ms = t_now_ms;

	g_send_data = !g_send_data;
	if (g_send_data) {
		printf("Sending data.\n");
		g_tx_count = 0;
		libtock_led_on(2);
	} else {
		printf("Stopped.\n");
		libtock_led_off(2);
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


void initUdp(otInstance* aInstance) {
	otSockAddr listenSockAddr;

	memset(&sUdpSocket, 0, sizeof(sUdpSocket));
	memset(&listenSockAddr, 0, sizeof(listenSockAddr));

	listenSockAddr.mPort = 12122;

	otUdpBind(aInstance, &sUdpSocket, &listenSockAddr, OT_NETIF_THREAD);
}
