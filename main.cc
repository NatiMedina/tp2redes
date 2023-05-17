/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//  n0   n1   n2 --------------- n3   n4   n5   
//  |    |    |   point-to-point  |    |    |  
//  ===========                  ============
//   LAN 10.1.1.0                 LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Dumbbell_Topology");

int main(int argc, char *argv[])
{

    uint32_t n1 = 3;
    uint32_t n2 = 3;

    //CommandLine cmd(__FILE__);
    /*cmd.AddValue("n1", "Number of LAN 1 nodes", n1);
    cmd.AddValue("n2", "Number of LAN 2 nodes", n2);

    cmd.Parse(argc, argv);*/

    // LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer lan1Nodes;
    NodeContainer lan2Nodes;
    NodeContainer routerNodes;

    lan1Nodes.Create(n1);
    lan2Nodes.Create(n2);
    routerNodes.Create(2);

    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    lan1Nodes.Add(routerNodes.get(0));
    // Actually attaching CsmaNetDevice to all LAN 1 nodes.
    NetDeviceContainer lan1Devices;
    lan1Devices = csma1.Install(lan1Nodes);

    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    lan2Nodes.Add(routerNodes.get(1));
    // Actually attaching CsmaNetDevice to all LAN 2 nodes.
    NetDeviceContainer lan2Devices;
    lan2Devices = csma2.Install(lan2Nodes);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer routerDevices;
    routerDevices = pointToPoint.Install(routerNodes);

    InternetStackHelper stack;
    stack.Install(lan1Nodes);
    stack.Install(lan2Nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer lan1Interfaces;
    lan1Interfaces = address.Assign(lan1Devices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer lan2Interfaces;
    lan2Interfaces = address.Assign(lan2Devices);

    address.SetBase("10.1.100.0", "255.255.255.0");
    Ipv4InterfaceContainer routerInterfaces;
    routerInterfaces = address.Assign(routerDevices);

    UdpEchoServerHelper echoServer(9);

    // Only install in the middle device
    ApplicationContainer serverApps = echoServer.Install(lan2Nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Let's create UdpEchoClients in LAN1 nodes.
    UdpEchoClientHelper echoClient(lan2interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(200)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(lan1Nodes.get(1));
    clientApps.Start(Seconds(1));
    clientApps.Stop(Seconds(10));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    csma1.EnablePcap("lan1", lan1Devices);
    csma2.EnablePcap("lan2", lan2Devices);
    pointToPoint.EnablePcapAll("routers");
    pointToPoint.EnableAscii("ascii-p2p", routerNodes);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
