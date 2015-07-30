using UnityEngine;
using System.Collections;
using System.Net;
using System.Net.Sockets;

public class eyeLike_Unity : MonoBehaviour 
{
    public bool isLooking;

    public int SOCKET = 1300;
    IPAddress IP;

    UdpClient client;
    IPEndPoint ep;

	void Start () 
    {
        IP = IPAddress.Parse(Network.player.ipAddress);
        Debug.Log(IP);
        
        client = new UdpClient();
        ep = new IPEndPoint(IP, SOCKET);
        client.Connect(ep);

        string args = IP + " " + SOCKET;
        System.Diagnostics.Process proc = new System.Diagnostics.Process();
        proc.StartInfo.Arguments = args;
        proc.StartInfo.FileName = "eyeLike.exe";
        proc.Start();

        //System.Diagnostics.Process.Start("eyeLike.exe");
	}
	
	void Update () 
    {
        //var receivedData = client.Receive(ref ep);
        //Debug.Log("Received Data: " + receivedData);
	}
}
