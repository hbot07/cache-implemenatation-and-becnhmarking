import matplotlib.pyplot as plt
import subprocess
import concurrent.futures

# Create a ThreadPoolExecutor with the desired number of worker threads
executor = concurrent.futures.ThreadPoolExecutor()


# Function to simulate cache and return total access time
def simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace_number=1):
    # Implement your cache simulation logic here
    # ...
    # Return the total access time
    # Run a shell command and capture its output
    command = f"./'COL216 Cache'/sim_cache {blockSize} {L1Size} {L1Assoc} {L2Size} {L2Assoc} 'COL216 Cache'/memory_trace_files/trace{trace_number}.txt"
    output = subprocess.check_output(command, shell=True)

    # Print the output
    # print(output)
    # print(int(output.decode().split()[-1]))

    return int(output.decode().split()[-1])


# Default parameter values
blockSize = 64
L1Size = 1024
L1Assoc = 2
L2Size = 65536
L2Assoc = 8

# Configure the plots
fig, axs = plt.subplots(5, 1, figsize=(15, 30))
print(subprocess.check_output("ls -l", shell=True).decode())

start_trace = 5
end_trace = 8


def graph1(trace_no):
    # Graph 1: Vary block size
    blockSizes = [8, 16, 32, 64, 128]
    for trace in range(trace_no, trace_no + 1):
        accessTimes = []
        for size in blockSizes:
            blockSize = size
            accessTime = simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace)
            accessTimes.append(accessTime)
        axs[0].plot(blockSizes, accessTimes, label=f"Trace {trace}")
    axs[0].set_xlabel("Block Size")
    axs[0].set_ylabel("Total Access Time")
    axs[0].set_title("Graph 1: Vary Block Size")
    axs[0].legend()


def graph2(trace_no):
    # Graph 2: Vary L1 size
    L1Sizes = [512, 1024, 2048, 4096, 8192]
    for trace in range(trace_no, trace_no + 1):
        accessTimes = []
        for size in L1Sizes:
            L1Size = size
            accessTime = simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace)
            accessTimes.append(accessTime)
        axs[1].plot(L1Sizes, accessTimes, label=f"Trace {trace}")
    axs[1].set_xlabel("L1 Size")
    axs[1].set_ylabel("Total Access Time")
    axs[1].set_title("Graph 2: Vary L1 Size")
    axs[1].legend()


def graph3(trace_no):
    # Graph 3: Vary L1 associativity
    L1AssocValues = [1, 2, 4, 8, 16]
    for trace in range(trace_no, trace_no + 1):
        accessTimes = []
        for assoc in L1AssocValues:
            L1Assoc = assoc
            accessTime = simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace)
            accessTimes.append(accessTime)
        axs[2].plot(L1AssocValues, accessTimes, label=f"Trace {trace}")
    axs[2].set_xlabel("L1 Associativity")
    axs[2].set_ylabel("Total Access Time")
    axs[2].set_title("Graph 3: Vary L1 Associativity")
    axs[2].legend()


def graph4(trace_no):
    # Graph 4: Vary L2 size
    L2Sizes = [16384, 32768, 65536, 131072, 262144]
    for trace in range(trace_no, trace_no + 1):
        accessTimes = []
        for size in L2Sizes:
            L2Size = size
            accessTime = simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace)
            accessTimes.append(accessTime)
        axs[3].plot(L2Sizes, accessTimes, label=f"Trace {trace}")
    axs[3].set_xlabel("L2 Size")
    axs[3].set_ylabel("Total Access Time")
    axs[3].set_title("Graph 4: Vary L2 Size")
    axs[3].legend()


def graph5(trace_no):
    # Graph 5: Vary L2 associativity
    L2AssocValues = [1, 2, 4, 8, 16]
    for trace in range(trace_no, trace_no + 1):
        accessTimes = []
        for assoc in L2AssocValues:
            L2Assoc = assoc
            accessTime = simulateCache(blockSize, L1Size, L1Assoc, L2Size, L2Assoc, trace)
            accessTimes.append(accessTime)
        axs[4].plot(L2AssocValues, accessTimes, label=f"Trace {trace}")
    axs[4].set_xlabel("L2 Associativity")
    axs[4].set_ylabel("Total Access Time")
    axs[4].set_title("Graph 5: Vary L2 Associativity")
    axs[4].legend()


# Submit the functions as tasks to the executor
futures = []

for i in range(start_trace, end_trace+1):
    futures += [executor.submit(graph1, i), executor.submit(graph2, i), executor.submit(graph3, i),
                executor.submit(graph4, i), executor.submit(graph5, i)]

# Wait for all tasks to complete
concurrent.futures.wait(futures)

# Adjust spacing between subplots
plt.tight_layout()

# Show the plots
# plt.show()
plt.savefig("plot.pdf")
