import socket
import struct
import time
from sim_stream import encode_arinc429_bnr

UDP_IP = "127.0.0.1"
UDP_PORT = 5005

def run_stress_test(total_frames=100000):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"[Stress Test] Streaming {total_frames} ARINC 429 frames...")

    start_time = time.time()
    
    for i in range(total_frames):
        # Generate valid frame (95% of time) or force corrupted parity frame (5% of time)
        alt = 10000.0 + (i % 5000)
        word = encode_arinc429_bnr(0o204, 1, alt, 17, 131072.0, True)

        if i % 20 == 0:  # Introduce parity fault
            word ^= (1 << 31)

        sock.sendto(struct.pack('<I', word), (UDP_IP, UDP_PORT))

    elapsed = time.time() - start_time
    print(f"[Stress Test] Done in {elapsed:.2f} seconds ({total_frames / elapsed:.0f} words/sec transmitted).")
    sock.close()

if __name__ == "__main__":
    run_stress_test(total_frames=100000)