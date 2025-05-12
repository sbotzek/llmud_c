# test_runner.py
#
# MUD Test Runner (Script Mode Only)
# No external dependencies.
# Supports: send, expect, delay, compare_files
# Clears 'data/' directory before each test
# Applies 'data_template' if specified
# Fails fast on first error

import os
import shutil
import socket
import sys
import json
import time

# Constants
DEFAULT_EXPECT_TIMEOUT = 5.0  # seconds
DATA_DIR = './data'

# Helper Functions

def log_pass(message):
    print(f"[PASS] {message}")

def log_fail(message):
    print(f"[FAIL] {message}")
    sys.exit(1)

def clear_data_directory():
    if os.path.exists(DATA_DIR):
        shutil.rmtree(DATA_DIR)
    os.makedirs(DATA_DIR)
    print(f"[info] Cleared data directory: {DATA_DIR}")

def apply_data_template(template_path):
    if not os.path.isdir(template_path):
        log_fail(f"Data template not found: {template_path}")
    shutil.copytree(template_path, DATA_DIR, dirs_exist_ok=True)
    print(f"[info] Applied data template: {template_path} -> {DATA_DIR}")

# Client class

class Client:
    def __init__(self, name, host, port):
        self.name = name
        self.host = host
        self.port = port
        self.sock = None
        self.buffer = ""

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port))
        self.sock.settimeout(0.5)  # Non-blocking read with timeout
        print(f"[info] Connected client '{self.name}'")

    def send_line(self, line):
        self.sock.sendall((line + '\n').encode('utf-8'))

    def read_available(self):
        try:
            data = self.sock.recv(4096)
            if data:
                text = data.decode('utf-8', errors='ignore')
                print(f"[recv] ({self.name}): {repr(text)}")
                self.buffer += text
        except socket.timeout:
            pass
        except Exception as e:
            log_fail(f"Socket error on client '{self.name}': {e}")

    def expect(self, substring, timeout=DEFAULT_EXPECT_TIMEOUT):
        end_time = time.time() + timeout
        while time.time() < end_time:
            self.read_available()
            if substring in self.buffer:
                return True
            time.sleep(0.05)
        return False

    def disconnect(self):
        if self.sock:
            self.sock.close()
            print(f"[info] Disconnected client '{self.name}'")

# Compare two files exactly

def compare_files(file1, file2):
    try:
        with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
            content1 = f1.read()
            content2 = f2.read()
            return content1 == content2
    except Exception as e:
        log_fail(f"Error reading files for comparison: {e}")
    return False

def ensure_file_not_exists(path):
    if os.path.exists(path):
        log_fail(f"File exists but should not: {path}")
    else:
        log_pass(f"File correctly does not exist: {path}")

# Main runner

def run_test(test_data):
    if test_data.get('mode') != 'script':
        log_fail("Only 'script' mode is supported.")

    # Clear and setup data directory
    clear_data_directory()
    if 'data_template' in test_data:
        apply_data_template(test_data['data_template'])

    # Connect clients
    clients = {}
    for client_name, conn_str in test_data['clients'].items():
        host, port_str = conn_str.split(':')
        port = int(port_str)
        client = Client(client_name, host, port)
        client.connect()
        clients[client_name] = client

    # Run script
    for idx, step in enumerate(test_data['script']):
        fn = step.get('fn')
        client_name = step.get('client')

        if fn == 'send':
            if not client_name or 'text' not in step:
                log_fail(f"Missing client/text in send step {idx}")
            clients[client_name].send_line(step['text'])
            print(f"[send] ({client_name}): {step['text']}")

        elif fn == 'expect':
            if not client_name or 'text' not in step:
                log_fail(f"Missing client/text in expect step {idx}")
            success = clients[client_name].expect(step['text'])
            if success:
                log_pass(f"({client_name}) saw expected text: '{step['text']}'")
            else:
                log_fail(f"({client_name}) did NOT see expected text: '{step['text']}' within {DEFAULT_EXPECT_TIMEOUT}s")

        elif fn == 'delay':
            if 'time' not in step:
                log_fail(f"Missing time in delay step {idx}")
            time.sleep(step['time'] / 1000.0)
            print(f"[delay] {step['time']}ms")

        elif fn == 'compare_files':
            if 'files' not in step or len(step['files']) != 2:
                log_fail(f"Missing or invalid files list in compare_files step {idx}")
            file1, file2 = step['files']
            if compare_files(file1, file2):
                log_pass(f"Files match: {file1} == {file2}")
            else:
                log_fail(f"File mismatch: {file1} != {file2}")

        elif fn == 'file_not_exists':
            if 'file' not in step:
                log_fail(f"Missing file in file_not_exists step {idx}")
            ensure_file_not_exists(step['file'])

        else:
            log_fail(f"Unknown command '{fn}' in step {idx}")

    # Clean shutdown
    for client in clients.values():
        client.disconnect()

    print("\n✅ TEST PASSED")

# Entry point
def main():
    if len(sys.argv) < 2:
        print("Usage: python3 test_runner.py <testfile1.json> [<testfile2.json> ...]")
        sys.exit(1)

    test_files = sys.argv[1:]

    for test_file in test_files:
        print(f"\n=== Running test: {test_file} ===")
        if not os.path.isfile(test_file):
            log_fail(f"Test file not found: {test_file}")

        try:
            with open(test_file, 'r') as f:
                test_data = json.load(f)
        except Exception as e:
            log_fail(f"Failed to load test file '{test_file}': {e}")

        run_test(test_data)

    print("\n✅ All tests passed.")
    sys.exit(0)

if __name__ == "__main__":
    main()

