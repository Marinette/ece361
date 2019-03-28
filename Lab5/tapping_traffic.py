# yo yo this is marinette yo
import ryu_ofctl
flow = ryu_ofctl.FlowEntry()
act1 = ryu_ofctl.OutputAction(3)
act2 = ryu_ofctl.OutputAction(1)

flow.in_port = 2
flow.addAction(act1)
flow.addAction(act2)

dpid = '1'  # '0x1' or '0000000000000001' works as well
ryu_ofctl.insertFlow(dpid, flow)
printf('Success!')
