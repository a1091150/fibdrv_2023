reset
set xlabel 'F(n)'
set ylabel 'time(ns)'
set title 'Fibonacci runtime in userspace'
set term png
set output "ufib_picture.png"
set grid
plot [0:500][0:] \
"ufib_time.ut" using 1:2 with linespoints linewidth 2  pointtype 7 pointsize 0.5 title "fast doubling"