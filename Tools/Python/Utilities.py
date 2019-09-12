import subprocess

from colorama import Fore, Style, deinit, init

def Task(name, *arguments):

    print("[{}] started".format(name))
    response = subprocess.run(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    init(autoreset=True)

    try:      
        response.check_returncode()
        print(Fore.GREEN +"[{}] completed successfully".format(name))

    except:
        print(Fore.RED + "[{}] failed".format(name))
        print(Fore.RED + response.stderr.decode('ascii'))

    deinit()