public class Main {
    public static void main(String[] args) throws Exception {
        // 접속할 서버의 주소와 포트
        String host = "localhost";
        int port = 33775;

        // TCP / UDP 선택
        if (true) {
            TcpClient tcpClient = new TcpClient();
            tcpClient.start(host, port);
        }
        else {
            UdpClient udpClient = new UdpClient();
            udpClient.start(host, port);
        }
    }
}