using UnityEngine;
using System;
using System.Net;
using System.Text;
using System.Collections;
using System.Net.Sockets;
using System.Diagnostics;
using System.Runtime.InteropServices;

public enum PacketType : byte {
	Ack = 1,
	ServAck,
	SetSmooth,
	GetData,
	Data,

	EnableDebug,

	CantFindFace,
}

[StructLayout(LayoutKind.Sequential, Pack=1)]
public struct Packet {
	public PacketType type;
	public double data;
}

public class EyeTracker : MonoBehaviour {
	public bool isLooking;
	public double data;

	public int PORT = 1300;

	Socket client;
	IPEndPoint ep;

	Process proc;

	public bool hasAcked = false;
	public float ackTimeout = 0f;
	public bool requestSent = false;

	delegate void RequestDelegate(Packet data);

	void Start () {
		var IP = IPAddress.Parse("127.0.0.1");
		
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
			Request(PacketType.Ack, d => {
				if(d.type != PacketType.ServAck){
					print("Server didn't ack correctly");
				}else{
					hasAcked = true;
					Send(PacketType.EnableDebug);
				}
			});
		}

		if(hasAcked && !requestSent){
			requestSent = true;

			Request(PacketType.GetData, d => {
				if(d.type != PacketType.Data){
					print("Server didn't respond correctly");
				}else{
					data = d.data;
					if(data < 0f){
						data *= -data * 20f;
					}

					isLooking = data >= -0.2f;
				}

				requestSent = false;
			});
		}
	}

	void OnDestroy() {
		if(!proc.HasExited) proc.CloseMainWindow();
	}

	void Request(PacketType type, RequestDelegate rdg){
		Request(type, 0.0, rdg);
	}
	void Request(PacketType type, double data, RequestDelegate rdg){
		Send(type, data);

		int packetSize = Marshal.SizeOf(typeof(Packet));
		var buffer = new byte[packetSize];
		client.BeginReceive(buffer, 0, packetSize, 0, ar => {
			var read = client.EndReceive(ar);
			if(read != packetSize){
				print("Received malformed packet of size "+read.ToString());
				return;
			}

			rdg(GetPacket(buffer));
		}, null);
	}

	void Send(PacketType type, double data = 0.0){
		Packet tosend;
		tosend.type = type; tosend.data = data;
		client.SendTo(GetBytes(tosend), ep);
	}

	static byte[] GetBytes(Packet str) {
		int size = Marshal.SizeOf(str);
		byte[] arr = new byte[size];

		IntPtr ptr = Marshal.AllocHGlobal(size);
		Marshal.StructureToPtr(str, ptr, true);
		Marshal.Copy(ptr, arr, 0, size);
		Marshal.FreeHGlobal(ptr);
		return arr;
	}

	static Packet GetPacket(byte[] arr) {
		Packet str;

		int size = Marshal.SizeOf(typeof(Packet));
		IntPtr ptr = Marshal.AllocHGlobal(size);

		Marshal.Copy(arr, 0, ptr, size);

		str = (Packet)Marshal.PtrToStructure(ptr, typeof(Packet));
		Marshal.FreeHGlobal(ptr);

		return str;
	}
}
