# old debian is needed to get cpython 2.7
FROM debian:10

MAINTAINER daru@dtu.dk

# general linux packages
RUN apt-get update
RUN apt install tmux time -y

# set up cpython 2.7
RUN apt install python python-pip -y
RUN pip install numpy

# set up pypy, needs virtualenv for numpy
# RUN bash ./setup_pypy.sh

CMD ["/bin/bash"]

