# helpers/std.py
#
# Built-in testing helpers and logging for the MUD Test Runner.

import os
import shutil
import time
import socket
import sys

# Constants
DEFAULT_EXPECT_TIMEOUT = 5.0
DATA_DIR = './data'
LOG_FILE = 'test_runner.log'

# Log storage
current_log = []
current_test_file = ""

# Logging functions
def log_line(message):
    current_log.append(message)
    with open(LOG_FILE, 'a') as f:
        f.write(message + '\n')

def log_pass(message):
    log_line(f"[PASS] {message}")

def log_fail(message):
    log_line(f"[FAIL] {message}")
    dump_failure()
    sys.exit(1)

def dump_failure():
    print("\n=== TEST OUTPUT (failure) ===")
    for line in current_log:
        print(line)
    print(f"\nFAILED {current_test_file}")

def reset_log(file_name):
    global current_log, current_test_file
    current_log = []
    current_test_file = file_name
    if os.path.exists(LOG_FILE):
        os.remove(LOG_FILE)

# Utility functions
def clear_data_directory():
    if os.path.exists(DATA_DIR):
        shutil.rmtree(DATA_DIR)
    os.makedirs(DATA_DIR)
    log_line(f"[info] Cleared data directory: {DATA_DIR}")

def apply_data_template(template_path):
    if not os.path.isdir(template_path):
        log_fail(f"Data template not found: {template_path}")
    shutil.copytree(template_path, DATA_DIR, dirs_exist_ok=True)
    log_line(f"[info] Applied data template: {template_path} -> {DATA_DIR}")

def compare_files(file1, file2):
    try:
        with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
            return f1.read() == f2.read()
    except Exception as e:
        log_fail(f"Error comparing files: {e}")
    return False

def ensure_file_not_exists(path):
    if os.path.exists(path):
        log_fail(f"File exists but should not: {path}")
    else:
        log_pass(f"File correctly does not exist: {path}")

def delay(milliseconds):
    time.sleep(milliseconds / 1000.0)
    log_line(f"[delay] {milliseconds}ms")

def files_match(file1, file2):
    if compare_files(file1, file2):
        log_pass(f"Files match: {file1} == {file2}")
    else:
        log_fail(f"File mismatch: {file1} != {file2}")

def file_not_exists(path):
    ensure_file_not_exists(path)

# Client tracking
_all_clients = []

class Client:
    def __init__(self, name):
        self.name = name
        self.host = 'localhost'
        self.port = 4444
        self.sock = socket.create_connection((self.host, self.port))
        self.connected = True
        self.sock.settimeout(0.5)
        self.buffer = ""
        _all_clients.append(self)
        log_line(f"[info] Connected client '{self.name}'")

    def send(self, text):
        self.sock.sendall((text + '\n').encode('utf-8'))
        log_line(f"[send] ({self.name}): {text}")

    def read_available(self):
        try:
            data = self.sock.recv(4096)
            if not data:
                log_line(f"[info] Client '{self.name}' disconnected (socket closed)")
                self.connected = False
                self.sock.close()
                self.sock = None
                return
            text = data.decode('utf-8', errors='ignore')
            log_line(f"[recv] ({self.name}): {repr(text)}")
            self.buffer += text
        except socket.timeout:
            pass
        except Exception as e:
            log_fail(f"Socket error on client '{self.name}': {e}")

    def expect(self, substring, timeout=DEFAULT_EXPECT_TIMEOUT):
        end_time = time.time() + timeout
        while time.time() < end_time:
            self.read_available()
            if not self.connected:
                log_fail(f"Client '{self.name}' disconnected unexpectedly while waiting for text '{substring}'.")
            if substring in self.buffer:
                log_pass(f"({self.name}) saw expected text: '{substring}'")
                return
            time.sleep(0.05)
        log_fail(f"({self.name}) did NOT see expected text: '{substring}' within {timeout}s")

    def disconnect(self):
        if self.sock:
            self.sock.sendall(('killconn\n').encode('utf-8'))
            self.sock.close()
            log_line(f"[info] Disconnected client '{self.name}'")

def disconnect_all_clients():
    for client in _all_clients:
        client.disconnect()
    _all_clients.clear()
