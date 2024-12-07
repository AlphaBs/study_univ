import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

public class TcpServer {
    public void start(String host, int port) throws Exception {
        // host:port 로 TCP 바인딩
        try (ServerSocket server = new ServerSocket(port, 0, InetAddress.getByName(host))) {
            // 연결 대기
            try (Socket client = server.accept()) {
                // 입력 스트림, 출력 스트림 만들기
                PrintWriter writer = new PrintWriter(client.getOutputStream());
                BufferedReader reader = new BufferedReader(new InputStreamReader(client.getInputStream()));

                // 클라이언트로부터 메세지 수신하고 출력
                String line = reader.readLine();
                System.out.println(line);

                // 클라이언트로 학번 전송
                writer.println("20211400");
                writer.flush();
            }
        }
    }
}