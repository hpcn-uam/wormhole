package wh.tests.throughput;

import org.apache.kafka.clients.consumer.Consumer;
import org.apache.kafka.clients.consumer.ConsumerConfig;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.KafkaConsumer;

import java.util.Arrays;
import java.util.Properties;
import java.util.regex.Pattern;

import java.util.Random;
import java.security.SecureRandom;

public class ThroughputServer {

    public static void main(final String[] args) throws Exception {
        if (args.length != 3) {
            System.out.println("Params: <KafkaBroker> <message-size> <message-count>");
        }
        final String bootstrapServers = args[0];

        final int message_size = Integer.parseInt(args[1]);
        final int message_count = Integer.parseInt(args[2]);

        final Properties config = new Properties();

        config.put("bootstrap.servers", bootstrapServers);
        config.put("acks", "1");
        config.put("retries", 0);
        config.put("group.id", "servers");
        config.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
        config.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");

        Consumer<String, String> consumer = new KafkaConsumer<>(config);
        consumer.subscribe(Arrays.asList("thr"));
        long start = 0;
        for (int i = 0; i < message_count;) {
            ConsumerRecords<String, String> consumerRecords = consumer.poll(0);
            if (i == 0 && consumerRecords.count() > 0) {
                start = System.currentTimeMillis() * 1000;
            }
            i += consumerRecords.count();
        }
        long end = System.currentTimeMillis() * 1000;
        long elapsed = end - start;
        consumer.close();

        double throughput = ((double) message_count / (double) elapsed * 1000000);
        double megabits = ((double) throughput * message_size * 8) / 1000000;

        System.out.println("message size: " + message_size + " [B]");
        System.out.println("message count: " + message_count + "");
        System.out.println("mean throughput: " + throughput + " [msg/s]");
        System.out.println("mean throughput: " + megabits + " [Mb/s]");

    }

}