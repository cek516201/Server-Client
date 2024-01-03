using System;
using CVSP;

namespace UnityClientTest
{
    class Program
    {
        static void Main(string[] args)
        {
            ConnectionManager manager = new ConnectionManager();
            string message;
            manager.ConnectionServer("127.0.0.1", 5004);

            if(manager.CONNECT)
            {
                Console.WriteLine("서버 연결 성공");
            }

            while(manager.CONNECT)
            {
                message = Console.ReadLine();

                if(message =="EXIT")
                {
                    manager.EndConnectionMessage();
                }
                else
                {
                    manager.SendWithPayLoad(SpecificationCVSP.CVSP_CHATTINGREQ, SpecificationCVSP.CVSP_SUCCESS, message);
                }
            }
        }
    }
}
