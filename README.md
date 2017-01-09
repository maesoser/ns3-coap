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

Here is some example about the usage of this script
```
./run --routing=0 --verbose=1 --ping=0 --speed=40  --interval=15
```
