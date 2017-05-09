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
