./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=15
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=30
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=60
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=90
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=120

./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=2 --interval=30
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=5 --interval=30
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=10 --interval=30
./run --routing=0 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=20 --interval=30

./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=15
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=30
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=60
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=90
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=0 --interval=120

./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=2 --interval=30
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=5 --interval=30
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=10 --interval=30
./run --routing=1 --cache=1 --verbose=1 --mdns=1 --ping=1 --speed=20 --interval=30

./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=15
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=30
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=60
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=90
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=120

./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=2 --interval=30
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=5 --interval=30
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=10 --interval=30
./run --routing=0 --cache=1 --verbose=1 --ping=1 --speed=20 --interval=30

./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=15
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=30
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=60
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=90
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=0 --interval=120

./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=2 --interval=30
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=5 --interval=30
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=10 --interval=30
./run --routing=1 --cache=1 --verbose=1 --ping=1 --speed=20 --interval=30

rm experiments.zip
zip experiments routing_*
rm routing_*
