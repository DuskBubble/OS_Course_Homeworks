import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results.csv')

df['CPU Utilization'] = df['CPU Utilization'].str.replace('%', '').astype(float)

metrics = ['Job Throughput','CPU Utilization','Average Turnaround Time','Average Response Time','Average Waiting Time']
algorithms = df['Algorithm']

plt.figure(figsize=(14, 10))

for i, metric in enumerate(metrics, 1):
    plt.subplot(3, 2, i)
    bars = plt.bar(algorithms, df[metric])
    plt.title(metric + (' (%)' if metric == 'CPU Utilization' else ''))
    plt.xticks(rotation=15)

    # Add Tags
    for bar in bars:
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height,
                 f'{height:.2f}' if metric != 'CPU Utilization' else f'{height:.1f}%',
                 ha='center', va='bottom')

plt.tight_layout()
plt.savefig('comparison.png', dpi=300)
plt.show()
