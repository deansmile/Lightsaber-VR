// ------------------------------------------------------------------------------
/*
   -----------------------
    UDP-Receive (send to)
    -----------------------
    // [url]http://msdn.microsoft.com/de-de/library/bb979228.aspx#ID0E3BAC[/url]
   
   
    // > receive
    // 127.0.0.1 : 8051
   
    // send
    // nc -u 127.0.0.1 8051
 
*/
using UnityEngine;
using System.Collections;

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class UDPReceiveSaber : MonoBehaviour {

	// receiving Thread
	Thread receiveThread;

	// udpclient object
	UdpClient client;

	// public
	// public string IP = "127.0.0.1"; default local
	public int port; // define > init
	public float smooth = 1F;
	public float tiltAngleX = 0.0F;
	public float tiltAngleY = 0.0F;
	public float tiltAngleZ = 0.0F;

	public float accelX = 0.0F;
	public float accelY = 0.0F;
	public float accelZ = 0.0F;

	public float velocityX = 0;
	// infos
	public string lastReceivedUDPPacket="";
	public string allReceivedUDPPackets=""; // clean up this from time to time!

	string[] rot;

	public GameObject lightSaber; 
	ParticleRenderer particleRenderer;

	public GameObject sword;

	// start from shell
	private static void Main()
	{

		UDPReceiveSaber receiveObj=new UDPReceiveSaber();
		receiveObj.init();

		string text="";
		do
		{
			text = Console.ReadLine();
		}
		while(!text.Equals("exit"));
	}
	// start from unity3d
	public void Start()
	{
		init();

		sword = GameObject.FindGameObjectWithTag ("Sword");
		lightSaber = GameObject.FindGameObjectWithTag ("Light");
		particleRenderer = lightSaber.GetComponentInChildren<ParticleRenderer> ();

	}


	// OnGUI
	void OnGUI()
	{
		Rect rectObj=new Rect(40,10,200,400);
		GUIStyle style = new GUIStyle();
		style.alignment = TextAnchor.UpperLeft;
		GUI.Box(rectObj,"# UDPReceive\n127.0.0.1 "+port+" #\n"
			+ "shell> nc -u 127.0.0.1 : "+port+" \n"
			+ "\nLast Packet: \n"+ lastReceivedUDPPacket
			+ "\n\nAll Messages: \n"+allReceivedUDPPackets
			,style);
	}

	// init
	private void init()
	{
		// Endpunkt definieren, von dem die Nachrichten gesendet werden.
		print("UDPSend.init()");

		// define port
		port = 33333;

		// status
		print("Sending to 127.0.0.1 : "+port);
		print("Test-Sending to this Port: nc -u 127.0.0.1  "+port+"");

		receiveThread = new Thread(
			new ThreadStart(ReceiveData));
		receiveThread.IsBackground = true;
		receiveThread.Start();

	}

	void Update () {

		Quaternion target = Quaternion.Euler(- tiltAngleY, 0 ,-tiltAngleX);
		sword.transform.localRotation = Quaternion.Slerp(sword.transform.localRotation, target,(float)(Time.deltaTime * 0.1));

	}
	// receive thread
	private  void ReceiveData()
	{

		client = new UdpClient(port);
		while (true)
		{

			try
			{
				// Bytes empfangen.
				IPEndPoint anyIP = new IPEndPoint(IPAddress.Any, 0);
				byte[] data = client.Receive(ref anyIP);

				// Bytes mit der UTF8-Kodierung in das Textformat kodieren.
				string text = Encoding.UTF8.GetString(data);

				// Den abgerufenen Text anzeigen.
				print(">> " + text);


				// latest UDPpacket
				lastReceivedUDPPacket=text;
				rot = lastReceivedUDPPacket.Split(',');

				tiltAngleX = Convert.ToSingle (rot [0]); //roll
				tiltAngleY = Convert.ToSingle (rot [1]); //pitch
				tiltAngleZ = Convert.ToSingle (rot [2]);
				accelX = Convert.ToSingle (rot [4]) ; //roll
				accelY = Convert.ToSingle (rot [5]) ; //pitch
				accelZ = Convert.ToSingle (rot [6]) ;

			}
			catch (Exception err)
			{
				print(err.ToString());
			}
		}
	}

	// getLatestUDPPacket
	// cleans up the rest
	public string getLatestUDPPacket()
	{
		allReceivedUDPPackets="";
		return lastReceivedUDPPacket;
	}
}
