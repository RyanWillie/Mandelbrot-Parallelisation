#!/bin/bash
echo "========================================"
echo " Cleaning Up"
make clean > /dev/null
echo " Compiling programs"
make > /dev/null
echo "========================================"
echo "1. Mandelbrot Serial Program"
echo "2. Mandelbrot Fork Program using Sockets"
echo "3. Mandelbrot Fork Program using pipes"
echo "4. Mandelbrot PThreads program"
echo "5. Mandelbrot OpenMP Program"
echo "========================================"
echo "What program would you like to run? "
read x

echo "You Typed ${x}"
if [ $x == 1 ]
then
    echo "========================================"
    echo " Running Serial Program"
    time ./mb5 10000 -0.668 0.32 0.02 > /dev/null
    echo "========================================"
    echo " Running GNUPLOT"
    gnuplot -p mandel.gp
    mv mandel.png Images/mandelSerial.png
elif [ $x == 2 ]
then
    echo "========================================"
    echo " Running Fork Program with 6 threads (Sockets)"
    time ./mbfs 10000 -0.668 0.32 0.002 6 > /dev/null
    echo "========================================"

    echo " Running GNUPLOT"
    gnuplot -p mandel.gp
    mv mandel.png Images/mandelFork_Sockets.png

elif [ $x == 3 ]
then
    echo "========================================"
    echo " Running Fork Program with 6 threads (Pipes)"
    time ./mbfp 10000 -0.668 0.32 0.002 6 > /dev/null
    echo "========================================"

    echo " Running GNUPLOT"
    gnuplot -p mandel.gp
    mv mandel.png Images/mandelFork_Pipes.png
elif [ $x == 4 ]
then
    echo "========================================"
    echo " Running PThreads Program with 6 threads"
    time ./mbp 10000 -0.668 0.32 0.002 6 > /dev/null
    echo "========================================"

    echo " Running GNUPLOT"
    gnuplot -p mandel.gp
    mv mandel.png Images/mandelFork_PThreads.png
elif [ $x == 5 ]
then
    echo "========================================"
    echo " Running OpenMP Program with 6 threads"
    time ./mbomp 10000 -0.668 0.32 0.002 6 > /dev/null
    echo "========================================"

    echo " Running GNUPLOT"
    gnuplot -p mandel.gp
    mv mandel.png Images/mandelFork_OMP.png
fi
