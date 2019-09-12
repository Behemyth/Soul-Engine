import subprocess

from colorama import Fore, Style, deinit, init

def Task(name, *arguments):

    print("Task \"{}\" started...".format(name), end = " ", flush=True)
    response = subprocess.run(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    init(autoreset=True)

    try:      
        response.check_returncode()
        print(Fore.GREEN + "completed successfully")

    except:
        print(Fore.RED + "failed")
        print(Fore.RED + response.stderr.decode('ascii'))

    deinit()