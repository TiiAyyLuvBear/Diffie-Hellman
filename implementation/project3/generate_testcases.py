import random
import os
from pathlib import Path

def is_prime(n, k=10):
    """Miller-Rabin primality test"""
    if n < 2:
        return False
    if n == 2 or n == 3:
        return True
    if n % 2 == 0:
        return False
    
    # Write n-1 as 2^r * d
    r, d = 0, n - 1
    while d % 2 == 0:
        r += 1
        d //= 2
    
    # Witness loop
    for _ in range(k):
        a = random.randrange(2, n - 1)
        x = pow(a, d, n)
        
        if x == 1 or x == n - 1:
            continue
        
        for _ in range(r - 1):
            x = pow(x, 2, n)
            if x == n - 1:
                break
        else:
            return False
    
    return True

def generate_prime(bits):
    """Generate a random prime number with specified bit length"""
    while True:
        # Generate odd number
        p = random.getrandbits(bits)
        p |= (1 << bits - 1) | 1  # Set MSB and LSB to 1
        
        if is_prime(p):
            return p

def find_primitive_root(p):
    """Find a primitive root modulo p (simple approach for small primes)"""
    # For large primes, this is a simplified approach
    # A proper implementation would factor p-1 and check the order
    
    phi = p - 1
    
    # Get prime factors of phi
    def prime_factors(n):
        factors = set()
        d = 2
        while d * d <= n:
            while n % d == 0:
                factors.add(d)
                n //= d
            d += 1
        if n > 1:
            factors.add(n)
        return factors
    
    factors = prime_factors(phi)
    
    # Try small values first for efficiency
    for g in range(2, min(1000, p)):
        is_primitive = True
        for factor in factors:
            if pow(g, phi // factor, p) == 1:
                is_primitive = False
                break
        if is_primitive:
            return g
    
    # If no small primitive root found, try random values
    for _ in range(1000):
        g = random.randrange(2, p)
        is_primitive = True
        for factor in factors:
            if pow(g, phi // factor, p) == 1:
                is_primitive = False
                break
        if is_primitive:
            return g
    
    return 2  # Fallback (might not be primitive)

def int_to_reversed_hex(num):
    """Convert integer to reversed hex format (matching C++ implementation)"""
    if num == 0:
        return "0"
    
    # Convert to hex string (uppercase, no prefix)
    hex_str = format(num, 'X')
    
    # Remove leading zeros
    hex_str = hex_str.lstrip('0')
    if not hex_str:
        return "0"
    
    # Reverse to get reversed hex format
    reversed_hex = hex_str[::-1]
    
    # Remove trailing zeros
    reversed_hex = reversed_hex.rstrip('0')
    if not reversed_hex:
        return "0"
    
    return reversed_hex

def mod_inverse(a, m):
    """Extended Euclidean Algorithm for modular inverse"""
    def extended_gcd(a, b):
        if a == 0:
            return b, 0, 1
        gcd, x1, y1 = extended_gcd(b % a, a)
        x = y1 - (b // a) * x1
        y = x1
        return gcd, x, y
    
    gcd, x, _ = extended_gcd(a % m, m)
    if gcd != 1:
        raise ValueError("Modular inverse does not exist")
    return (x % m + m) % m

def generate_elgamal_testcase(bits, test_num, output_dir):
    """Generate one ElGamal test case"""
    print(f"Generating test {test_num:02d} with {bits} bits...")
    
    # Generate prime p
    p = generate_prime(bits)
    
    # Find primitive root g
    g = find_primitive_root(p)
    
    # Generate private key x (random, < p-1)
    x = random.randrange(1, p - 1)
    
    # Compute public key h = g^x mod p
    h = pow(g, x, p)
    
    # Generate random message m (< p)
    m = random.randrange(1, p)
    
    # Generate random ephemeral key y (for encryption)
    y = random.randrange(1, p - 1)
    
    # Encrypt: c1 = g^y mod p, c2 = m * h^y mod p
    c1 = pow(g, y, p)
    c2 = (m * pow(h, y, p)) % p
    
    # Verify decryption: m' = c2 * (c1^x)^(-1) mod p
    c1x = pow(c1, x, p)
    c1x_inv = mod_inverse(c1x, p)
    m_decrypted = (c2 * c1x_inv) % p
    
    assert m == m_decrypted, f"Decryption verification failed! m={m}, m'={m_decrypted}"
    
    # Convert to reversed hex
    p_hex = int_to_reversed_hex(p)
    g_hex = int_to_reversed_hex(g)
    x_hex = int_to_reversed_hex(x)
    c1_hex = int_to_reversed_hex(c1)
    c2_hex = int_to_reversed_hex(c2)
    h_hex = int_to_reversed_hex(h)
    m_hex = int_to_reversed_hex(m)
    
    # Write input file
    input_file = output_dir / f"test_{test_num:02d}.inp"
    with open(input_file, 'w') as f:
        f.write(f"{p_hex}\n")
        f.write(f"{g_hex}\n")
        f.write(f"{x_hex}\n")
        f.write(f"{c1_hex}\n")
        f.write(f"{c2_hex}\n")
    
    # Write output file
    output_file = output_dir / f"test_{test_num:02d}.out"
    with open(output_file, 'w') as f:
        f.write(f"{h_hex}\n")
        f.write(f"{m_hex}\n")
    
    print(f"  p = {p_hex} ({bits} bits)")
    print(f"  g = {g_hex}")
    print(f"  Test {test_num:02d} generated successfully!")
    
    return True

def main():
    # Create output directory
    script_dir = Path(__file__).parent
    output_dir = script_dir / "testcase_generated"
    output_dir.mkdir(exist_ok=True)
    
    print("="*60)
    print("ElGamal Test Case Generator")
    print("="*60)
    print(f"Output directory: {output_dir}")
    print(f"Integer limit: < 512 bits")
    print(f"Format: Reversed hexadecimal (little-endian representation)")
    print("="*60)
    
    # Test configuration: different bit sizes (20 tests total)
    test_configs = [
        (8, 3),    # 8-bit primes: 3 tests
        (16, 3),   # 16-bit primes: 3 tests
        (32, 3),   # 32-bit primes: 3 tests
        (64, 3),   # 64-bit primes: 3 tests
        (128, 3),  # 128-bit primes: 3 tests
        (256, 2),  # 256-bit primes: 2 tests
        (512, 3),  # 512-bit primes: 3 tests (ít nhất 3)
    ]
    
    test_num = 0
    total_tests = sum(count for _, count in test_configs)
    
    for bits, count in test_configs:
        print(f"\nGenerating {count} test cases with {bits}-bit primes...")
        for i in range(count):
            try:
                generate_elgamal_testcase(bits, test_num, output_dir)
                test_num += 1
            except Exception as e:
                print(f"  Error generating test {test_num}: {e}")
                print(f"  Retrying...")
                try:
                    generate_elgamal_testcase(bits, test_num, output_dir)
                    test_num += 1
                except Exception as e2:
                    print(f"  Failed again: {e2}")
                    print(f"  Skipping test {test_num}")
                    test_num += 1
    
    print("\n" + "="*60)
    print(f"Generation complete!")
    print(f"Total tests generated: {test_num}/{total_tests}")
    print(f"Output directory: {output_dir}")
    print("="*60)
    
    # List generated files
    inp_files = sorted(output_dir.glob("test_*.inp"))
    out_files = sorted(output_dir.glob("test_*.out"))
    print(f"\nGenerated files:")
    print(f"  Input files: {len(inp_files)}")
    print(f"  Output files: {len(out_files)}")

if __name__ == "__main__":
    main()
