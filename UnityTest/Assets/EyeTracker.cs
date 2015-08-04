using UnityEngine;
using System;
using System.Net;
using System.Text;
using System.Collections;
using System.Net.Sockets;
using System.Diagnostics;
using System.Runtime.InteropServices;

enum PacketType : byte {
	Ack = 1,
	ServAck,
	SetSmooth,
	GetData,
	Data,

	EnableDebug,

	CantFindFace,
}

[StructLayout(LayoutKind.Sequential, Pack=1)]
struct Packet {
	public PacketType type;
	public float data;
}

public class EyeTracker : MonoBehaviour {
	// A rough estimation of whether or not the user is NOT looking down
	public bool isLooking = false;

	// The latest data received from the server (after smoothing)
	public float rawData;

	// The adjusted version of the latest data. 
	//	This is what should be used for calculations
	public float adjustedData;

	// Whether or not the server can see a face
	public bool serverCanFindFace = false;

	// The threshold, below which the user is assumed to be looking down
	public float lookingThreshold = -0.2f;

	// The factor by which adjustedData is adjusted when rawData is < 0
	//	The reason for this is that the server is less sensitive to looking
	//	down than looking up
	public float lookingDownFactor = 20f;

	// On startup, this sets the server-side smoothing ratio
	//	A value of 0f would result in no smoothing.
	//	Setting after startup does nothing, for that use SetSmoothing()
	[SerializeField] float serverSmoothing = 5f;

	// On startup, determines whether or not the server
	//	should open debug windows. Good for getting lighting conditions
	//	right. After startup, this can be enabled with EnableDebugWindows()
	[SerializeField] bool showDebugWindows = false;

	// Specifies the path to the server executable, from project root
	//	(the folder where the Assets/ and ProjectSettings/ folders are)
	[SerializeField] string pathToServer = "../build/build.exe";

	// The port to start the server on. Should be changed if something else
	//	is already running on a port.
	//	Note: Must be above 1024 and less than 65535
	[SerializeField] ushort port = 13370;

	// This is set once the server has started and has begun acknowledging
	//	requests for data
	[SerializeField] bool hasAcked = false;
	
	float ackTimeout = 0f;
	bool requestSent = false;

	Socket client;
	IPEndPoint ep;

	Process proc;

	delegate void RequestDelegate(Packet data);

	public void SetSmoothing(float smooth = 5f){
		serverSmoothing = smooth;
		Request(PacketType.SetSmooth, smooth, r => {
			if(r.type == PacketType.SetSmooth){
				print("Server smoothing set to " + r.data.ToString());
			}
		});
	}

	public void EnableDebugWindows(){
		Send(PacketType.EnableDebug);
	}

	void Start () {
		var IP = IPAddress.Parse("127.0.0.1");
		ep = new IPEndPoint(IP, port);
		
		client = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

		proc = new Process();
		proc.StartInfo.FileName = pathToServer;
		proc.StartInfo.Arguments = port.ToString();
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
					if(showDebugWindows) EnableDebugWindows();
					SetSmoothing(serverSmoothing);
				}
			});
		}

		if(hasAcked && !requestSent){
			requestSent = true;

			Request(PacketType.GetData, d => {
				if(d.type == PacketType.CantFindFace){
					serverCanFindFace = false;

				}else if(d.type != PacketType.Data){
					print("Server didn't respond correctly");

				}else{
					serverCanFindFace = true;

					rawData = d.data;
					adjustedData = rawData;
					if(adjustedData < 0f){
						adjustedData *= -adjustedData * lookingDownFactor;
					}

					isLooking = adjustedData >= lookingThreshold;
				}

				requestSent = false;
			});
		}
	}

	void OnDestroy() {
		if(!proc.HasExited) proc.CloseMainWindow();
	}

	void Request(PacketType type, RequestDelegate rdg){
		Request(type, 0f, rdg);
	}
	void Request(PacketType type, float data, RequestDelegate rdg){
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

	void Send(PacketType type, float data = 0f){
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
