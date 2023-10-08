import matplotlib.pyplot as plt
import seaborn as sns

file = open('new2.txt', 'r')
fileLines = file.readlines()
proclist = [4, 5, 6, 7, 8]

data_lists = [[] for _ in range(len(proclist))]

for line in fileLines:
    for i, num in enumerate(proclist):
        if f"{num} runble" in line or f"{num} run" in line:
            data_lists[i].append(int(line.split()[-4]))

for i, data_list in enumerate(data_lists):
    sns.lineplot(data=data_list, label=f"Process {i+1}")

plt.xlabel("Number of Ticks", fontsize=20)
plt.ylabel("Queue Number", fontsize=20)
plt.text(0.01, 0.95, "Aging time: 50 ticks", transform=plt.gca().transAxes,
         fontsize=20)

plt.gcf().set_size_inches(22, 15)
plt.legend(loc='lower right', fontsize=20)
plt.savefig("plot.png")
