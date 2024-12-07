public class Main {
    public static void main(String[] args) throws Exception {
        // 서버의 주소와 포트
        String host = "localhost";
        int port = 33775;

        if (true) {
            TcpServer server = new TcpServer();
            server.start(host, port);
        }
        else {
            UdpServer server = new UdpServer();
            server.start(host, port);
        }
    }
}