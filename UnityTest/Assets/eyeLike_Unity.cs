using UnityEngine;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;
using System.Text;

public class eyeLike_Unity : MonoBehaviour {
	public bool isLooking;

	public int PORT = 1300;
	IPAddress IP;

	Socket client;
	IPEndPoint ep;

	Process proc;

	bool requestSent = false;

	delegate void RequestDelegate(string data);

	void Start () {
		IP = IPAddress.Parse("127.0.0.1");
		
		client = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
		ep = new IPEndPoint(IP, PORT);


		proc = new Process();
		proc.StartInfo.FileName = "../build/build.exe";
		proc.StartInfo.Arguments = PORT.ToString();
		proc.StartInfo.RedirectStandardOutput = true;
		proc.StartInfo.UseShellExecute = false;
		proc.StartInfo.CreateNoWindow = true;
		proc.OutputDataReceived += new DataReceivedEventHandler((sender, evt) => {
			var data = evt.Data;
			print("App stdout: " + data);

			if(!requestSent){
				requestSent = true;
				print("Request");

				Request("Ack", d => {
					print(d);
				});
			}
		});
		proc.Start();
		proc.BeginOutputReadLine();
	}
	
	void Update () {
		//var receivedData = client.Receive(ref ep);
		//Debug.Log("Received Data: " + receivedData);
	}

	void OnDestroy() {
		if(!proc.HasExited) proc.CloseMainWindow();
	}

	void Request(string data, RequestDelegate rdg){
		client.SendTo(Encoding.UTF8.GetBytes(data), ep);

		var buffer = new byte[32];
		client.BeginReceive(buffer, 0, 32, 0, ar => {
			var read = client.EndReceive(ar);
			rdg(Encoding.UTF8.GetString(buffer, 0, read));
		}, null);
	}
}
