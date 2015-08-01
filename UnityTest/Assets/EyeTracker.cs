using UnityEngine;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;
using System.Text;

public class EyeTracker : MonoBehaviour {
	public bool isLooking;
	public double data;

	public int PORT = 1300;
	IPAddress IP;

	Socket client;
	IPEndPoint ep;

	Process proc;

	public bool hasAcked = false;
	public float ackTimeout = 0f;

	public bool requestSent = false;

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
		});
		proc.Start();
		proc.BeginOutputReadLine();

		ackTimeout = 1f;
	}
	
	void Update () {
		ackTimeout -= Time.deltaTime;
		if(!hasAcked && ackTimeout <= 0f){
			ackTimeout = 2f;
			Request("ack", d => {
				print(d);
				hasAcked = true;
			});
		}

		if(hasAcked && !requestSent){
			requestSent = true;

			Request("data", d => {
				data = System.Convert.ToDouble(d);
				if(data < 0f){
					data *= -data * 20f;
				}

				isLooking = data >= -0.2f;

				requestSent = false;
			});
		}
	}

	void OnDestroy() {
		if(!proc.HasExited) proc.CloseMainWindow();
	}

	void Request(string data, RequestDelegate rdg){
		client.SendTo(Encoding.UTF8.GetBytes(data), ep);

		var buffer = new byte[128];
		client.BeginReceive(buffer, 0, 128, 0, ar => {
			var read = client.EndReceive(ar);
			rdg(Encoding.UTF8.GetString(buffer, 0, read));
		}, null);
	}
}
