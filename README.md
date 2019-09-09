# Chat-in-LAN
hw for UNIX programming

## Intro
- implement a simple chat program in LAN using Linux packet(7) socket. Instead of sending IP packets, broadcasting chat messages using layer 2 broadcast packets.

## Requirements
- Enumerate all Ethernet compatible network interfaces.
- Ask the user to provide his/her username.
- Repeatedly ask the user to enter his/her message. The message is then broadcasted to all enumerated Ethernet-compatible network interfaces.
- At the same time, your program should receive messages broadcasted by other host in connected LANs.
