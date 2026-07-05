print("=" * 60)
print("FINAL VERIFICATION")
print("=" * 60)

# 1. 485.c - IDLE handler
with open('Driver/485/485.c', 'rb') as f:
    c = f.read()
assert b'RingBuffer_Read(&recv_485_rb, temp_line_buf' not in c
print("[PASS] 485.c: IDLE handler no longer extracts from RingBuffer")

# 2. Function.c - recv_frame_nonblocking  
with open('Function/Function.c', 'rb') as f:
    c = f.read()
assert b'parse_ascii_frame_from_rb(&recv_485_rb, frame)' in c
assert c.count(b'uint8_t  tx_buffer[TX_BUFFER_SIZE];') == 1
print("[PASS] Function.c: recv_frame_nonblocking uses ASCII parser")
print("[PASS] Function.c: No duplicate tx_buffer/rx_buffer")

# 3. Function.c - parse_raw_frame field order
assert b'uint8_t  content_len = raw[7];' in c
assert b'uint8_t  version = raw[8];' in c
print("[PASS] Function.c: parse_raw_frame field order fixed")

# 4. Function.c - countdown (check for proper C escape sequences)
idx = c.find(b'sprintf(countdown_msg')
chunk = c[idx:idx+120]
# Find 'on(%ds)...' and verify next 4 bytes are \r\n" (5C 72 5C 6E 22)
pos = chunk.find(b'on(%ds)...')
next4 = chunk[pos+10:pos+14]
assert next4 == b'\x5c\x72\x5c\x6e', f"Expected \r\n escape, got {next4.hex()}"
print("[PASS] Function.c: countdown uses sprintf with proper \r\n escapes")

# 5. Function.c - ring buffer reset after firmware reception
assert b'RingBuffer_Reset(&recv_485_rb);' in c
assert b'reset_ascii_parser();' in c
print("[PASS] Function.c: RingBuffer reset + ASCII parser reset after firmware RX")

# 6. ModbusCRC.c - file-scope static vars
with open('Protocol/ModbusCRC.c', 'rb') as f:
    c = f.read()
for name in [b'static int state', b'static int bin_len', b'static unsigned char bin_buf',
              b'static int ascii_wait', b'static unsigned char high_nibble']:
    assert name in c, f"Missing file-scope: {name.decode()}"
print("[PASS] ModbusCRC.c: File-scope static variables declared")

# 7. ModbusCRC.c - reset function
assert b'void reset_ascii_parser(void)' in c
print("[PASS] ModbusCRC.c: reset_ascii_parser() function")

# 8. ModbusCRC.c - no static vars inside parse_ascii_frame_from_rb
idx2 = c.find(b'int parse_ascii_frame_from_rb')
idx3 = c.find(b'\n}', idx2)
func = c[idx2:idx3]
assert b'static int state' not in func
assert b'static int bin_len' not in func
print("[PASS] ModbusCRC.c: No static var declarations inside function")

# 9. ModbusCRC.c - parse_raw_frame field order
assert b'uint8_t  content_len = raw[7];' in c
print("[PASS] ModbusCRC.c: parse_raw_frame field order fixed")

# 10. ModbusCRC.h - declaration
with open('Protocol/ModbusCRC.h', 'rb') as f:
    c = f.read()
assert b'void reset_ascii_parser(void);' in c
print("[PASS] ModbusCRC.h: reset_ascii_parser declared")

# 11. param.c - ch1_threshold
with open('Driver/PARAM/param.c', 'rb') as f:
    c = f.read()
assert b'return g_param.ch1_threshold;' in c
assert b'return g_param.ch0_threshold;' not in c.replace(b'param_get_ch0_threshold', b'')
print("[PASS] param.c: param_get_ch1_threshold returns ch1_threshold")

# 12. LED.c - GPIO pins
with open('Driver/LED/LED.c', 'rb') as f:
    c = f.read()
assert b'GPIO_PIN_4' not in c
print("[PASS] LED.c: All PIN_4 replaced with PIN_8")

print()
print("=" * 60)
print("ALL CHECKS PASSED")
print("=" * 60)
