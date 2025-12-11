import subprocess
import sys
import os
from pathlib import Path
import time

def reversed_hex_to_int(hex_str):
    """
    Convert reversed hex string to integer
    Mimics C++ fromReversedHex logic:
    1. Remove whitespace
    2. Reverse the string to get normal hex format
    3. Pad to even length if necessary
    4. Convert to integer
    """
    # Remove all whitespace
    s = ''.join(hex_str.split())
    
    # Reverse to get normal hex format
    s = s[::-1]
    
    # Handle empty string
    if not s:
        return 0
    
    # Pad to even length
    if len(s) % 2 == 1:
        s = '0' + s
    
    return int(s, 16) if s else 0

def int_to_reversed_hex(num):
    """
    Convert integer to reversed hex string
    Mimics C++ toReversedHex logic:
    1. Convert number to bytes (little-endian)
    2. Convert each byte to hex (always 2 digits)
    3. Concatenate all hex pairs
    4. Remove leading zeros (but keep at least "00")
    5. Result is already in reversed format
    """
    if num == 0:
        return "0"
    
    # Convert to bytes and build hex string like C++
    bytes_list = []
    temp = num
    while temp > 0:
        bytes_list.append(temp & 0xFF)
        temp >>= 8
    
    # Build hex string from highest byte to lowest (like C++)
    hex_str = ""
    for i in range(len(bytes_list) - 1, -1, -1):
        b = bytes_list[i]
        hex_str += format((b >> 4) & 0xF, 'X')
        hex_str += format(b & 0xF, 'X')
    
    if not hex_str:
        return "0"
    
    # Remove leading zeros (but keep at least one digit)
    hex_str = hex_str.lstrip('0')
    if not hex_str:
        return "0"
    
    # Reverse to get reversed hex format
    reversed_hex = hex_str[::-1]
    
    # Remove trailing zeros from reversed hex (which were leading zeros in normal hex)
    # Example: "0009A" reversed is "A9000" -> should be "A9"
    reversed_hex = reversed_hex.rstrip('0')
    if not reversed_hex:
        return "0"
    
    return reversed_hex

def mod_inverse(a, m):
    """Compute modular inverse using Extended Euclidean Algorithm"""
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

def elgamal_decrypt_python(p, g, x, c1, c2):
    """
    Perform ElGamal decryption in Python
    Returns: (h, m) where h = g^x mod p and m = c2 * (c1^x)^(-1) mod p
    """
    # Compute public key: h = g^x mod p
    h = pow(g, x, p)
    
    # Compute c1^x mod p
    c1x = pow(c1, x, p)
    
    # Compute modular inverse of c1x
    c1x_inv = mod_inverse(c1x, p)
    
    # Decrypt: m = c2 * c1x_inv mod p
    m = (c2 * c1x_inv) % p
    
    return h, m

def run_exe_with_timeout(exe_path, input_file, output_file, timeout=60):
    """Run a.exe with timeout"""
    try:
        # Run the executable with timeout
        result = subprocess.run(
            [exe_path, input_file, output_file],
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd=os.path.dirname(exe_path)
        )
        return result.returncode == 0, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return False, "", "TIMEOUT: Execution exceeded 60 seconds"
    except Exception as e:
        return False, "", f"ERROR: {str(e)}"

def compare_test(exe_path, test_input, test_num):
    """Compare results for a single test case"""
    print(f"\n{'='*60}")
    print(f"Test {test_num:02d}: {os.path.basename(test_input)}")
    print(f"{'='*60}")
    
    # Check if input file exists
    if not os.path.exists(test_input):
        print(f"❌ Input file not found: {test_input}")
        return False
    
    # Read input
    try:
        with open(test_input, 'r') as f:
            lines = f.readlines()
            if len(lines) < 5:
                print(f"❌ Invalid input file (expected 5 lines)")
                return False
            
            p_hex = lines[0].strip()
            g_hex = lines[1].strip()
            x_hex = lines[2].strip()
            c1_hex = lines[3].strip()
            c2_hex = lines[4].strip()
    except Exception as e:
        print(f"❌ Error reading input: {e}")
        return False
    
    print(f"Input (reversed hex):")
    print(f"  p  = {p_hex}")
    print(f"  g  = {g_hex}")
    print(f"  x  = {x_hex}")
    print(f"  c1 = {c1_hex}")
    print(f"  c2 = {c2_hex}")
    
    # Convert to integers
    try:
        p = reversed_hex_to_int(p_hex)
        g = reversed_hex_to_int(g_hex)
        x = reversed_hex_to_int(x_hex)
        c1 = reversed_hex_to_int(c1_hex)
        c2 = reversed_hex_to_int(c2_hex)
    except Exception as e:
        print(f"❌ Error converting hex to int: {e}")
        return False
    
    # Python calculation
    print(f"\n🐍 Python calculation...")
    start_time = time.time()
    try:
        h_py, m_py = elgamal_decrypt_python(p, g, x, c1, c2)
        h_py_hex = int_to_reversed_hex(h_py)
        m_py_hex = int_to_reversed_hex(m_py)
        python_time = time.time() - start_time
        print(f"  Time: {python_time:.4f}s")
        print(f"  h = {h_py_hex}")
        print(f"  m = {m_py_hex}")
    except Exception as e:
        print(f"❌ Python calculation error: {e}")
        return False
    
    # Run a.exe
    print(f"\n💻 Running a.exe...")
    output_file = test_input.replace('.inp', '_temp.out')
    start_time = time.time()
    success, stdout, stderr = run_exe_with_timeout(exe_path, test_input, output_file, timeout=60)
    exe_time = time.time() - start_time
    
    if not success:
        print(f"❌ a.exe execution failed")
        print(f"  Time: {exe_time:.4f}s")
        if stderr:
            print(f"  Error: {stderr}")
        return False
    
    print(f"  Time: {exe_time:.4f}s")
    
    # Read a.exe output
    try:
        with open(output_file, 'r') as f:
            lines = f.readlines()
            if len(lines) < 2:
                print(f"❌ Invalid output file (expected 2 lines)")
                return False
            h_exe_hex = lines[0].strip()
            m_exe_hex = lines[1].strip()
        
        print(f"  h = {h_exe_hex}")
        print(f"  m = {m_exe_hex}")
        
        # Clean up temporary output
        os.remove(output_file)
    except Exception as e:
        print(f"❌ Error reading output: {e}")
        return False
    
    # Compare results
    print(f"\n📊 Comparison:")
    h_match = h_py_hex.upper() == h_exe_hex.upper()
    m_match = m_py_hex.upper() == m_exe_hex.upper()
    
    print(f"  h match: {'✅ YES' if h_match else '❌ NO'}")
    if not h_match:
        print(f"    Python: {h_py_hex}")
        print(f"    a.exe:  {h_exe_hex}")
    
    print(f"  m match: {'✅ YES' if m_match else '❌ NO'}")
    if not m_match:
        print(f"    Python: {m_py_hex}")
        print(f"    a.exe:  {m_exe_hex}")
    
    result = h_match and m_match
    print(f"\n{'✅ PASSED' if result else '❌ FAILED'}")
    
    return result

def main():
    import sys
    
    # Get the script directory
    script_dir = Path(__file__).parent
    exe_path = script_dir / "main.exe"
    
    # Allow user to specify test directory via command line
    if len(sys.argv) > 1:
        test_dir = script_dir / sys.argv[1]
    else:
        # Default to project_02_03, or testcase_generated if it exists
        test_dir_generated = script_dir / "testcase_generated"
        test_dir_default = script_dir / "project_02_03"
        

        test_dir = test_dir_default
    
    # Check if a.exe exists
    if not exe_path.exists():
        print(f"❌ Error: a.exe not found at {exe_path}")
        print(f"Please compile main.cpp first")
        return
    
    # Find all test input files
    test_files = sorted(test_dir.glob("test_*.inp"))
    
    if not test_files:
        print(f"❌ No test files found in {test_dir}")
        return
    
    print(f"Test directory: {test_dir}")
    print(f"Found {len(test_files)} test files")
    print(f"Timeout limit: 60 seconds per test")
    print(f"Output format: reversed hex")
    
    # Run all tests
    passed = 0
    failed = 0
    
    for test_file in test_files:
        test_num = int(test_file.stem.split('_')[1])
        
        if compare_test(str(exe_path), str(test_file), test_num):
            passed += 1
        else:
            failed += 1
    
    # Summary
    print(f"\n{'='*60}")
    print(f"SUMMARY")
    print(f"{'='*60}")
    print(f"Total tests: {len(test_files)}")
    print(f"Passed: {passed} ✅")
    print(f"Failed: {failed} ❌")
    print(f"Success rate: {passed/len(test_files)*100:.1f}%")

if __name__ == "__main__":
    main()
