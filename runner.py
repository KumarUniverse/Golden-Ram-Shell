import sys
import os
import subprocess

if (sys.argv[1] == "grsh.out"):
    with open(sys.argv[2]) as fp:
        cmd = ["./grsh.out"]
        grsh = subprocess.run(cmd, stdin=fp, universal_newlines=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    fp.close()
    print(grsh.stdout.replace('grsh> ',''))
    print(grsh.stderr, file=sys.stderr)
else:
    with open(sys.argv[2]) as fp:
        cmd = ["python3","grsh.py"]
        grsh = subprocess.run(cmd, stdin=fp, universal_newlines=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    fp.close()
    print(grsh.stdout.replace('grsh> ',''))
    print(grsh.stderr, file=sys.stderr)
