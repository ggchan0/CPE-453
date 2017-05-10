import sys

valid_algs = ["FIFO", "RR", "SRJN"]

class Process():
    arrival_time = 0
    burst_time = 0
    time_remaining = 0
    turnaround_time = -1
    waiting_time = -1
    response_time = -1
    job_no = -1

    def __init__(self, arrival_time, burst_time):
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.time_remaining = burst_time
        self.turnaround_time = -1
        self.waiting_time = -1
        self.response_time = -1

    def __str__(self):
        return (str(self.burst_time) + " " + str(self.arrival_time))

def getAlgorithm(args):
    alg = "FIFO"
    try:
        index = args.index("-p")
    except:
        return alg
    if index > -1 and len(args) > index + 1:
        try:
            alg = args[index + 1]
        except:
            return alg
        if alg not in valid_algs:
            alg = "FIFO"
    return alg

def getQuantum(args):
    quantum = 1
    try:
        index = args.index("-q")
    except:
        return quantum
    if index > -1 and len(args) > index + 1:
        try:
            quantum = int(args[index + 1])
        except:
            return quantum
        if quantum < 1:
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
    while procs_left > 0:
        proc_scheduled = False
        for proc in procs:
            if proc.arrival_time <= time and proc.time_remaining > 0:
                proc.waiting_time = time - proc.arrival_time
                proc.response_time = proc.waiting_time + 1
                time += proc.burst_time
                proc.turnaround_time = time - proc.arrival_time
                proc.time_remaining = 0
                procs_left -= 1
                proc_scheduled = True
        if proc_scheduled == False:
            time += 1
    return procs

def executeRR(procs, quantum):
    time = procs[0].arrival_time
    procs_left = len(procs)
    last_proc = -1
    streak = 0
    while procs_left > 0:
        proc_scheduled = False
        for proc in procs:
            if proc.arrival_time <= time:
                if proc.time_remaining <= quantum and proc.time_remaining > 0:
                    if proc.response_time == -1:
                        proc.response_time = time - proc.arrival_time + 1
                    if last_proc != proc.job_no:
                        proc.waiting_time = time - proc.arrival_time
                    else:
                        proc.waiting_time = time - streak - proc.arrival_time
                    time += proc.time_remaining
                    proc.time_remaining = 0
                    procs_left -= 1
                    proc.turnaround_time = time - proc.arrival_time
                    last_proc = proc.job_no
                    proc_scheduled = True
                elif proc.time_remaining > 0:
                    if last_proc != proc.job_no:
                        streak = 0
                    streak += quantum
                    if proc.response_time == -1:
                        proc.response_time = time - proc.arrival_time + 1
                    time += quantum
                    proc.time_remaining -= quantum
                    last_proc = proc.job_no
                    proc_scheduled = True
        if proc_scheduled == False:
            time += 1
    return procs

def getProcToSchedule(procs, time):
    proc = -1
    least_time_remaining = sys.maxsize
    num_procs = len(procs)
    for i in range(0, num_procs):
        if procs[i].time_remaining < least_time_remaining \
            and procs[i].arrival_time <= time and procs[i].time_remaining != 0:
            proc = i
            least_time_remaining = procs[i].time_remaining
    return proc

def executeSRJN(procs):
    time = 0
    streak = 0
    procs_left = len(procs)
    last_proc = -1
    while (procs_left > 0):
        next_proc = getProcToSchedule(procs, time)
        proc = procs[next_proc]
        if next_proc == -1:
            time += 1
        else:
            if proc.response_time == -1:
                proc.response_time = time - proc.arrival_time + 1
            if last_proc == proc.job_no:
                streak += 1
            else:
                streak = 1
            last_proc = proc.job_no
            time += 1
            proc.time_remaining -= 1
            if proc.time_remaining == 0:
                proc.turnaround_time = time - proc.arrival_time
                proc.waiting_time = time - proc.arrival_time - streak
                procs_left -= 1
    return procs

def printProcStats(procs):
    for proc in procs:
        response = proc.response_time
        turnaround = proc.turnaround_time
        wait = proc.waiting_time
        print("Job %3d -- Response %3.2f Turnaround %3.2f Wait %3.2f" \
            % (proc.job_no, response, turnaround, wait))

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
    print("Average -- Response %3.2f Turnaround %3.2f Wait %3.2f" \
        % (avg_response, avg_turnaround, avg_wait))

def main():
    if len(sys.argv) < 2:
        print("Usage: python schedSim \
                <job-file.txt> -p <ALGORITHM> -q <QUANTUM>")
        sys.exit()
    algorithm = getAlgorithm(sys.argv)
    quantum = getQuantum(sys.argv)
    file = open(sys.argv[1], "r")
    procs = getSchedule(file)
    new_procs = sort_procs(procs)
    if algorithm == "FIFO":
        new_procs = executeFIFO(new_procs)
    elif algorithm == "RR":
        new_procs = executeRR(new_procs, quantum)
    else:
        new_procs = executeSRJN(new_procs)
    printProcStats(new_procs)
    printAverageStats(new_procs)

if __name__ == "__main__":
    main()
