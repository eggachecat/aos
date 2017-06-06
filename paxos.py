import os
import time
from multiprocessing import Process, Pipe

import numpy as np
import termcolor

class Accepter:
    def __init__(self):
        pass

class Proposer:
    def __init__(self):
        pass


def make_nodes():

    num_acc = 3
    num_pro = 2
    num_procs = num_acc + num_pro

    receiver_arr = []
    sender_arr = []
    pool = []

    for i in range(int(num_procs * num_procs)):
        receiver, sender = Pipe(False)
        receiver_arr.append(receiver)
        sender_arr.append(sender)

    print(sender_arr)

    for i in range(num_procs):
        senders = []
        receivers = []

        for j in range(num_procs):
            print(i + j * num_procs, i * num_procs + j)
            senders.append(sender_arr[i + j * num_procs])
            receivers.append(receiver_arr[j + i * num_procs])

        pool.append(Process(target=worker, args=(senders, receivers)))
        print("====")