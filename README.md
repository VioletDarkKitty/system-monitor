# system-monitor

Qt based replacement for the gnome system montior on Linux

## Compiling

Qmake can be used to directly build the project as long as all dependancies are met

Dependancies
------------

System monitor depends on libprocps, this can be installed on debian
based systems from the package libprocps-dev.

Please use libprocps6 is available. Versions lower than 2:2.3.12 are
known to cause sigsegv.

## References

* QCustomPlot - [http://qcustomplot.com/](http://qcustomplot.com/)
