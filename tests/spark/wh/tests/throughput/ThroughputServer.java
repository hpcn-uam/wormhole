package wh.tests.throughput;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.StorageLevels;
import org.apache.spark.api.java.function.*;
import org.apache.spark.streaming.Durations;
import org.apache.spark.streaming.api.java.JavaDStream;
import org.apache.spark.streaming.api.java.JavaPairDStream;
import org.apache.spark.streaming.api.java.JavaReceiverInputDStream;
import org.apache.spark.streaming.api.java.JavaStreamingContext;

import java.util.Iterator;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Properties;
import java.util.regex.Pattern;

import java.util.Random;
import java.security.SecureRandom;

public class ThroughputServer {

    public static long start;
    public static int i = 0;

    public static void main(final String[] args) throws Exception {
        if (args.length != 4) {
            System.out.println("Params: <hostname> <port> <message-size> <message-count>");
        }
        final String hostname = args[0];
        final int port = Integer.parseInt(args[1]);

        final int message_size = Integer.parseInt(args[2]);
        final int message_count = Integer.parseInt(args[3]);

        SparkConf sparkConf = new SparkConf().setAppName("WhThTest");
        JavaStreamingContext ssc = new JavaStreamingContext(sparkConf, Durations.milliseconds(10));

        JavaReceiverInputDStream<String> messages = ssc.socketTextStream(args[0], Integer.parseInt(args[1]),
                StorageLevels.MEMORY_ONLY);

        JavaDStream<String> words = messages.flatMap(new FlatMapFunction<String, String>() {
            public Iterator<String> call(String s) {
                i++;
                if (i == 1) {
                    start = System.currentTimeMillis() * 1000;
                } else if (i >= message_count) {
                    long end = System.currentTimeMillis() * 1000;
                    long elapsed = end - start;

                    double throughput = ((double) message_count / (double) elapsed * 1000000);
                    double megabits = ((double) throughput * message_size * 8) / 1000000;

                    System.out.println("message size: " + message_size + " [B]");
                    System.out.println("message count: " + message_count + "");
                    System.out.println("mean throughput: " + throughput + " [msg/s]");
                    System.out.println("mean throughput: " + megabits + " [Mb/s]");

                    // ssc.stop(true, true);
                    // System.exit(0);
                    i = 0;
                }
                return new ArrayList<String>().iterator();
            }
        });

        words.print();

        ssc.start();
        while (true)
            ;
        //ssc.stop();

        //ssc.awaitTermination();

    }
}
