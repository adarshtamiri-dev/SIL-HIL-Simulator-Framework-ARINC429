# sim_stream.py
import socket
import struct
import time
import math

UDP_IP = "127.0.0.1"
UDP_PORT = 5005

def reverse_bits_8(val: int) -> int:
    """Reverse 8 bits for ARINC 429 label standard."""
    result = 0
    for i in range(8):
        if (val >> i) & 1:
            result |= (1 << (7 - i))
    return result

def encode_arinc429_bnr(label_octal: int, sdi: int, value: float, bits: int, max_range: float, is_signed: bool, ssm: int = 3) -> int:
    """Pack flight telemetry into a 32-bit ARINC word."""
    word = 0
    
    # 1. Reverse Label
    word |= reverse_bits_8(label_octal)
    
    # 2. SDI
    word |= (sdi & 0x03) << 8
    
    # 3. BNR Data Field Encoding
    if is_signed:
        normalized = value / max_range
        scaled = int(normalized * (1 << (bits - 1)))
        data_val = scaled & ((1 << bits) - 1)
    else:
        normalized = value / max_range
        scaled = int(normalized * ((1 << bits) - 1))
        data_val = scaled & ((1 << bits) - 1)

    # Place payload inside bits 11-29
    word |= (data_val << (19 - bits)) << 10

    # 4. SSM
    word |= (ssm & 0x03) << 29

    # 5. Enforce Odd Parity
    ones_count = bin(word).count('1')
    if ones_count % 2 == 0:
        word |= (1 << 31)

    return word

def run_simulation():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"[Python Sim] Streaming telemetry to {UDP_IP}:{UDP_PORT}...")

    t = 0.0
    try:
        while True:
            # Simulate real-time flight telemetry curves
            altitude = 12000.0 + 1500.0 * math.sin(t / 4.0)  # Feet
            airspeed = 260.0 + 20.0 * math.cos(t / 2.0)      # Knots

            # Encode Altitude: Label 0204 (Octal), 17 bits, 131072.0 max ft
            alt_word = encode_arinc429_bnr(0o204, 1, altitude, 17, 131072.0, True)

            # Encode Airspeed: Label 0206 (Octal), 14 bits, 1024.0 max knots
            spd_word = encode_arinc429_bnr(0o206, 1, airspeed, 14, 1024.0, False)

            # Send 32-bit unsigned binary frames over UDP
            sock.sendto(struct.pack('<I', alt_word), (UDP_IP, UDP_PORT))
            sock.sendto(struct.pack('<I', spd_word), (UDP_IP, UDP_PORT))

            print(f"Sent -> Alt: {altitude:.1f} ft | Speed: {airspeed:.1f} kts")
            t += 0.5
            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\n[Sim] Stopped.")
    finally:
        sock.close()

if __name__ == "__main__":
    run_simulation()