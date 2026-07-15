#!/usr/bin/env bash
set -euo pipefail

cat <<'LIST'
Project1/assignment1          POSIX threads + pipes file-copy pipeline
Project1/assignment2          pthread worker scheduler / prime-style workload
Project1/assignment3          pthread binary-file sorting utility
Project2/assignment1          custom semaphore library using System V semaphores
Project2/assignment2          semaphore-based worker scheduler
Project2/assignment3          semaphore-based one-lane bridge / car coordination
Project2/assignment4          semaphore-based train/passenger coordination
Project3/assignment1          revised custom semaphore library
Project3/assignment2          pthread worker scheduler
Project3/assignment2_java     Java master/worker implementation
Project3/assignment3          pthread one-lane bridge / car coordination
Project3/assignment4          pthread train/passenger coordination
Project4/list                 intrusive list helper for coroutine/thread runtime
Project4/assignment1          user-level coroutine runtime
Project4/assignment2          preemptive user-level thread library
Project4/assignment3          readers/writers on the user-level thread library
Project4/project1_assignment1 user-level-thread file-copy pipeline
Project4/project2_assignment2 user-level-thread worker scheduler
Project4/project2_assignment3 user-level-thread bridge / car coordination
Project4/project2_assignment4 user-level-thread train/passenger coordination
LIST
