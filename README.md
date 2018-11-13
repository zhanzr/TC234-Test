TC234 Demo.


• Four General Purpose Service Requests (GPSR) per CPU that can be used as
Software Interrupts (not assigned to peripherals or external interrupts)
• Mechanism to signal General Purpose Service Requests (Software Interrupts)
simultaneously to multiple Service Providers (Service Request Broadcast Registers,
SRB)

The TC21x/TC22x/TC23x contains groups of General Purpose Service Request SRNs
per CPU that support software-initiated interrupts. One group per implemented TriCore
CPU, each group including four SRNs. These SRNs are not connected to internal or
external hardware trigger signals and can only be used as software interrupts / software
initiated service requests. These SRNs are called General Purpose Service Requests
Nodes (SRC_GPSRxy, x=group number, y=0-3).
Additionally, any otherwise unused SRN can be employed to generate software
interrupts.

