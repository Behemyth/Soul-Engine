import subprocess

def Task(*arguments):

    response = subprocess.run(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        response.check_returncode()
    except:
        print(response.stderr.decode('ascii'))