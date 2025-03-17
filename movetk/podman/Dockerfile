FROM fedora:35

# seems to take forever
# RUN dnf -y update

# install MoveTK dependencies
RUN dnf -y install \
    boost-devel \
    cmake \
    CGAL-devel \
    doxygen \
    gcc \
    gcc-c++ \
    gsl-devel\
    GeographicLib-devel \
    make \
    mpfr-devel \
    rpm-build \
 && dnf clean all

RUN dnf install tmux time git -y


CMD ["/bin/bash"]

