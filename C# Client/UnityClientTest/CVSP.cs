namespace CVSP
{
    using System;
    using System.Collections;
    using System.Data;
    using System.Runtime.Serialization.Formatters.Binary;
    using System.IO;
    using System.Runtime.InteropServices;

    // 이 구조체는 CVSP(Collaborative Virtual Service Protocol)을 객체 직렬화를 사용하여 전송할 수 있도록 해주는 구조체이다
    [System.Serializable]
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct CVSP
    {
        public byte cmd;
        public byte option;
        public short packetLength;
    }

    // CVSP 프로토콜의 커맨드, 옵션 등 내부 플래그 등을 정의해주는 클래스
    public sealed class SpecificationCVSP
    {
        // 프로토콜의 버전
        public static byte CVSP_VER = (byte)0x01;

        // 프로토콜의 커맨드(Cmd)
        public static byte CVSP_JOINREQ = (byte)0x01;
        public static byte CVSP_JOINRES = (byte)0x02;
        public static byte CVSP_CHATTINGREQ = (byte)0x03;
        public static byte CVSP_CHATTINGRES = (byte)0x04;
        public static byte CVSP_OPERATIONREQ = (byte)0x05;
        public static byte CVSP_MONITORINGMSG = (byte)0x06;
        public static byte CVSP_LEAVEREQ = (byte)0x07;

        public static byte CVSP_SUCCESS = (byte)0x01;
        public static byte CVSP_FAIL = (byte)0x02;

        public static int CVSP_SIZE = 4;
        public static int CVSPP_BUFFERSIZE = 4096;

        public static int ServerPort = 5004;
    }
}