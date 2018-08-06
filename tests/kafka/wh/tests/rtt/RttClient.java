package wh.tests.rtt;

import org.apache.kafka.clients.consumer.Consumer;
import org.apache.kafka.clients.consumer.ConsumerConfig;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.apache.kafka.clients.consumer.KafkaConsumer;
import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.Producer;
import org.apache.kafka.clients.producer.ProducerRecord;

import java.util.Arrays;
import java.util.Properties;
import java.util.regex.Pattern;

import java.util.Random;
import java.security.SecureRandom;

public class RttClient {

    static int batch_size;
    static int batches;
    static Properties config;
    static long measurements[];

    public static byte[] longToBytes(long l) {
        byte[] result = new byte[8];
        for (int i = 7; i >= 0; i--) {
            result[i] = (byte) (l & 0xFF);
            l >>= 8;
        }
        return result;
    }

    public static long bytesToLong(byte[] b) {
        long result = 0;
        for (int i = 0; i < 8; i++) {
            result <<= 8;
            result |= (b[i] & 0xFF);
        }
        return result;
    }

    public static class RttClientThr extends Thread {

        public void run() {
            Consumer<String, byte[]> consumer = new KafkaConsumer<>(config);
            consumer.subscribe(Arrays.asList("latout"));

            for (int i = 0; i < batches * batch_size;) {

                ConsumerRecords<String, byte[]> consumerRecords = consumer.poll(0);

                for (ConsumerRecord<String, byte[]> record : consumerRecords) {
                    i++;
                    if (i % batch_size == 0) {
                        measurements[i] = System.nanoTime() - bytesToLong(record.value());
                    }
                }
            }

            consumer.close();
        }
    }

    public static void main(final String[] args) throws Exception {
        if (args.length != 5) {
            System.out.println("Params: <KafkaBroker> <message-size> <batch size> <batches> <sleep>");
        }
        final String bootstrapServers = args[0];

        final int message_size = Integer.parseInt(args[1]);
        batch_size = Integer.parseInt(args[2]);
        batches = Integer.parseInt(args[3]);
        final int sleep = Integer.parseInt(args[4]);

        measurements = new long[batches];

        config = new Properties();

        config.put("bootstrap.servers", bootstrapServers);
        config.put("acks", "1");
        config.put("retries", 0);
        config.put("group.id", "servers");
        config.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
        config.put("value.serializer", "org.apache.kafka.common.serialization.ByteArraySerializer");
        config.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
        config.put("value.deserializer", "org.apache.kafka.common.serialization.ByteArrayDeserializer");

        Producer<String, byte[]> producer = new KafkaProducer<String, byte[]>(config);

        Thread th = new RttClientThr();
        th.start();
        byte[] msg = new byte[message_size];

        for (int i = 0; i != batches; i++) {
            for (int j = 0; j != batch_size; j++) {
                if (j == 0) {
                    long ctime = System.nanoTime();
                    byte[] ctimeb = longToBytes(ctime);
                    for (int k = 0; k < 8; ++k) {
                        msg[k] = ctimeb[k];
                    }
                }

                ProducerRecord<String, byte[]> data = new ProducerRecord<String, byte[]>("latin", msg);
                producer.send(data);
            }
            if (sleep > 0) {
                Thread.sleep(sleep);
            }
        }

        th.join();

        for (int i = 0; i < batches; i++) {
            System.out.println((measurements[i]) + " ns");
        }

        producer.close();
    }

}