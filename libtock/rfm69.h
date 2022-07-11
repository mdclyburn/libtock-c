#pragma once

#include "tock.h"

#define DRIVER_NUM_RFM69 0x30003

/// Return 0 if the driver is available.
int rfm69_exists(void);

// Configuration

/// Use variable-length packets.
int rfm69_use_variable_length_packets();
/// Use fixed-length packets.
int rfm69_use_fixed_length_packets(
    const uint8_t packet_length);

/// Enable/disable the sync word.
int rfm69_set_sync_word_length(
    const uint8_t length);
/// Set the sync word.
int rfm69_set_sync_word(
    const uint32_t word_msb,
    const uint32_t word_lsb);

/// Turn off node address functionality.
int rfm69_disable_addressing();
/// Set and use node address only.
int rfm69_configure_address(
    const uint8_t node_address);
/// Set the node and broadcast addresses.
int rfm69_configure_address_bcast(
    const uint8_t node_address,
    const uint8_t broadcast_address);

/// Clear and disable the encryption key.
int rfm69_clear_encryption_key();
/// Set the encryption key's bytes.
int rfm69_set_encryption_key(
    const uint8_t byte_idx,
    const uint8_t val);

// TX/RX

/// Set the buffer for radio operations.
int rfm69_set_tx_rx_buffer(
    uint8_t* const buffer,
    const uint32_t len);

/// Transmit the current buffer contents.
int rfm69_transmit();
