WormHole Streaming Library
==========================

Wormhole has been design to be a faster implementation of other streaming systems such as Storm or Flink.
It has its own stream-API wrote in c/c++ and Java.
Also we are working in a Storm-envelope that will allow any storm-application run freelly over wormhole without need of modify *any line* in our original application.

## How does it works?

WormHole Streaming Library is divided into 2 differnt parts: Einsteins and Worms.

#### Einstein
There is planned that there would be more than Einstein instence.
The master instance, ensures that the topology described is running at it should do. This means: Deploy new worms, check for worm over/underload, check if a worm has stopped, etc.
Also, the master Einstein instance "heart-beat" the other instances telling any modification in the deployed topology. So, if something goes wrong with the master Einstein, another instance can take the control of the topology.

To run a topology, we have to start Einstein in the following way:

```
einstein <Configuration_File> <Listen_IP>
```

A Topology is defined by a group of worm and their interconnections.
Each worm, is also defined by 2 different lines: a General configuration line and a Output Topology.
The first line (general conf.) is composed by the following item:
- `ID`                  [**Mandatory**] : The Worm ID. It is accepted to provide a range (like 1-5), so 5 worms(1,2,3,4,5) would be deployed **WITH THE SAME** configuration, but diferent ID.
- `Program_Name`        [**Mandatory**] : [The program name](#how-worms-are-deployed)
- `Deploy_IP`           [**Mandatory**] : The IP address where the worm would be deployed
- `Affinity_hex_mask`   [**Mandatory**] : The core-affinity of the worm. -1 means that there is no affinity
- `SSL`                 [**Optional**]  : If SSL is present in a Worm definition, that worm would encript thougt TLS1.2 the communications **with any other worm** (input and output). Also, SSL would check if is an authorized worm.

The second line has a tab. in the begining. Then a S-Expression defines how the routing would be. There are 4 possible operators:
- `DUP`    : This operator duplicates each message between each following item. If no operator is defined, DUP operator would be supposed. Example: `(DUP 1 2 3)`
- `RR`     : This operator simply uses roundrobin algorithm. Example: `(RR 1 2 3)`
- `CAT`    : This operator check the fild category, and send acordly to it. Example: `(CAT (1.(1 2)) (5.(6 7)))`
- `HASH`   : This operator check the field hash, and perform a modulus of the numbrer of outputs in order to define the output. Example: `(HASH 1 2 3)`
- `IGNORE` : This operator ignores/drops every message. Everything that comes begind this command (but in the same context) will be ignored. 


A configuration file looks like:
```
<ID> <Program_Name> <Deploy_IP> <Affinity_hex_mask (-1 means no affinity)> [SSL]
	(Cat (1.(DUP 1 2 3)) (2.(RR 1 2 (HASH 3 4))))
```

## Known Issues and problems
- A worm does not allowed loopback messages. It will be sometime fixed

###### How worms are deployed
Each program name would be tried to be find in the current directory adding the extension `.tgz`. This file should content all the necesary data to deploy a worm application, that must have :
- `lib/`    : All the lib/ folder of the compiled WormHole library for the correct architecture where the worm would be deployed.
- `myApp`   : The application itself, no matters its name.
- `run.sh`  : This script will be executed by Einstein to run the application. This script allows to execute an application indepently on wich language it is written (compiled, java, python, etc). Also, this script can have arguments, that can be passed to the finnal application.

#### Worm
Each worm is a program that sends/receives messages using the WormHole Streaming Library. The language program **does not matter**.

Each worm:
- Is identified by an `ID`.
- Is deployed in a `IP` thought ssh/scp connection.
- Can encrypt thought TLS1.2 any income and output message.

### Security and Certificates

To use SSL between two worms, there should be the SSL param in the configuration.

**NOT YET IMPLEMENTED** To use SSL connection between Einstein and any Worm, the parameter -SSL should be passed to the Einstein starter program.

In order to work with SSL properly, there must be in each .tgz a copy of: `cert/ca.pem` `cert/einstein.pem` `cert/worm.pem`

