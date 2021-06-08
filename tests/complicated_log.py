import time
import sys


class bcolors:
    OK = '\033[92m'  # GREEN
    WARNING = '\033[93m'  # YELLOW
    FAIL = '\033[91m'  # RED
    RESET = '\033[0m'  # RESET COLOR


counter = 0
while True:
    counter += 1
    sys.stdout.write(
        "                   s    p    a     c     e                  ")
    sys.stdout.write(
        str(counter)+f" {bcolors.OK}English English English{bcolors.RESET}\n")
    sys.stdout.write(str(counter) +
                     f" {bcolors.WARNING}Русский русский русский{bcolors.RESET}\n")
    sys.stderr.write(
        str(counter)+f" {bcolors.FAIL}M\nI\nS\nC{bcolors.RESET}\n")
    sys.stdout.flush()
    sys.stderr.flush()
    time.sleep(0.1)
    # if counter == 100:
    #     break
