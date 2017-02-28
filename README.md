# ns3-coAP

##

Here we include three parts of our research:

- A partial coAP implementation with several modifications like mDNS support and periodical multicast coAP discovery request.
- A simple multicast forwarding (SMF) algorithm using classic flooding (the simplest option)
- A script that performs the simulation with several modificable parameters

# How to work with git

I'm the only one commitiing so I just do:
```
git add .
git commit
git push origin master
```

and 

```
git pull origin master

```

# How to run things

`make all` will make the modified modules again
`./run` is some sort of "wrapper" around waf that runs a simulation and package all the results into a compressed file.

Thats and example about the usage of this script:
```
./run --routing=0 --verbose=1 --ping=0 --speed=40  --interval=15
```

We performed several simulations. That's an example of some of them:
```
// Playing with speed and interval
./run --routing=0 --verbose=1 --ping=0 --speed=40  --interval=15

// Using mcast responses
./run --routing=0 --verbose=1 --mcast=1 --ping=0 --speed=40  --interval=15

// Using mcast responses and pinging the discvovered services, using mdns
./run --routing=0 --verbose=1 --mcast=1 --ping=1 --mdns=1 --speed=40  --interval=15

// Using mcast responses but, without sending cache
./run --routing=0 --verbose=1 --mcast=1 --cache=0 --speed=40  --interval=15

// Using mcast responses with a variable response time (pdp)
./run --routing=0 --verbose=1 --mcast=1 --cache=1 --stime=1 --speed=40  --interval=15

// Using mcast responses with a variable response time (pdp) and with a etag to avoid unnecesary data transmission
./run --routing=0 --verbose=1 --mcast=1 --cache=1 --stime=1  --etag=1 --speed=40  --interval=15

//Sometimes we would like to make experiments shorter to evaluate some aspects of the simulation
./run --routing=0 --verbose=1 --ping=0 --speed=40  --interval=15  --runtime=60

```
# Acknowledgements

mDNS code is strongly based on @mrdunk [code written for esp8255 microcontroller](https://github.com/mrdunk/esp8266_mdns)

COAP code is strongly based on @hirotakaser [Simple CoAP library for Particle Core and Photon](https://github.com/hirotakaster/CoAP)

