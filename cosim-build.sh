#!/bin/bash
if [ "$COSIM_PATH" = "" ] ; then
  COSIM_PATH="/local/endhostsim/ehsim"
fi

export CPPFLAGS="-I$COSIM_PATH/lib"
export LDFLAGS="-L$COSIM_PATH/lib/simbricks/nicif/ -lnicif -L$COSIM_PATH/lib/simbricks/network/ -lnetwork -L$COSIM_PATH/lib/simbricks/base/ -lbase"
export CXXFLAGS="-Wno-range-loop-construct"
if [ "$1" = "configure" ] ; then
  ./waf configure --enable-examples
fi
./waf build --enable-examples
