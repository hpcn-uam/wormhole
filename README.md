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
Also, the master Einstein instance "heart-beat" the other instances telling any modification in the deployed topology. So, if something goes wrong with the master Einstein, other instance can take the control of the topology.

To run a topology, we have to start Einstein in the following way:

```
Einstein <Configuration_File> <Listen_IP>
```

The Topology is defined have the following fields
- `IP`                  [**Mandatory**] : The IP where the worm would be deployed.
- `Program_Name`        [**Mandatory**] : [The program name](#how-worms-are-deployed)
- `Deploy_IP`           [**Mandatory**] : The IP address where the worm would be copied and started
- `Affinity_hex_mask`   [**Mandatory**] : The core-affinity of the worm. -1 means that there is no affinity
- `SSL`                 [**Optional**]  : If SSL is present in a Worm definition, that worm would encript thougt TLS1.2 the communications **with any other worm** (input and output). Also, SSL would check if is an authorized worm.

A configuration file looks like:
```
<ID> <Program_Name> <Deploy_IP> <Affinity_hex_mask (-1 means no affinity)> [SSL]
```

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

