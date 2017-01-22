#!/bin/bash

echo "./lab2a --iterations=10 --threads=1"
(>&2 echo "./lab2a --iterations=10 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=10 --threads=1
done
echo "./lab2a --iterations=20 --threads=1"
(>&2 echo "./lab2a --iterations=20 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=20 --threads=1
done
echo "./lab2a --iterations=50 --threads=1"
(>&2 echo "./lab2a --iterations=50 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=50 --threads=1
done
echo "./lab2a --iterations=100 --threads=1"
(>&2 echo "./lab2a --iterations=100 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=100 --threads=1
done
echo "./lab2a --iterations=250 --threads=1"
(>&2 echo "./lab2a --iterations=250 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=250 --threads=1
done
echo "./lab2a --iterations=500 --threads=1"
(>&2 echo "./lab2a --iterations=500 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=500 --threads=1
done
echo "./lab2a --iterations=1000 --threads=1"
(>&2 echo "./lab2a --iterations=1000 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=1000 --threads=1
done
echo "./lab2a --iterations=5000 --threads=1"
(>&2 echo "./lab2a --iterations=5000 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=5000 --threads=1
done
echo "./lab2a --iterations=10000 --threads=1"
(>&2 echo "./lab2a --iterations=10000 --threads=1")
for i in `seq 1 11`; do
	./lab2a --iterations=10000 --threads=1
done
