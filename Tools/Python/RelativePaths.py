import argparse
import os
from pathlib import Path, PurePosixPath, PurePath

#TODO: Create API for this module if imported

def _main():
    parser = argparse.ArgumentParser()

    #Commandline arguments
    parser.add_argument("path", help="The root path to list relative files and directories for")
    parser.add_argument('-i','--ignores', nargs='*', help='List of file names to ignore')
    args = parser.parse_args()

    #Find the relative files and directories
    relativePaths = set()

    for fileDir, _, files in os.walk(args.path):
        for fileName in files:

            if fileName in args.ignores:
                continue

            relativeDirectory = Path(os.path.relpath(fileDir, args.path))
            relativeFile = relativeDirectory / fileName
            relativePaths.add(PurePosixPath(relativeFile))

    relativePaths = sorted(relativePaths)

    for path in relativePaths:
        print(path)

    
if __name__== "__main__":
    _main()