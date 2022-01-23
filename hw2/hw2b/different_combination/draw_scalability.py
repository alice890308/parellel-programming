import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from tqdm import tqdm
from subprocess import Popen, PIPE, STDOUT

filename = input("Please input the filename: ")
testcase = input("\nPlease input the testcase: ")

def print_command(p):
  for line in p.stdout:
    print(line.strip())

thread_num = [[2, 12], [3, 8], [4, 6], [6, 4], [8, 3], [12, 2]] # node, thread
testcases = {0: "174170376 -0.7894722222222222 -0.7825277777777778 0.145046875 0.148953125 2549 1439",
            1: "10000 -0.29899250664589705 -0.2772002993319328 -0.6327591639095336 -0.6433840614725646 7680 4320",
            2: "10000 -0.5 0.5 -0.5 0.5 491 935"}

testcase = testcases[int(testcase)]

p = Popen(["make clean"], shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
print_command(p)

p = Popen(["make"], shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
print_command(p)

result_cpu = list()
result_comm = list()
result_io = list()

for comb in tqdm(thread_num):
  cmd = f"srun -c{comb[1]} -n{comb[0]} ./hw2b experiment.png {testcase}"
  p = Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
  line = [i for i in p.stdout]
  result_cpu.append(float(line[0]))
  result_comm.append(float(line[1]))
  result_io.append(float(line[2]))
  num_thread = comb[0] * comb[1];
  print(f"\nThread num: {num_thread}")
  print(f"cpu time: {line[0]}")
  print(f"comm time: {line[1]}")
  print(f"io time: {line[2]}\n")

labels = [f"[{i[0]} * {i[1]}]" for i in thread_num]

# bar chart for different thread number's CPU, COMM, IO time
fig, ax = plt.subplots()
ax.bar(labels, result_io, label='IO_time')
ax.bar(labels, result_cpu, bottom=result_io, label='CPU_time')
ax.bar(labels, result_comm, bottom=np.array(result_cpu)+np.array(result_io), label='COMM_time')
ax.set_xlabel("Combination")
ax.set_ylabel("Runtime (seconds)")
ax.set_title(f"Time Profile")
ax.legend(loc="upper right")
fig.savefig(f"./images/{filename}_time_profile_bar.png")

# line chart for different thread number's CPU, COMM, IO time
fig, ax = plt.subplots()
total_time = [result_comm[i] for i in range(len(thread_num))]
ax.plot(labels, total_time, label='total_time')
ax.set_xlabel("Combination")
ax.set_ylabel("Communication Time (seconds)")
ax.set_title(f"Time Profile")
ax.legend(loc="upper right")
fig.savefig(f"./images/{filename}_time_profile_line.png")


# line seperate speedup factor
cpu_speedup = [total_time[0]/total_time[i] for i in range(len(thread_num))]
idea_speedup = [thread_num[i] for i in range(len(thread_num))]
fig, ax = plt.subplots()
ax.plot(labels, cpu_speedup, label='Speedup Time')
ax.plot(labels, idea_speedup, label='Ideal Time')
ax.set_xlabel("Combination")
ax.set_ylabel("Speedup")
ax.set_title(f"Speedup Time")
ax.legend(loc="upper right")
fig.savefig(f"./images/{filename}_speedup.png")

p = Popen(["make clean"], shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
print_command(p)