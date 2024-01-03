using System;
using System.IO;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Runtime.Serialization.Formatters.Binary;
using System.Collections;
using System.Data;
using System.Text;
using System.Runtime.Serialization;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;

namespace CVSP
{
    // 서버와 클라이언트 간의 연결을 담당해주는 클래스
    public class ConnectionManager
    {
        private Socket mSocket;
        private byte[] mReadBuffer = new Byte[SpecificationCVSP.CVSPP_BUFFERSIZE];
        private BinaryFormatter mBinaryFormat;
        private bool mNewUser;
        private ArrayList mTransBuffer;

        public bool CONNECT {  get; private set; }

        // 보낼 패킷 구조체를 바이트로 변환
        public unsafe object ByteToStructure(Byte[] data, Type type)
        {
            // 배열의 크기 만큼 비관리 메모리 영역에 메모리를 할당
            IntPtr Buffer = Marshal.AllocHGlobal(data.Length);

            // 배열에 저장된 데이터를 위해서 할당한 메모리 영역에 복사
            Marshal.Copy(data, 0, Buffer, data.Length);

            // 복사된 데이터를 구조체 객체로 변환
            object Obj = Marshal.PtrToStructure(Buffer, type);

            // 비관리 메모리 영역에 할당했던 메모리를 해제
            Marshal.FreeHGlobal(Buffer);

            // 구조체와 원래의 데이터의 크기 비교
            if (Marshal.SizeOf(Obj) != data.Length)
            {
                return null;
            }

            return Obj;
        }

        // 바이트에서 패킷 구조체로 변환
        public unsafe Byte[] StructureToByte(object obj)
        {
            // 구조체에 할당된 메모리의 크기 저장
            int Datasize = Marshal.SizeOf(obj);

            // 비관리 메모리 영역에 구조체 크기만큼의 메모리 할당
            IntPtr Buffer = Marshal.AllocHGlobal(Datasize);

            // 할당된 구조체 객체의 주소 저장
            Marshal.StructureToPtr(obj, Buffer, false);

            // 구조체가 복사될 배열
            Byte[] Data = new byte[Datasize];

            // 구조체 객체를 배열에 복사
            Marshal.Copy(Buffer, Data, 0, Datasize);

            // 비관리 메모리 영역에 할당한 메모리 해제
            Marshal.FreeHGlobal(Buffer);

            return Data;
        }

        public ConnectionManager()
        {
            // 소켓 초기화를 여기서 처리
            this.mSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.IP);
            this.mNewUser = false;
            mTransBuffer = new ArrayList();
        }

        public bool ConnectionServer(string ipaddress, int port)
        {
            if(this.mSocket.Connected)
            {
                // 만약 연결이 성립되어서 정보교환 중 다시 이 메서드가 호출될 경우 여기서 무시
                return true;
            }

            try
            {
                // ClientSocket을 생성
                IPAddress myIP = IPAddress.Parse(ipaddress);
                this.mSocket.Connect(new IPEndPoint(myIP, port));

                Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
            }
            catch (Exception e)
            {
                string message = "서버와 연결에 실패 했습니다" + e.Message;
                Console.WriteLine(message);

                return false;
            }

            if(this.mSocket.Connected)
            {
                Thread echo_thread = new Thread(ReceiveThread);
                echo_thread.Start();
                CONNECT = true;

                return true;
            }
            else
            {
                return false;
            }
        }

        public bool ConnectionServer(string ip)
        {
            return ConnectionServer(ip, SpecificationCVSP.ServerPort);
        }

        public void Stop()
        {
            lock(this.mSocket)
            {
                this.mSocket.Close();
            }

            CONNECT = false;
        }

        public void EndConnectionMessage()
        {
            Send(SpecificationCVSP.CVSP_LEAVEREQ, SpecificationCVSP.CVSP_SUCCESS);
            Stop();
        }

        public int Send(byte cmd, byte option)
        {
            CVSP header = new CVSP();
            header.cmd = cmd;
            header.option = option;

            header.packetLength = (short)(4);

            byte[] buffer = new byte[header.packetLength];
            StructureToByte(header).CopyTo(buffer, 0);
            //this.mSocket.Send(buffer, 0, buffer.Length, SocketFlags.None);

            int result = mSocket.Send(buffer, 0, buffer.Length, SocketFlags.None);

            return result;
        }

        public void SendWithPayLoad(byte cmd, byte option, object data)
        {
            CVSP header = new CVSP();
            header.cmd = cmd;
            header.option = option;

            byte[] payload = StructureToByte(data);

            header.packetLength = (short)(4 + payload.Length);

            byte[] buffer = new Byte[header.packetLength];
            StructureToByte(header).CopyTo(buffer, 0);
            payload.CopyTo(buffer, 4);

            this.mSocket.Send(buffer, 0, buffer.Length, SocketFlags.None);
        }

        public void SendWithPayLoad(byte cmd, byte option, byte[] data)
        {
            CVSP header = new CVSP();
            header.cmd = cmd;
            header.option = option;

            //byte[] payload = StructureToByte(data);

            header.packetLength = (short)(4 + data.Length);

            byte[] buffer = new Byte[header.packetLength];
            StructureToByte(header).CopyTo(buffer, 0);
            data.CopyTo(buffer, 4);

            this.mSocket.Send(buffer, 0, buffer.Length, SocketFlags.None);
        }

        public int SendWithPayLoad(byte cmd, byte option, string message)
        {
            CVSP header = new CVSP();
            header.cmd = cmd;
            header.option = option;

            // EUCKR 인코딩 번호
            int euckrCodepage = 51949;

            // 인코딩을 편리하게 해주기 위해서 인코딩 클래스 변수를 만듭니다.
            System.Text.Encoding euckr = System.Text.Encoding.GetEncoding(euckrCodepage);

            byte[] payload = Encoding.GetEncoding("euc-kr").GetBytes(message);

            header.packetLength = (short)(4 + payload.Length);

            byte[] buffer = new byte[header.packetLength];
            StructureToByte(header).CopyTo(buffer, 0);
            payload.CopyTo(buffer, 4);

            int result = mSocket.Send(buffer, 0, buffer.Length, SocketFlags.None);

            return result;
        }

        public void ReceiveThread()
        {
            int nReadBytes = 0;
            byte[] headerBuffer = new byte[4];
            byte[] payload;
            CVSP header = new CVSP();

            try
            {
                while(CONNECT)
                {
                    nReadBytes = mSocket.Receive(headerBuffer, 4, SocketFlags.Peek);

                    if (nReadBytes < 0 || nReadBytes != 4)
                    {
                        continue;
                    }

                    header = (CVSP)ByteToStructure(headerBuffer, header.GetType());
                    nReadBytes = mSocket.Receive(mReadBuffer, header.packetLength, SocketFlags.None);
                    payload = new byte[header.packetLength - headerBuffer.Length];
                    Buffer.BlockCopy(mReadBuffer, headerBuffer.Length, payload, 0, header.packetLength - headerBuffer.Length);

                    if (header.cmd == SpecificationCVSP.CVSP_CHATTINGRES)
                    {
                        Console.WriteLine("서버로부터 받은 메시지..");
                        int euckrCodepage = 51949;
                        System.Text.Encoding euckr = System.Text.Encoding.GetEncoding(euckrCodepage);
                        string message = Encoding.GetEncoding("euc-kr").GetString(payload);
                        Console.WriteLine("서버로부터 받은 메시지 : " + message);
                    }
                }
            }
            catch (Exception e)
            {
                string temp = "에러가 발생해서 서버와의 연결을 닫습니다." + e.Message;
                Console.WriteLine(temp);
                Stop();
                CONNECT = false;
                return;
            }
        }
    }
}
