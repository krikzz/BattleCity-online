using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace netserver
{
    class Program
    {

        //static TcpListener server;
        static bool stop_sever;
        

        static void Main(string[] args)
        {
            
            int server_port = 98;
            try
            {
                if (args.Length > 0) server_port = Convert.ToInt32(args[0]);
            }
            catch (Exception) { }

            stop_sever = false;

            Thread ntsc = new Thread(serverLoop);
            ntsc.Start(server_port);

            Thread pal = new Thread(serverLoop);
            pal.Start(server_port-1);

            for (; ; )
            {
                if (Console.ReadLine().Contains("stop"))
                {
                    Console.WriteLine("Sherver shotdown");
                    stop_sever = true;
                    Thread.Sleep(100);
                    pal.Abort();
                    ntsc.Abort();
                    return;
                }
            }
        }

        static void serverLoop(Object arg)
        {
            int port = (int)arg;
            TcpListener server;

            Console.WriteLine("Blast processing server v 1.03");
            Console.WriteLine("Server port: "+port);
            try
            {
                server = new TcpListener(IPAddress.Parse("0.0.0.0"), port);
                server.Start();

                while (true)
                {
                    if (stop_sever) return;
                    try
                    {
                        waitPair(server, port);
                        Thread.Sleep(200);
                    }
                    catch (Exception x)
                    {
                        Console.WriteLine(x.Message);
                    }
                }

            }
            catch (Exception x)
            {
                Console.WriteLine("ERROR: " + x.Message);
            }
        }

        static void waitPair(TcpListener server, int port)
        {
            TcpClient[] clients = new TcpClient[2];
            bool discconect;
            NetworkStream ns;
            long time;
            string client_addr;

            while (!server.Pending())
            {
                if (stop_sever) return;
                Thread.Sleep(10);
            }



            clients[0] = server.AcceptTcpClient();
            client_addr = clients[0].Client.RemoteEndPoint.ToString();
            Console.WriteLine("Connection 1 from " + client_addr + ":sp " + port);
            ns = clients[0].GetStream();
            

            time = DateTime.Now.Ticks;
            while (!server.Pending())
            {
                if (stop_sever) return;
                discconect = false;
                if (DateTime.Now.Ticks - time < 10000000) continue;
                time = DateTime.Now.Ticks;
                try
                {
                    ns.WriteByte((byte)'p');
                }
                catch (Exception)
                {
                    discconect = true;
                }

                if (!clients[0].Connected || discconect)
                {
                    throw new Exception("Connection lost: " + client_addr + ":sp " + port);
                }
                Thread.Sleep(10);
            }

            clients[1] = server.AcceptTcpClient();
            Console.WriteLine("Connection 2 from " + clients[1].Client.RemoteEndPoint + ":sp " + port);
           

            makePair(clients);
            
        }

        static void makePair(TcpClient[] clients)
        {
            Thread t = new Thread(pair);
            t.Start(clients);
        }

        static void pair(Object arg)
        {
            TcpClient[] clients = (TcpClient[])arg;
            NetworkStream[] stream = new NetworkStream[2];

            long timeout;
            byte[] buff = new byte[1];
            int len;

            try
            {
                Console.WriteLine("Player 1 " + clients[0].Client.RemoteEndPoint);
                Console.WriteLine("Player 2 " + clients[1].Client.RemoteEndPoint);
                stream[0] = clients[0].GetStream();
                stream[1] = clients[1].GetStream();
                Thread.Sleep(200);
                stream[0].WriteByte(0);
                stream[1].WriteByte(0);

                stream[0].WriteByte((byte)'+');
                stream[0].WriteByte((byte)'j');
                stream[0].WriteByte(0);
                stream[1].WriteByte((byte)'+');
                stream[1].WriteByte((byte)'j');
                stream[1].WriteByte(1);

                timeout = DateTime.Now.Ticks;
                while (true)
                {

                    if (stop_sever)
                    {
                        throw new Exception("server shotdown");
                    }

                    if (stream[0].DataAvailable)
                    {
                        len = stream[0].Read(buff, 0, buff.Length);
                        if (len > 0) stream[1].Write(buff, 0, len);
                        timeout = DateTime.Now.Ticks;
                    }

                    if (stream[1].DataAvailable)
                    {
                        len = stream[1].Read(buff, 0, buff.Length);
                        if (len > 0) stream[0].Write(buff, 0, len);
                        timeout = DateTime.Now.Ticks;
                    }


                    if (DateTime.Now.Ticks - timeout > 10000000 * 60)
                    {
                        try
                        {
                            stream[0].Close();
                        }
                        catch (Exception) { }
                        try
                        {
                            stream[1].Close();
                        }
                        catch (Exception) { }

                        throw new Exception("timeout");

                    }

                    Thread.Sleep(1);
                }


            }
            catch (Exception x)
            {
                Console.WriteLine("pair connection terminated: " + x.Message);
                Thread.CurrentThread.Abort();
                return;
            }

        }

    }
}
