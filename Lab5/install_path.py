#!/usr/bin/python

import sys
import re # For regex

import ryu_ofctl as ryu
from ryu_ofctl import *

def main(macHostA, macHostB):
    print "Installing flows for %s <==> %s" % (macHostA, macHostB)

    ##### FEEL FREE TO MODIFY ANYTHING HERE #####
    try:
        pathA2B = dijkstras(macHostA, macHostB)
        print "ran dijkstra, path is:"
        print pathA2B
        installPathFlows(macHostA, macHostB, pathA2B)
        print "installed path flows"
    except:
        raise

	print "returning"
    return 0

# Installs end-to-end bi-directional flows in all switches
def installPathFlows(macHostA, macHostB, pathA2B):
    for link in pathA2B:
        flow1 = ryu.FlowEntry()
        flow2 = ryu.FlowEntry()

        action1= ryu.OutputAction(link['out_port'])
        action2= ryu.OutputAction(link['in_port'])

        flow1.in_port = link['in_port']
        flow2.in_port = link['out_port']

        flow1.addAction(action1)
        flow2.addAction(action2)

        ryu.insertFlow(str(link['dpid']), flow1)
        ryu.insertFlow(str(link['dpid']), flow2)

    print "added flows"
    return

# Returns List of neighbouring DPIDs
def findNeighbours(dpid):
	if dpid < 0:
		raise TypeError("DPID should be a positive integer value")

	else:
		neighbours = []
		links = ryu.listSwitchLinks(dpid)
		links = links['links']
		for link in links:
			#print link
			if link['endpoint1']['dpid'] != dpid:
				neighbours.append(link['endpoint1']['dpid'])
			else:
				neighbours.append(link['endpoint2']['dpid'])
		neighbours = list(set(neighbours))
		print "nearest neighbours of", dpid, "are:" ,neighbours
		return neighbours


# Optional helper function if you use suggested return format
def nodeDict(dpid, in_port, out_port):
    assert type(dpid) in (int, long)
    assert type(in_port) is int
    assert type(out_port) is int
    return {'dpid': dpid, 'in_port': in_port, 'out_port': out_port}

'''accepts as parameters: dpid_to, dpid of the next switches
neighbour_links, list of dictionaries of the links, one endpoint is the current
switch dpid and the other is the neighbour switch dpid

This function finds the port in the switch that corresponds with the
dpid desired'''

def find_port(dpid, neighbour_links):
    for connection in neighbour_links:
        if connection['endpoint1']['dpid'] is dpid:
            return connection['endpoint1']['port']

        elif connection['endpoint2']['dpid'] is dpid:
            return connection['endpoint2']['port']


def backtrace(parent,start,end,p_start,p_end):
    if(start == end):
        return [nodeDict(int(start),int(p_start),int(p_end))]

    path = []
    path.append(end) # add the end

    id = parent[end] # start with parent of the end if parent of the end is start, won't go through loop
    while(id != start):
        path.append(id)
        id = parent[id]

    path.append(id) # add the start to it
    path = path[::-1]# go from start -> end

    ret = [] # now we make the dictionaries of the ports u go into
    print "path is:",path
    port_in = p_start
    for i in range(0,len(path)):
        id = path[i]

        if id == end:#if it's the dest connect the ports in the switch
            ret.append(nodeDict(int(id), int(port_in), int(p_end)))
            return [ret]
        else:
            next_id = path[i+1]
            print "next id is:" ,next_id
            neighbours = ryu.listSwitchLinks(id)['links'] #get neighbours
            print "neighbours are:", neighbours
            port_out = find_port(next_id,neighbours)
            print "id is:",id,"port_in:", port_in, "port_out:", port_out
            ret.append(nodeDict(int(id),int(port_in), int(port_out)))

        # find the corresponding port # of current dpid for the next switch
        neighbour_neighbours = ryu.listSwitchLinks(next_id)['links']
        port_in = find_port(id,neighbours)


# Calculates least distance path between A and B
# Returns detailed path (switch ID, input port, output port)
#   - Suggested data format is a List of Dictionaries
#       e.g.    [   {'dpid': 3, 'in_port': 1, 'out_port': 3},
#                   {'dpid': 2, 'in_port': 1, 'out_port': 2},
#                   {'dpid': 4, 'in_port': 3, 'out_port': 1},
#               ]
# Raises exception if either ingress or egress ports for the MACs can't be found
def bfs(start,end, portStart, portEnd):
	print "start, end is" ,start,end
	parent = {}
	parent[start] = start # some stuff
	queue = []
	queue.append(start)

	while(queue):
		id = queue.pop(0)
		if id == end:
			print "found ending dip"
			return backtrace(parent,start,end, portStart, portEnd)

		neighbours = findNeighbours(id)
		for neighbour in neighbours:
			if neighbour not in queue:
				parent[neighbour] = id
				queue.append(neighbour)

def dijkstras(macHostA, macHostB):

    # Optional variables and data structures
    # INFINITY = float('inf')
    # distanceFromA = {} # Key = node, value = distance
    # leastDistNeighbour = {} # Key = node, value = neighbour node with least distance from A
    # pathAtoB = [] # Holds path information

    ##### YOUR CODE HERE ##### BFS
    packet = ryu.getMacIngressPort(macHostB)
    dpidEnd = packet['dpid']
    portEnd = packet['port']

    packet = ryu.getMacIngressPort(macHostA)
    dpidStart = packet['dpid']
    portStart = packet['port']
    print "doing BFS"
    pathAtoB = bfs(dpidStart, dpidEnd, portStart, portEnd)

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
