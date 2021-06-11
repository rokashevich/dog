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
    n = str(counter)
    sys.stdout.write(
        f"{bcolors.OK}   <   {n}   р  у  с  с  к  и  й    >      {bcolors.RESET}\n")
    sys.stdout.write(">>>"+str(counter)+"<<<\n")
    sys.stdout.write("                s    p    a     c          e        \n")
    sys.stdout.write(f" {n} {bcolors.OK}English  English \n{bcolors.RESET}")
    # sys.stdout.write(
    #     f" {n} {bcolors.OK}Русский  р\nу\nс\nс\nк\nи\nй\n \n{bcolors.RESET}")
    sys.stdout.write(
        f" {n} {bcolors.OK}русский русский{bcolors.RESET}")
    sys.stderr.write(
        f" {bcolors.OK}   111\n222\n     333("+str(counter)+f")\n{bcolors.RESET}")
    sys.stdout.write("[[["+str(counter)+"]]]\n")
    sys.stdout.flush()
    sys.stderr.flush()
    time.sleep(0.01)
    if counter == 1000:
        break
