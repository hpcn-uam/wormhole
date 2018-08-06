package wh.tests.throughput;

import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.Producer;
import org.apache.kafka.clients.producer.ProducerRecord;

import java.util.Arrays;
import java.util.Properties;
import java.util.regex.Pattern;

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
        config.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
        config.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");

        Producer<String, String> producer = new KafkaProducer<String, String>(config);

        String str = randomString(message_size);

        for (int i = 0; i != message_count; i++) {
            ProducerRecord<String, String> data = new ProducerRecord<String, String>("thr", str);
            producer.send(data);
        }

        producer.close();
    }

}