./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=1 --stime=1 --mdns=0 --speed=0 --interval=30                                                                      
./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=1 --stime=1 --mdns=1 --speed=0 --interval=30

# OLSR MCAST NOCACHE MDNS
./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=1 --stime=0 --mdns=1 --speed=0 --interval=30
./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=1 --stime=0 --mdns=0 --speed=0 --interval=30

# OLSR MCAST NOCACHE MDNS
./run --routing=2 --ping=1 --verbose=1 --cache=0 --mcast=1 --stime=0 --mdns=1 --speed=0 --interval=30
./run --routing=2 --ping=1 --verbose=1 --cache=0 --mcast=1 --stime=0 --mdns=0 --speed=0 --interval=30

./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=0 --stime=0 --mdns=1 --speed=0 --interval=30
./run --routing=2 --ping=1 --verbose=1 --cache=1 --mcast=0 --stime=0 --mdns=0 --speed=0 --interval=30

./run --routing=2 --ping=1 --verbose=1 --cache=0 --mcast=0 --stime=0 --mdns=1 --speed=0 --interval=30
./run --routing=2 --ping=1 --verbose=1 --cache=0 --mcast=0 --stime=0 --mdns=0 --speed=0 --interval=30
