import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class UdpServer {
    public void start(String host, int port) throws Exception {
        // port 로 들어오는 UDP 패킷 받기
        try (DatagramSocket socket = new DatagramSocket(port)) {
            // UDP 패킷 들어올때까지 대기
            byte[] receiveBuffer = new byte[128]; // 버퍼
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length); // 전송받을 패킷
            socket.receive(receivePacket); // 받기

            // 전송받은 패킷의 데이터 출력
            String receiveMessage = new String(receivePacket.getData(), 0, receivePacket.getLength()); // String 으로 변환
            System.out.println(receiveMessage); // 출력

            // 패킷 보내온 곳으로 다시 학번 데이터 보내기
            byte[] sendBuffer = "20211400".getBytes(); // 버퍼 만들기
            DatagramPacket sendPacket = new DatagramPacket(sendBuffer, sendBuffer.length, receivePacket.getAddress(), receivePacket.getPort()); // 패킷 만들기
            socket.send(sendPacket); // 전송
        }
    }
}
