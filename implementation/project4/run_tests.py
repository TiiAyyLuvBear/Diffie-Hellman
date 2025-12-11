import os
import subprocess

def validate_output(test_input, program_output):
    """
    Custom validation logic for program output.
    Modify this function to define correctness criteria.
    """
    # Example: Check if output contains a specific keyword
    if "success" in program_output.lower():
        return True
    return False

def run_tests():
    test_dir = "test"
    executable = "main.exe"  # Ensure main.cpp is compiled to main.exe

    # Compile main.cpp if not compiled
    if not os.path.exists(executable):
        print("Compiling main.cpp...")
        compile_result = subprocess.run(["g++", "main.cpp", "-o", executable], capture_output=True, text=True)
        if compile_result.returncode != 0:
            print("Compilation failed:")
            print(compile_result.stderr)
            return

    # Iterate through all .inp files in the test directory
    for file in os.listdir(test_dir):
        if file.endswith(".inp"):
            test_input_path = os.path.join(test_dir, file)
            test_output_path = test_input_path.replace(".inp", ".out")

            print(f"Running test: {file}")

            # Run the executable with the input and output file
            result = subprocess.run(["./" + executable, test_input_path, test_output_path], capture_output=True, text=True)

            if result.returncode != 0:
                print(f"Test {file} failed to execute:")
                print(result.stderr)
                continue

            # Read the output file
            if os.path.exists(test_output_path):
                with open(test_output_path, "r") as out_file:
                    program_output = out_file.read().strip()

                print("Program Output:")
                print(program_output)

                # Validate output using custom logic
                if validate_output(test_input_path, program_output):
                    print(f"Test {file} passed validation.")
                else:
                    print(f"Test {file} failed validation.")
            else:
                print(f"Output file {test_output_path} was not created.")

if __name__ == "__main__":
    run_tests()