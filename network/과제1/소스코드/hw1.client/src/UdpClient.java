import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class UdpClient {
    public void start(String host, int port) throws Exception {
        try (DatagramSocket socket = new DatagramSocket()) {
            // host:port 로 메세지 패킷 만들고 전송
            InetAddress address = InetAddress.getByName(host);
            byte[] sendBuffer = "Hello, world!".getBytes(); // 버퍼 만들기
            DatagramPacket sendPacket = new DatagramPacket(sendBuffer, sendBuffer.length, address, port); // 패킷 만들기
            socket.send(sendPacket); // 전송

            // UDP 패킷 들어올때까지 대기
            byte[] receiveBuffer = new byte[128]; // 버퍼
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length); // 전송받을 패킷
            socket.receive(receivePacket); // 받기

            // 전송받은 패킷의 데이터 출력
            String receiveMessage = new String(receivePacket.getData(), 0, receivePacket.getLength()); // String 변환
            System.out.println(receiveMessage); // 출력
        }
    }
}
