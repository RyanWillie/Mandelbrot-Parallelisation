#!/bin/bash
echo "========================================"
echo " Cleaning Up"
make clean > /dev/null
echo " Compiling programs"
make > /dev/null

echo "========================================"
echo " Running Serial Program"
time ./mb5 10000 -0.668 0.32 0.02 > /dev/null
echo "========================================"
echo " Running GNUPLOT"
gnuplot -p mandel.gp
mv mandel.png Images/mandelSerial.png
echo "========================================"
echo " Running Fork Program with 6 threads (Sockets)"
time ./mbfs 10000 -0.668 0.32 0.002 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot -p mandel.gp
mv mandel.png Images/mandelFork_Sockets.png

echo "========================================"
echo " Running Fork Program with 6 threads (Pipes)"
time ./mbfp 10000 -0.668 0.32 0.002 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot -p mandel.gp
mv mandel.png Images/mandelFork_Pipes.png

echo "========================================"
echo " Running PThreads Program with 6 threads"
time ./mbp 10000 -0.668 0.32 0.002 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot -p mandel.gp
mv mandel.png Images/mandelFork_PThreads.png


echo "========================================"
echo " Running OpenMP Program with 6 threads"
time ./mbomp 10000 -0.668 0.32 0.002 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot -p mandel.gp
mv mandel.png Images/mandelFork_OMP.png
