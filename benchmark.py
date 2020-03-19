import json
import matplotlib.pyplot as plt

with open('benchmark.json') as file:
    benchdata = json.load(file)

x, y = [], []

for test in benchdata['benchmarks']:
    name = test['name'].split('/')
    x.append(int(name[1]))
    y.append(float(test['real_time']))

plt.plot(x, y)
plt.xlabel('spaces')
plt.ylabel('time in us')
plt.show()
