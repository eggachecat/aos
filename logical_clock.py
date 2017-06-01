import os
import time
from multiprocessing import Process, Pipe

import numpy as np
import termcolor


def send_to_all(_msg, senders):
    for sender in senders:
        sender.send(_msg)


terminal_color = ["red", "green", "yellow"]


def worker(senders, receivers):
    pid = os.getpid()

    print(pid)

    color = terminal_color[pid % 3]

    np.random.seed(pid % 997)

    logical_ctr = np.random.randint(0, 4)

    processes_queue = []
    operation_queue = []
    operation_ack_dict = dict()

    _msg = {
        "pid": pid,
        "type": 0
    }
    send_to_all(_msg, senders)

    for _ in range(2):

        logical_ctr += 1
        op_id = np.random.randint(0, 1000000000)
        # rd = np.random.rand()
        _msg = {
            "pid": pid,
            "type": 1,
            "msg": op_id,
            "ts": logical_ctr
        }

        send_to_all(_msg, senders)


    time.sleep(1)

    for _ in range(10):
        # logical_ctr += 1
        # op_id = np.random.randint(0, 1000000000)
        # rd = np.random.rand()
        # if rd > 0.6:
        #     _msg = {
        #         "pid": pid,
        #         "type": 1,
        #         "msg": op_id,
        #         "ts": logical_ctr
        #     }
        #
        #     send_to_all(_msg, senders)

        for receiver in receivers:

            try:
                msg = receiver.recv()

                flag = msg["type"]
                if flag == 0:
                    if msg["pid"] not in processes_queue:
                        processes_queue.append(msg["pid"])
                elif flag == 1:

                    info = "Process #{pid} received request from #{src} '{msg}' with timestamp={ts} ".format(pid=pid,
                                                                                                             src=msg[
                                                                                                                 "pid"],
                                                                                                             msg=msg[
                                                                                                                 "msg"],
                                                                                                             ts=msg[
                                                                                                                 "ts"])

                    print(termcolor.colored(info, color))

                    operation_queue.append(msg)

                    operation_queue = sorted(operation_queue, key=lambda k: k["ts"])

                    info = "Process #{pid} ready-queue #{q}".format(pid=pid, q=[x["msg"] for x in operation_queue])
                    print(termcolor.colored(info, color))

                    operation_ack_dict[msg["msg"]] = []

                    _msg = {
                        "pid": pid,
                        "type": 2,
                        "msg": msg["msg"]
                    }

                    send_to_all(_msg, senders)
                elif flag == 2:

                    operation_ack_dict[msg["msg"]].append(msg["pid"])

                    info = "Process #{pid} received ack from #{src} '{msg}' ".format(pid=pid, src=msg["pid"],
                                                                                     msg=msg["msg"])
                    print(termcolor.colored(info, color))

                    head = operation_queue[0]["msg"]
                    #
                    # info = "{stat_1}, {stat_2}".format(stat_1=set(operation_ack_dict[head]), stat_2=set(processes_queue))
                    # print(termcolor.colored(info, color))

                    if set(operation_ack_dict[head]) == set(processes_queue):
                        info = "Process #{pid} ready-queue head {h} received enough ack!".format(pid=pid, h=msg["msg"])
                        print(termcolor.colored(info, color))

                        operation_queue.pop(0)
                        operation_ack_dict[head] = None

                else:
                    pass
            except:
                pass


if __name__ == '__main__':
    receiver_1, sender_1 = Pipe(False)
    receiver_2, sender_2, = Pipe(False)

    num_procs = 3

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

    for proc in pool:
        proc.start()

    for proc in pool:
        proc.join()

        # p_1 = Process(target=worker, args=(sender_1, receiver_2))
        # p_2 = Process(target=worker, args=(sender_2, receiver_1))
        #
        # p_2.start()
        # p_1.start()


        # print(parent_conn.recv())  # prints "[42, None, 'hello']"
        # p.join()


# from multiprocessing import Process, Pipe
# import os
#
# def info(title):
#     print(title, 'module name:', __name__, 'parent process:', os.getppid(), 'process id:', os.getpid())
#
# def f(name):
#     info('function f')
#     print('hello', name)
#
# if __name__ == '__main__':
#     info('main line')
#     p_1 = Process(target=f, args=('sunao',))
#     p_1.start()
#
#     p_2 = Process(target=f, args=('chenhao',))
#     p_2.start()
#
#     while True:
#         pass

# p_1.join()
# p_1.join()


# import _thread
# import time
#
#
# # Define a function for the thread
# def print_time(threadName, delay):
#     count = 0
#     while count < 5:
#         time.sleep(delay)
#         count += 1
#         print("%s: %s" % (threadName, time.ctime(time.time())))
#
#
# # Create two threads as follows
# try:
#     _thread.start_new_thread(print_time, ("Thread-1", 2,))
#     _thread.start_new_thread(print_time, ("Thread-2", 4,))
# except:
#     print("Error: unable to start thread")
#
# while 1:
#     pass
