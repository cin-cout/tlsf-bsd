import numpy as np
import matplotlib.pyplot as plt


if __name__ == "__main__":

    Ya = np.loadtxt('./txt/tlsf_bench_range_all.txt', dtype='float').T
    Yb = np.loadtxt('./txt/tlsf_bench_range_big.txt', dtype='float').T
    Ys = np.loadtxt('./txt/tlsf_bench_range_small.txt', dtype='float').T

    Ya = Ya[3]
    Yb = Yb[3]
    Ys = Ys[3]

    X = np.arange(1, 51)

    fig, ax = plt.subplots(1, 1, sharey = True)
    ax.set_title('tlsf bench', fontsize = 16)
    ax.set_xlabel('execute time', fontsize = 16)
    ax.set_ylabel('avg time (us)', fontsize = 16)

    ax.plot(X, Ya, marker = '+', markersize = 3, label = '0 ~ 16384 bytes')
    ax.plot(X, Yb, marker = '*', markersize = 3, label = '8192 ~ 16384 bytes')
    ax.plot(X, Ys, marker = '.', markersize = 3, label = '0 ~ 8192 bytes')
    ax.legend(loc = 'upper left')

    plt.savefig('/png/range_compare.png')