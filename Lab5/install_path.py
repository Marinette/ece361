#!/usr/bin/python

import sys
import re # For regex

import ryu_ofctl
from ryu_ofctl import *

def main(macHostA, macHostB):
    print "Installing flows for %s <==> %s" % (macHostA, macHostB)

    ##### FEEL FREE TO MODIFY ANYTHING HERE #####
    try:
        pathA2B = dijkstras(macHostA, macHostB)
        installPathFlows(macHostA, macHostB, pathA2B)
    except:
        raise


    return 0

# Installs end-to-end bi-directional flows in all switches
def installPathFlows(macHostA, macHostB, pathA2B):
    ##### YOUR CODE HERE #####

    return

# Returns List of neighbouring DPIDs
def findNeighbours(dpid):
    if type(dpid) not in (int, long) or dpid < 0:
        raise TypeError("DPID should be a positive integer value")

    neighbours = []

    ##### YOUR CODE HERE #####

    return neighbours

# Calculates least distance path between A and B
# Returns detailed path (switch ID, input port, output port)
#   - Suggested data format is a List of Dictionaries
#       e.g.    [   {'dpid': 3, 'in_port': 1, 'out_port': 3},
#                   {'dpid': 2, 'in_port': 1, 'out_port': 2},
#                   {'dpid': 4, 'in_port': 3, 'out_port': 1},
#               ]
# Raises exception if either ingress or egress ports for the MACs can't be found
def dijkstras(macHostA, macHostB):

    # Optional helper function if you use suggested return format
    def nodeDict(dpid, in_port, out_port):
        assert type(dpid) in (int, long)
        assert type(in_port) is int
        assert type(out_port) is int
        return {'dpid': dpid, 'in_port': in_port, 'out_port': out_port}

    # Optional variables and data structures
    INFINITY = float('inf')
    distanceFromA = {} # Key = node, value = distance
    leastDistNeighbour = {} # Key = node, value = neighbour node with least distance from A
    pathAtoB = [] # Holds path information

    ##### YOUR CODE HERE #####

    # Some debugging output
    #print "leastDistNeighbour = %s" % leastDistNeighbour
    #print "distanceFromA = %s" % distanceFromA
    #print "pathAtoB = %s" % pathAtoB

    return pathAtoB



# Validates the MAC address format and returns a lowercase version of it
def validateMAC(mac):
    invalidMAC = re.findall('[^0-9a-f:]', mac.lower()) or len(mac) != 17
    if invalidMAC:
        raise ValueError("MAC address %s has an invalid format" % mac)

    return mac.lower()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "This script installs bi-directional flows between two hosts"
        print "Expected usage: install_path.py <hostA's MAC> <hostB's MAC>"
    else:
        macHostA = validateMAC(sys.argv[1])
        macHostB = validateMAC(sys.argv[2])

        sys.exit( main(macHostA, macHostB) )
