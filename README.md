# system-monitor

Qt based replacement for the gnome system montior on Linux

## Compiling

Qmake can be used to directly build the project as long as all dependancies are met.

`qmake && make; make install`

The program uses C++14 and pthreads, g++ is recommended.

## Dependancies

Qt5 must be installed to use system monitor.

### Compiling

Compiling depends on the following:

* libprocps-dev

### Running

Running the program depends on the following:

* libprocps6

## References

* QCustomPlot - [http://qcustomplot.com/](http://qcustomplot.com/)

## Contributing

See [guide](CONTRIBUTING.md)
