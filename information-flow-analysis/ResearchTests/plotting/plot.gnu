set key tmargin top horiz 
set term postscript color #font "Times New Roman"
set output "performance.eps"
set yrange [0:1]
set xrange [0:1]
set xlabel '% codes covered'
set ylabel '% faults located'
set size 1,1

plot \
      "node_const.plot" using 1:2 with lp title "Pr(n) = 0.2" \
     ,"node_freq.plot" using 1:2 with lp title "Pr(n) = freq(n)" \
     ,"edge_const.plot" using 1:2 with lp title "Pr(e) = 0.2" \
     ,"edge_freq.plot" using 1:2 with lp title "Pr(e) = freq(e)" \
#     ,"node_const_x2.plot" using 1:2 with lp title "Pr(n) = 0.2, no p2" \
#     ,"node_const_x3.plot" using 1:2 with lp title "Pr(n) = 0.2, no p3" \

!evince performance.eps
#!epstopdf performance.eps
