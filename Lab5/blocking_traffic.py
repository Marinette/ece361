#marinette - march 28, 2019

import ryu_ofctl
dpid = '1'  # '0x1' or '0000000000000001' works as well

#Deleting flows
ryu_ofctl.deleteAllFlows(dpid)
print("Deleted all flows.")
flow = ryu_ofctl.FlowEntry()
flow2 = ryu_ofctl.FlowEntry()
flow3 = ryu_ofctl.FlowEntry()
flow4 = ryu_ofctl.FlowEntry()
act = ryu_ofctl.OutputAction(2)

flow.in_port = 1
flow.dl_dst = "00:00:00:00:00:03"

flow2.in_port = 1
flow2.dl_dst = "00:00:00:00:00:02"
flow2.addAction(act)

flow3.in_port = 3
flow3.dl_dst = "00:00:00:00:00:01"

flow4.in_port = 3
flow4.dl_dst = "00:00:00:00:00:02"
flow4.addAction(act)

ryu_ofctl.insertFlow(dpid, flow)
ryu_ofctl.insertFlow(dpid, flow2)
ryu_ofctl.insertFlow(dpid, flow3)
ryu_ofctl.insertFlow(dpid, flow4)

print("blocking_traffic success")
