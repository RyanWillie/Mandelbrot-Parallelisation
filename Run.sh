#!/bin/bash
echo "========================================"
echo " Cleaning Up"
make clean > /dev/null
rm *.png
echo " Compiling programs"
make > /dev/null
echo "========================================"
echo " Running Serial Program"
time ./mb5 10000 -0.668 0.32 0.02 > /dev/null
echo "========================================"
echo " Running GNUPLOT"
gnuplot mandel.gp
mv mandel.png Images/mandelSerial.png
echo "========================================"
echo " Running Fork Program with 6 threads (Sockets)"
time ./mbfs 10000 -0.668 0.32 0.02 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot mandel.gp
mv mandel.png Images/mandelFork_Sockets.png

echo "========================================"
echo " Running Fork Program with 6 threads (Pipes)"
time ./mbfp 10000 -0.668 0.32 0.02 6 > /dev/null
echo "========================================"

echo " Running GNUPLOT"
gnuplot mandel.gp
mv mandel.png Images/mandelFork_Pipes.png