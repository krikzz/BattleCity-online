using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Net;
using System.Net.Sockets;

namespace netclient
{
    class Program
    {

        static SerialPort port;
        static TcpClient client;
        static NetworkStream ns;
        static byte system_type;
        static string servername;
        static int server_port;

        static void Main(string[] args)
        {
            server_port = 98;
            servername = "krikzz.org";

            try
            {

                if (args.Length == 2)
                {
                    servername = args[0];
                    server_port = Convert.ToInt32(args[1]);
                }
            }
            catch (Exception x)
            {
                Console.WriteLine("Arg ERROR: " + x.Message);
            }

            Console.WriteLine("server: " + servername + ":" + server_port);
            for (; ; )
            {

                Console.WriteLine("Waiting for MegaDrive connection!");
                for (; ; )
                {
                    try
                    {
                        //connectToServer();
                        connectToMegadrive();
                        Console.WriteLine("");
                        Console.WriteLine("Connected to MegaDrive " + port.PortName);
                        break;
                    }
                    catch (Exception)
                    {

                        Thread.Sleep(1000);
                    }
                }

                try
                {
                    slave();
                }
                catch (Exception x)
                {
                    try
                    {
                        if (client != null) client.Close();
                        client = null;
                        ns = null;
                    }
                    catch (Exception) { }
                    Console.WriteLine("ERROR: " + x.Message);
                }
            }
        }


        static void connectToMegadrive()
        {

            byte[] buff = new byte[1];
            buff[0] = 0xff;
            string[] port_list = SerialPort.GetPortNames();

            try
            {
                port.Close();
            }
            catch (Exception) { }

            for (int i = 0; i < port_list.Length; i++)
            {
                try
                {
                    port = new SerialPort(port_list[i]);
                    port.ReadTimeout = 200;
                    port.WriteTimeout = 200;
                    port.Open();
                    port.Write(buff, 0, 1);//terminate connection if exist
                    port.Write("+T");
                    if (port.ReadByte() != 'k') throw new Exception("Unexpected response");
                    port.ReadTimeout = -1;
                    port.WriteTimeout = 1000;
                    port.Write("+m");
                    return;
                }

                catch (Exception)
                {
                    try
                    {
                        port.Close();
                    }
                    catch (Exception) { }
                }
            }
            throw new Exception("Mega EverDrive is not detected");
        }



        static void slave()
        {
            byte cmd;
            for (; ; )
            {
                cmd = (byte)port.ReadByte();
                if (cmd != '+') continue;
                cmd = (byte)port.ReadByte();

                switch (cmd)
                {
                    case (byte)'0':
                        throw new Exception("Connection terminated by megadrive");
                    case (byte)'1':
                        system_type = (byte)port.ReadByte();
                        connectToServer();
                        break;
                    case (byte)'2':
                        startCommunication();
                        break;
                }

            }
        }

        static void startCommunication()
        {
            int len = 0;
            long time;
            bool ping_end = false;
            string cmd_stream = "";

            Console.WriteLine("Begin data streaming");
            while (ns.DataAvailable)
            {
                if (ns.ReadByte() == 0) break;
                len++;
            }
            Console.WriteLine("Ready! " + len);

            time = DateTime.Now.Ticks;
            for (; ; )
            {

                if (port.BytesToRead > 0)
                {
                    byte[] buff = new byte[port.BytesToRead];
                    len = port.Read(buff, 0, buff.Length);
                    if (len > 0)
                    {
                        ns.Write(buff, 0, len);
                        time = DateTime.Now.Ticks;
                    }
                    if (!ping_end)
                    {
                        for (int i = 0; i < len; i++) cmd_stream += (char)buff[i];
                        if (cmd_stream.Contains("+0")) throw new Exception("Connection terminated by megadrive");
                    }

                }

                if (ns.DataAvailable)
                {
                    byte[] buff = new byte[1];
                    len = ns.Read(buff, 0, buff.Length);
                    if (len > 0)
                    {
                        if (buff[0] != 'p') ping_end = true;
                        port.Write(buff, 0, len);
                        time = DateTime.Now.Ticks;
                    }

                }

                if (DateTime.Now.Ticks - time > 10000000 * 30)
                {
                    throw new Exception("Timeout");
                }

                Thread.Sleep(1);
            }
        }



        static void connectToServer()
        {

            int sport = server_port;
            try
            {
                if (system_type == 'p')
                {
                    Console.WriteLine("System type: PAL");
                    sport--;
                }
                else
                {
                    Console.WriteLine("System type: NTSC");
                }
                Console.Write("Connection to server: " + servername + ":" + sport + "...");
                client = new TcpClient(servername, sport);
                //client = new TcpClient("192.168.0.102", 98);
                ns = client.GetStream();

            }
            catch (Exception x)
            {
                client = null;
                ns = null;
                port.Write("e");
                Console.WriteLine("ERROR: " + x.Message);
            }

            port.Write("k");
            Console.WriteLine("OK");

        }
    }
}
