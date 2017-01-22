#!/bin/bash

echo "./lab2n --iterations=10 --threads=1"
(>&2 echo "./lab2b --iterations=10 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=10 --threads=1
done
echo "./lab2b --iterations=20 --threads=1"
(>&2 echo "./lab2b --iterations=20 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=20 --threads=1
done
echo "./lab2b --iterations=50 --threads=1"
(>&2 echo "./lab2b --iterations=50 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=50 --threads=1
done
echo "./lab2b --iterations=100 --threads=1"
(>&2 echo "./lab2b --iterations=100 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=100 --threads=1
done
echo "./lab2b --iterations=250 --threads=1"
(>&2 echo "./lab2b --iterations=250 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=250 --threads=1
done
echo "./lab2b --iterations=500 --threads=1"
(>&2 echo "./lab2b --iterations=500 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=500 --threads=1
done
echo "./lab2b --iterations=1000 --threads=1"
(>&2 echo "./lab2b --iterations=1000 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=1000 --threads=1
done
echo "./lab2b --iterations=2000 --threads=1"
(>&2 echo "./lab2b --iterations=2000 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=2000 --threads=1
done
echo "./lab2b --iterations=5000 --threads=1"
(>&2 echo "./lab2b --iterations=5000 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=5000 --threads=1
done
echo "./lab2b --iterations=10000 --threads=1"
(>&2 echo "./lab2b --iterations=10000 --threads=1")
for i in `seq 1 11`; do
	./lab2b --iterations=10000 --threads=1
done
