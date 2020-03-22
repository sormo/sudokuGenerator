import json
import matplotlib.pyplot as plt

def draw_plot(file_name, style):
    with open(file_name, 'r') as file:
        benchdata = json.load(file)

    x, y = [], []

    for test in benchdata['benchmarks']:
        name = test['name'].split('/')
        x.append(int(name[1]))
        y.append(float(test['real_time']))

    plt.plot(x, y, style)
    plt.xlabel('spaces')
    plt.ylabel('time in us')

draw_plot('benchmark-solve.json', 'r-')
draw_plot('benchmark-no-solve.json', 'b-')

plt.show()
