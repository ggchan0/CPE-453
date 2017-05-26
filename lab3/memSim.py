import sys
import os.path
import collections
import operator

algs = ["FIFO", "OPT", "LRU"]
backing_store = "BACKING_STORE.bin"

class Page:
    def __init__(self, page_no, frame_no, frame):
        self.page_no = page_no
        self.frame_no = frame_no
        self.frame = frame

class FIFOPageTable:
    def __init__(self, frame_count):
        self.frame_count = frame_count
        self.pages = []

    def get(self, page_no):
        for page in self.pages:
            if page.page_no == page_no:
                return page
        return None

    def set(self, frame):
        if len(self.pages) == self.frame_count:
            self.pages.pop()
        self.pages.insert(0, frame)

    def pop(self, paging_count):
        return self.pages.pop()

    def isFull(self):
        return len(self.pages) == self.frame_count

class OPTPageTable:
    def __init__(self, frame_count, addresses):
        self.frame_count = frame_count
        self.addresses = addresses
        self.pages = []

    def get(self, page_no):
        for page in self.pages:
            if page.page_no == page_no:
                return page
        return None

    def set(self, frame):
        if (len(self.pages)) >= self.frame_count:
            self.pages.pop()
        self.pages.insert(0, frame)

    def isFull(self):
        return len(self.pages) == self.frame_count

    def pop(self, paging_count):
        opt_entries = {}
        victim_no = -1
        for page in self.pages:
            opt_entries[page.page_no] = sys.maxint
        for i in range(paging_count, len(self.addresses)):
            page_no, offset = get_page_no_and_offset(self.addresses[i])
            opt_entries[page_no] = i
        sorted_opt_entries = sorted(opt_entries.items(), key = operator.itemgetter(1), reverse=True)
        victim_page_no = sorted_opt_entries[0][0]
        for i in range(0, len(self.pages)):
            if self.pages[i].page_no == victim_page_no:
                victim_no = i
                break
        return self.pages.pop(victim_no)

class LRUPageTable:
    def __init__(self, frame_count):
        self.frame_count = frame_count
        self.pages = []

    def get(self, page_no):
        page = None
        for i in range(0, len(self.pages)):
            if self.pages[i].page_no == page_no:
                page = self.pages.pop(i)
                self.pages.insert(0, page)
                break
        return page

    def set(self, frame):
        if (len(self.pages)) >= self.frame_count:
            self.pages.pop()
        self.pages.insert(0, frame)

    def pop(self, paging_count):
        return self.pages.pop()

    def isFull(self):
        return len(self.pages) == self.frame_count

class VirtualMemory:

    def __init__(self, pra, num_frames, addresses):
        self.pra = pra
        self.num_frames = num_frames
        self.num_page_faults = 0
        self.num_page_hits = 0
        self.cur_page = 0
        self.tlb_hits = 0
        self.tlb_misses = 0
        self.addresses = addresses
        self.mem = []
        self.paging_count = 0
        for i in range(0, num_frames):
            self.mem.append(None)
        self.tlb = FIFOPageTable(16)
        if (pra == "LRU"):
            self.page_table = LRUPageTable(num_frames)
        elif (pra == "OPT"):
            self.page_table = OPTPageTable(num_frames, addresses)
        else:
            self.page_table = FIFOPageTable(num_frames)

    def handlePageFault(self, page_no, offset):
        self.num_page_faults += 1
        frame = readFromBin(page_no)
        page = None
        if self.page_table.isFull():
            temp_page = self.page_table.pop(self.paging_count)
            page = Page(page_no, temp_page.frame_no, frame)
        else:
            page = Page(page_no, self.cur_page, frame)
            self.cur_page += 1
        self.page_table.set(page)
        self.mem[page.frame_no] = page.frame
        self.tlb.set(page)
        return page.frame_no

    def process(self, address):
        page_no, offset = get_page_no_and_offset(address)
        page = self.tlb.get(page_no)
        if page == None or page.frame != self.mem[page.frame_no]:
            page = None
        if page == None:
            self.tlb_misses += 1
            page = self.page_table.get(page_no)
            if page == None:
                new_page_no = self.handlePageFault(page_no, offset)
                page = self.page_table.get(page_no)
            else:
                self.num_page_hits += 1
                self.tlb.set(page)
        else:
            self.num_page_hits += 1
            self.tlb_hits += 1
        self.printData(address, convertToHex(page.frame[offset]), page.frame_no, page.frame)
        self.paging_count += 1

    def runSimulator(self):
        for address in self.addresses:
            frame = self.process(address)
        self.printStats()

    def printData(self, address, data, page_no, frame):
        print("%d, %s, %d,\n%s" % (address, data, page_no, frame.encode("hex").upper()))

    def printStats(self):
        print("Number of Translated Address = %d" % (self.num_page_hits + self.num_page_faults))
        print("Page Faults = %d" % (self.num_page_faults))
        print("Page Fault Rate = %3.3f" % (self.num_page_faults * 1.0 / (self.num_page_faults + self.num_page_hits)))
        print("TLB Hits = %d" % self.tlb_hits)
        print("TLB Misses = %d" % self.tlb_misses)
        print("TLB Hit Rate = %3.3f" % (self.tlb_hits * 1.0 / (self.tlb_hits + self.tlb_misses)))

def readFromBin(frame_no):
    file = open(backing_store, 'rb')
    frame_size = 256
    file.seek(frame_size * frame_no)
    return file.read(frame_size)

def convertToHex(c):
    num = int(c.encode('hex'), 16)
    if num > 127:
        num -= 256
    return num

#page no, offset
def get_page_no_and_offset(address):
    return (address >> 8), (address & 0xFF)

def pages_equal(page1, page2):
    return page1.page_no == page2.page_no and page1.frame_no == page2.frame_no

def parseArgs(args):
    frames = 256
    pra = "FIFO"
    if len(args) < 2:
        print("File name required")
        sys.exit()
    file_name = args[1]
    if len(args) >= 3:
        try:
            frames = int(args[2])
            if frames <= 0 or frames > 256:
                frames = 256
        except:
            print("Integer must be given for the frame #")
            sys.exit()
    if len(args) >= 4:
        if args[3] in algs:
            pra = args[3]
    return file_name, frames, pra

def main():
    if not os.path.isfile(backing_store):
        print("BACKING_STORE.bin not found, quitting")
        sys.exit()

    file_name, frames, pra = parseArgs(sys.argv)
    if not os.path.isfile(file_name):
        print("Reference file not found, quitting")
        sys.exit()

    file = open(file_name, "r")
    addresses = []
    for line in file:
        addresses.append(int(line))
    vm = VirtualMemory(pra, frames, addresses)
    vm.runSimulator()

if __name__ == '__main__':
    main()
