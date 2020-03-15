import json
import matplotlib.pyplot as plt

with open('benchmark.json') as file:
    benchdata = json.load(file)

data = {}

for test in benchdata['benchmarks']:
    name = test['name'].split('/')
    d = data.get(name[1], [])
    if len(d) == 0:
        d.extend([[], []])

    d[0].append(int(name[2]))
    d[1].append(float(test['real_time']))

    data[name[1]] = d

l = []
for solutions, d in data.items():
    # if solutions == '5':
    #     continue

    plt.plot(d[0], d[1])
    l.append(solutions)
plt.xlabel('spaces')
plt.ylabel('time in us')
plt.legend(l)
plt.show()
