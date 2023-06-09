import numpy as np
import matplotlib.pyplot as plt


if __name__ == "__main__":

    Ya = np.loadtxt('./txt/tlsf_bench_all_size.txt', dtype='float').T
    Yb = np.loadtxt('./txt/tlsf_bench_all_size_self.txt', dtype='float').T

    X = Ya[0]
    Ya = Ya[3]
    Yb = Yb[3]

    fig, ax = plt.subplots(1, 1, sharey = True)
    ax.set_title('tlsf bench', fontsize = 16)
    ax.set_xlabel('size ~ size + 64 (bytes)', fontsize = 16)
    ax.set_ylabel('avg time (us)', fontsize = 16)

    ax.plot(X, Ya, marker = '+', markersize = 3, label = 'arena grow/shrink')
    ax.plot(X, Yb, marker = '*', markersize = 3, label = 'tlsf init')
    ax.legend(loc = 'upper left')

    plt.savefig('./png/method_compare.png')