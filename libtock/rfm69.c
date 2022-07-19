#include "rfm69.h"
#include "tock.h"

int rfm69_exists(void) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 0, 0, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_use_variable_length_packets(void) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 45, 1, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_use_fixed_length_packets(const uint8_t packet_length) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 45, 0, packet_length);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_use_sync_word(const uint8_t length) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 40, length, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_set_sync_word(
    const uint32_t word_msb,
    const uint32_t word_lsb)
{
    syscall_return_t r = command(DRIVER_NUM_RFM69, 41, word_msb, word_lsb);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_set_bit_rate(const uint16_t bit_rate) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 42, bit_rate, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_disable_addressing(void) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 50, 256, 256);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_configure_address(const uint8_t node_address) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 50, node_address, 256);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_configure_address_bcast(
    const uint8_t node_address,
    const uint8_t broadcast_address)
{
    syscall_return_t r = command(
        DRIVER_NUM_RFM69,
        50,
        node_address,
        broadcast_address);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_clear_encryption_key(void) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 61, 0, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_set_encryption_key(
    const uint8_t byte_idx,
    const uint8_t val)
{
    syscall_return_t r = command(DRIVER_NUM_RFM69, 60, byte_idx, val);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_set_tx_rx_buffer(uint8_t* const buffer, const uint32_t len) {
    allow_rw_return_t r = allow_readwrite(DRIVER_NUM_RFM69, 0, (void*) buffer, len);
    return tock_allow_rw_return_to_returncode(r);
}

int rfm69_transmit(void) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 10, 0, 0);
    return tock_command_return_novalue_to_returncode(r);
}

int rfm69_set_tx_complete_callback(subscribe_upcall callback, void* callback_args) {
    subscribe_return_t sval = subscribe(DRIVER_NUM_RFM69, 0, callback, callback_args);
    return tock_subscribe_return_to_returncode(sval);
}

int rfm69_dump_registers(const uint32_t sel) {
    syscall_return_t r = command(DRIVER_NUM_RFM69, 400, sel, 0);
    return tock_command_return_novalue_to_returncode(r);
}
