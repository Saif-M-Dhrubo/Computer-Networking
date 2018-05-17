#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <stdio.h>
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (5);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer device1 = pointToPoint.Install (nodes.Get(0), nodes.Get(4));
  NetDeviceContainer device2 = pointToPoint.Install (nodes.Get(1), nodes.Get(4));
  NetDeviceContainer device3 = pointToPoint.Install (nodes.Get(2), nodes.Get(4));
  NetDeviceContainer device4 = pointToPoint.Install (nodes.Get(3), nodes.Get(4));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4AddressHelper address2;
  address2.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4AddressHelper address3;
  address3.SetBase ("10.1.4.0", "255.255.255.0");


  Ipv4InterfaceContainer interfaces1 = address.Assign (device1);
  Ipv4InterfaceContainer interfaces2 = address1.Assign (device2);
  Ipv4InterfaceContainer interfaces3 = address2.Assign (device3);
  Ipv4InterfaceContainer interfaces4 = address3.Assign (device4);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (4));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient0 (interfaces1.GetAddress (1), 9);
  echoClient0.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient0.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient0.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps0 = echoClient0.Install (nodes.Get (0));
  clientApps0.Start (Seconds (2.0));
  clientApps0.Stop (Seconds (10.0));

  printf("Client 0 created\n");

  UdpEchoClientHelper echoClient1 (interfaces2.GetAddress (1), 9);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (1));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (10.0));

  printf("Client 1 created\n");

  UdpEchoClientHelper echoClient2 (interfaces3.GetAddress (1), 9);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps2 = echoClient2.Install (nodes.Get (2));
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));

  printf("Client 2 created\n");

  UdpEchoClientHelper echoClient3 (interfaces4.GetAddress (1), 9);
  echoClient3.SetAttribute ("MaxPackets", UintegerValue (3));
  echoClient3.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient3.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps3 = echoClient3.Install (nodes.Get (3));
  clientApps3.Start (Seconds (2.0));
  clientApps3.Stop (Seconds (10.0));

  printf("Client 3 created\n");

  AnimationInterface anim("ns3_01.xml");
  anim.SetConstantPosition(nodes.Get(4),30,30);
  anim.SetConstantPosition(nodes.Get(0),10,10);
  anim.SetConstantPosition(nodes.Get(1),50,50);
  anim.SetConstantPosition(nodes.Get(2),50,10);
  anim.SetConstantPosition(nodes.Get(3),10,50);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
