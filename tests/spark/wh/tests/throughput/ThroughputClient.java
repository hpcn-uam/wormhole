package wh.tests.throughput;

import scala.Tuple2;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.StorageLevels;
import org.apache.spark.streaming.Durations;
import org.apache.spark.streaming.api.java.JavaDStream;
import org.apache.spark.streaming.api.java.JavaPairDStream;
import org.apache.spark.streaming.api.java.JavaReceiverInputDStream;
import org.apache.spark.streaming.api.java.JavaStreamingContext;

import java.util.Arrays;
import java.util.Properties;
import java.util.regex.Pattern;

import java.net.ServerSocket;
import java.net.Socket;
import java.io.PrintStream;

import java.util.Random;
import java.security.SecureRandom;

public class ThroughputClient {

    private static String randomString(long len) {
        final String AB = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        SecureRandom rnd = new SecureRandom();

        StringBuilder sb = new StringBuilder((int) len);

        for (long i = 0; i < len; i++) {
            sb.append(AB.charAt(rnd.nextInt(AB.length())));
        }

        return sb.toString();
    }

    public static void main(final String[] args) throws Exception {
        if (args.length != 4) {
            System.out.println("Params: <hostname> <port> <message-size> <message-count>");
        }
        final String hostname = args[0];
        final int port = Integer.parseInt(args[1]);

        final int message_size = Integer.parseInt(args[2]);
        final int message_count = Integer.parseInt(args[3]);

        SparkConf sparkConf = new SparkConf().setAppName("WhThTest");

        ServerSocket ss = new ServerSocket(port);
        Socket s = ss.accept();
        PrintStream pw = new PrintStream(s.getOutputStream());

        final String str = randomString(message_size);

        // for (int i = 0; i < message_count; i++) {
        while (true) {
            pw.println(str);
        }

    }

}