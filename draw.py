import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv("nbody_2d.csv")

# łączny pęd
df['px_total'] = df['px0'] + df['px1'] + df['px2']
df['py_total'] = df['py0'] + df['py1'] + df['py2']

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# trajektorie
colors = ['steelblue', 'tomato', 'seagreen']
for i, color in enumerate(colors):
    ax1.plot(df[f'x{i}'], df[f'y{i}'], color=color, alpha=0.7, label=f'Ciało {i+1}')
    ax1.plot(df[f'x{i}'].iloc[0], df[f'y{i}'].iloc[0], 'o', color=color, markersize=8)
ax1.set_title("Trajektorie")
ax1.legend()
ax1.axis("equal")

# pęd
ax2.plot(df['t'], df['px_total'], label='px łączny')
ax2.plot(df['t'], df['py_total'], label='py łączny')
ax2.set_title("Zachowanie pędu")
ax2.legend()

plt.tight_layout()
plt.show()