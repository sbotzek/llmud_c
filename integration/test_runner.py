# test_runner.py
#
# MUD Test Runner (final clean version)

import sys
import traceback
from helpers.std import *

def run_test_file(path):
    reset_log(path)
    clear_data_directory()

    with open(path, 'r') as f:
        code = f.read()

    exec(code)  # No global_vars needed, tests import their own

    disconnect_all_clients()

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 test_runner.py tests/*.py")
        sys.exit(1)

    test_files = sys.argv[1:]
    all_passed = True

    for test_file in test_files:
        print(f"Running {test_file}: ", end='', flush=True)

        try:
            run_test_file(test_file)
            print("PASSED")
        except SystemExit:
            print("FAILED")
            all_passed = False
            break
        except Exception:
            traceback.print_exc()
            dump_failure()
            print("FAILED")
            all_passed = False
            break

    if all_passed:
        print("\n✅ All tests passed.")
    else:
        print("\n❌ Test(s) failed.")

    sys.exit(0)

if __name__ == "__main__":
    main()
