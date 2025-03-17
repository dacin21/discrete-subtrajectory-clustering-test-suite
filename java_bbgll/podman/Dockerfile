# somehow this is the only image that has jfx for java 8.
FROM docker.io/library/amazoncorretto:8

RUN yum install maven -y

RUN yum install time -y

# FROM docker.io/library/maven:3.5.2
# FROM docker.io/library/eclipse-temurin:8-jdk-jammy
# FROM eclipse-temurin:17-jdk-jammy

# refresh package list, needed to install stuff with apt
# RUN apt-get update


CMD ["/bin/bash"]

