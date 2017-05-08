from process import *
import sys

valid_algs = ["FIFO", "RR", "SRJN"]

def getAlgorithm(args):
    alg = "FIFO"
    try:
        index = args.index("-p")
    except:
        return alg
    if (index > -1 and len(args) > index + 1):
        alg = args[index + 1]
        if (alg not in valid_algs):
            alg = "FIFO"
    return alg

def getQuantum(args):
    quantum = 1
    try:
        index = args.index("-q")
    except:
        return quantum
    if (index > -1 and len(args) > index + 1):
        quantum = int(args[index + 1])
        if (quantum < 1):
            print("Cannot have quantum less than 1")
            sys.exit()
    return quantum

def getSchedule(file):
    procs = []
    for line in file:
        contents = line.split(" ")
        proc = Process(int(contents[1]), int(contents[0]))
        procs.append(proc)
    return procs

def main():
    if (len(sys.argv) < 2):
        print("Usage: schedSim <job-file.txt> -p <ALGORITHM> -q <QUANTUM>")
        sys.exit()
    algorithm = getAlgorithm(sys.argv)
    quantum = getQuantum(sys.argv)
    file = open(sys.argv[1], "r")
    procs = getSchedule(file)
    print("Algorithm " + algorithm)
    print("Quantum " + str(quantum))
    print(str(len(procs)))
    new_procs = sorted(procs, key=lambda x: x.arrival_time, reverse=False)
    for proc in new_procs:
        print(proc)

if __name__ == "__main__":
    main()
