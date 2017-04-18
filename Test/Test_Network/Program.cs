using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Text;

namespace Test_Network
{
    static class Program
    {
        static void Main(string[] Args)
        {
            TcpClient client = new TcpClient();
            client.Connect("127.0.0.1", 5005);

            byte[] buffer = new byte[500];
            int size = client.GetStream().Read(buffer, 0, 500);
            Console.WriteLine(Encoding.UTF8.GetString(buffer, 0, size));
            Console.ReadKey();
        }
    }
}
