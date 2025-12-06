import os
import subprocess
import random
from Crypto.Util import number
import sys

# ================= CẤU HÌNH =================
NUM_TESTS = 10          # Số lượng test case
BITS = 512# Độ lớn (bits).
EXE_PATH = "./main"     # Đường dẫn file compiled C++ (Windows: "main.exe")
INPUT_FILE = "input.txt"
OUTPUT_FILE = "output.txt"

if os.name == 'nt' and not EXE_PATH.endswith('.exe'):
    EXE_PATH += ".exe"

# ================= HÀM HỖ TRỢ =================
def to_reversed_hex(n):
    hex_str = hex(n)[2:]
    if len(hex_str) % 2 != 0:
        hex_str = '0' + hex_str
    reversed_char_str = hex_str[::-1]
    #delete the last 0 if exists
    if reversed_char_str.endswith('0'):
        reversed_char_str = reversed_char_str[:-1]
    return reversed_char_str

def get_primitive_root(p):
    if p == 2: return 1
    # Check nhanh (không hoàn hảo nhưng đủ cho random test)
    while True:
        g = random.randint(2, p - 1)
        if pow(g, (p - 1) // 2, p) != 1:
            return g

def generate_test_case_data(bits):
    p = number.getPrime(bits)
    g = get_primitive_root(p)
    x = random.randint(2, p - 2)     # Private key (ẩn)
    y = pow(g, x, p)                 # Public key
    m = random.randint(1, p - 2)     # Message
    
    # Sinh k sao cho gcd(k, p-1) = 1
    while True:
        k = random.randint(2, p - 2)
        if number.GCD(k, p - 1) == 1:
            break
            
    # Tính chữ ký mẫu bằng Python
    r = pow(g, k, p)
    k_inv = number.inverse(k, p - 1)
    s = ((m - x * r) * k_inv) % (p - 1)
    
    # Ensure r > 0
    while True:
        r = pow(g, k, p)
        if r > 0:
            break
        k = random.randint(2, p - 2)  # Regenerate k if r == 0
    
    if s == 0: return generate_test_case_data(bits)

    return { "p": p, "g": g, "y": y, "m": m, "r": r, "s": s }

# Added a verification step to ensure the generated signature (r, h) is valid.
def verify_signature(data):
    """
    Verify the generated signature (r, h) using the public key and message.
    """
    p, g, y, m, r, h = data['p'], data['g'], data['y'], data['m'], data['r'], data['s']

    # Verify that 0 < r < p and 0 < h < p - 1
    if not (0 < r < p and 0 < h < p - 1):
        return False

    # Verify that g^m ≡ y^r * r^h (mod p)
    left = pow(g, m, p)
    right = (pow(y, r, p) * pow(r, h, p)) % p
    print (f"left={left} \n right={right}")
    return left == right

# ================= MAIN LOOP =================
def run_tests():
    passed = 0
    print(f"[*] Running {NUM_TESTS} test cases (Logic: REVERSED HEX INPUT)...")
    print("-" * 60)

    for i in range(1, NUM_TESTS + 1):
        data = generate_test_case_data(BITS)

        # Verify the generated signature
        if not verify_signature(data):
            print(f"[TEST {i}] Invalid signature generated. Retrying...")
            continue

        # Write input with reversed hexadecimal strings
        with open(INPUT_FILE, "w") as f:
            f.write(f"{to_reversed_hex(data['p'])}\n")
            f.write(f"{to_reversed_hex(data['g'])}\n")
            f.write(f"{to_reversed_hex(data['y'])}\n")
            f.write(f"{to_reversed_hex(data['m'])}\n")
            f.write(f"{to_reversed_hex(data['r'])}\n")
            f.write(f"{to_reversed_hex(data['s'])}\n")
        
            print(f"[TEST data] p={data['p']}, g={data['g']}, y={data['y']}, m={data['m']}, r={data['r']}, s={data['s']}")
            print(f"[TEST {i}] Input written to file:")
            print(f"{to_reversed_hex(data['p'])}")
            print(f"{to_reversed_hex(data['g'])}")
            print(f"{to_reversed_hex(data['y'])}")
            print(f"{to_reversed_hex(data['m'])}")
            print(f"{to_reversed_hex(data['r'])}")
            print(f"{to_reversed_hex(data['s'])}")
        # Run the C++ program
        try:
            subprocess.run([EXE_PATH, INPUT_FILE, OUTPUT_FILE], check=True, capture_output=True)
        except Exception as e:
            print(f"[TEST {i}] Error running C++ file: {e}")
            continue

        # Read the result
        try:
            with open(OUTPUT_FILE, "r") as f:
                result = f.read().strip()
        except:
            result = "Error reading output"

        if result == "1":
            print(f"[TEST {i}] PASSED ✅")
            passed += 1
        else:
            print(f"[TEST {i}] FAILED ❌")
            print(f"   Output from C++: {result}")
            break

    print("-" * 60)
    print(f"Results: {passed}/{NUM_TESTS} PASSED.")

if __name__ == "__main__":
    run_tests()