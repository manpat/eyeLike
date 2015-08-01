using UnityEngine;
using System.Collections;
using System.Net;
using System.Net.Sockets;

public class NetTest : MonoBehaviour {
	void Start () {
		try{
			var client = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
			var endpoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 1337);

			// var data = "Blah";
			// var data = "GetTheThing";
			// client.SendTo(StoB(data), endpoint);
			client.SendTo(new byte[4]{1, 2, 3, 4}, endpoint);
		
			var buf = new byte[1024];
			client.Receive(buf);
			print("Data received " + BtoS(buf));
			
		}catch(System.Exception e){
			print(e.ToString());
		}
	}
	
	void Update () {
	
	}


	byte[] StoB(string s){
		byte[] buf = new byte[s.Length*sizeof(char)];
		System.Buffer.BlockCopy(s.ToCharArray(), 0, buf, 0, buf.Length);
		return buf;
	}
	string BtoS(byte[] bytes){
		char[] chars = new char[bytes.Length / sizeof(char)];
		System.Buffer.BlockCopy(bytes, 0, chars, 0, bytes.Length);
		return new string(chars);
	}
}
