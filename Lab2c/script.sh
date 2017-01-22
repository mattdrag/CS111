#!/bin/bash


echo "./lab2c --iterations=1000 --threads=1 --lists=8 "
(>&2 echo "./lab2c --iterations=1000 --threads=1 --lists=8")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=1 --lists=8
done
echo "./lab2c --iterations=1000 --threads=1 --lists=4 "
(>&2 echo "./lab2c --iterations=1000 --threads=1 --lists=4")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=1 --lists=4
done
echo "./lab2c --iterations=1000 --threads=1 --lists=2 "
(>&2 echo "./lab2c --iterations=1000 --threads=1 --lists=2")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=1 --lists=2
done
echo "./lab2c --iterations=1000 --threads=1 --lists=1 "
(>&2 echo "./lab2c --iterations=1000 --threads=1 --lists=1")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=1 --lists=1
done



echo "./lab2c --iterations=1000 --threads=8 --lists=64 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=64 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=64 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=32 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=32 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=32 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=16 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=16 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=16 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=8 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=8 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=8 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=4 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=4 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=4 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=2 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=2 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=2 --sync=m
done
echo "./lab2c --iterations=1000 --threads=8 --lists=1 --sync=m"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=1 --sync=m")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=1 --sync=m
done

echo "./lab2c --iterations=1000 --threads=8 --lists=64 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=64 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=64 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=32 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=32 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=32 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=16 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=16 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=16 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=8 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=8 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=8 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=4 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=4 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=4 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=2 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=2 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=2 --sync=s
done
echo "./lab2c --iterations=1000 --threads=8 --lists=1 --sync=s"
(>&2 echo "./lab2c --iterations=1000 --threads=8 --lists=1 --sync=s")
for i in `seq 1 11`; do
	./lab2c --iterations=1000 --threads=8 --lists=1 --sync=s
done
