#!/bin/bash

# Script to cleanup lc3tools' lc3sim dump from a screenlog made with screen -L
#
# Usage :
#   - Compile target program with lc3tools' lc3as
#   - screen -L
#   - load file, run file, perform input, etc...
#   - on computer HALT, dump entire memory (to be adjusted)
#   - after dump, quit lc3sim using quit imediately
#   - exit screen
#   - run this script
#   - move screenlog.0 to appropriate dump file

sed -i '1,/(lc3sim) dump x0000 xffff/d' screenlog.0
sed -ni '/(lc3sim) quit/q;p' screenlog.0

