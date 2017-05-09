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

def sort_procs(procs):
    procs = sorted(procs, key=lambda x: x.arrival_time, reverse=False)
    cur_job_no = 0
    for proc in procs:
        proc.job_no = cur_job_no
        cur_job_no += 1
    return procs

def executeFIFO(procs):
    time = 0
    procs_left = len(procs)
    while (procs_left > 0):
        proc_scheduled = False
        for proc in procs:
            if (proc.arrival_time <= time and proc.time_remaining > 0):
                proc.waiting_time = time
                proc.response_time = time + 1
                proc.turnaround_time = time + proc.burst_time - proc.arrival_time
                time += proc.burst_time
                procs_left -= 1
        if (proc_scheduled == False):
            time += 1
    return procs

def executeRR(procs, quantum):
    time = procs[0].arrival_time
    procs_left = len(procs)
    while (procs_left > 0):
        proc_scheduled = False
        for proc in procs:
            if (proc.arrival_time <= time):
                if (proc.time_remaining <= quantum and proc.time_remaining > 0):
                    if (proc.time_remaining == proc.burst_time):
                        proc.response_time = time + 1
                    time += proc.time_remaining
                    proc.time_remaining = 0
                    procs_left -= 1
                    proc.waiting_time = time - proc.burst_time - proc.arrival_time
                    proc.turnaround_time = time - proc.arrival_time
                    proc_scheduled = True
                elif (proc.time_remaining > 0):
                    if (proc.time_remaining == proc.burst_time):
                        proc.response_time = time + 1
                    time += quantum
                    proc.time_remaining -= quantum
                    proc_scheduled = True
        if (proc_scheduled == False):
            time += 1
    return procs

def executeSRJN(procs):
    time = 0
    procs_left = len(procs)
    procs = procs.sorted(procs, key = lambda x: x.burst_time, reverse = False)
    while (procs_left > 0):

    return procs

def printProcStats(procs):
    for proc in procs:
        response = proc.response_time
        turnaround = proc.turnaround_time
        wait = proc.waiting_time
        print("Job %3d -- Response %3.2f Turnaround %3.2f Wait %3.2f" % (proc.job_no, response, turnaround, wait))

def printAverageStats(procs):
    avg_response = 0.0
    avg_turnaround = 0.0
    avg_wait = 0.0
    proc_count = len(procs)
    for proc in procs:
        avg_response += proc.response_time
        avg_turnaround += proc.turnaround_time
        avg_wait += proc.waiting_time
    avg_response /= proc_count
    avg_turnaround /= proc_count
    avg_wait /= proc_count
    print("Average -- Response %3.2f Turnaround %3.2f Wait %3.2f" % (avg_response, avg_turnaround, avg_wait))

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
    new_procs = sort_procs(procs)
    if (algorithm == "FIFO"):
        new_procs = executeFIFO(new_procs)
    elif (algorithm == "RR"):
        new_procs = executeRR(new_procs, quantum)
    else:
        new_procs = executeSJRN(new_procs)
    printProcStats(new_procs)
    printAverageStats(new_procs)

if __name__ == "__main__":
    main()
