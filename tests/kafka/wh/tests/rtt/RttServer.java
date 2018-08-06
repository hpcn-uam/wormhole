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

public class RttServer {
    public static void main(final String[] args) throws Exception {
        if (args.length != 4) {
            System.out.println("Params: <KafkaBroker> <message-size> <batch size> <batches>");
        }
        final String bootstrapServers = args[0];

        final int message_size = Integer.parseInt(args[1]);
        final int batch_size = Integer.parseInt(args[2]);
        final int batches = Integer.parseInt(args[3]);

        final Properties config = new Properties();

        config.put("bootstrap.servers", bootstrapServers);
        config.put("acks", "1");
        config.put("retries", 0);
        config.put("group.id", "servers");
        config.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
        config.put("value.serializer", "org.apache.kafka.common.serialization.ByteArraySerializer");
        config.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
        config.put("value.deserializer", "org.apache.kafka.common.serialization.ByteArrayDeserializer");

        Consumer<String, byte[]> consumer = new KafkaConsumer<>(config);
        consumer.subscribe(Arrays.asList("latin"));

        Producer<String, byte[]> producer = new KafkaProducer<String, byte[]>(config);

        for (int i = 0; i < batches * batch_size;) {
            ConsumerRecords<String, byte[]> consumerRecords = consumer.poll(0);
            for (ConsumerRecord<String, byte[]> record : consumerRecords) {
                i++;
                ProducerRecord<String, byte[]> data = new ProducerRecord<String, byte[]>("latout", record.value());
                producer.send(data);
            }
        }

        producer.close();
        consumer.close();
    }

}