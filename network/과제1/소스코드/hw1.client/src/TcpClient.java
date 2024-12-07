import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class TcpClient {
    public void start(String host, int port) throws Exception {
        // host:port 로 TCP 연결
        try (Socket socket = new Socket(host, port)) {
            // 입력 스트림, 출력 스트림 만들기
            PrintWriter writer = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            // 서버로 아무 메세지 송신
            writer.println("Hello, world!");

            // 서버로부터 메세지 수신하고 출력
            String line = reader.readLine();
            System.out.println(line);
        }
    }
}
