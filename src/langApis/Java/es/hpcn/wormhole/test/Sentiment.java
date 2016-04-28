package es.hpcn.wormhole.test;

import es.hpcn.wormhole.Worm;

import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;

import edu.stanford.nlp.sentiment.SentimentPipeline;

public class Sentiment
{
	public static void main(String[] args) throws Exception
	{
		Worm worm = new Worm();

		if (worm.getId() == 1) {
			//sender
			BufferedReader reader = IOUtils.readerFromStdin("utf-8");

			for (String line; (line = reader.readLine()) != null;) {
				worm.send(line);
			}

			worm.flushIO();

		} else {
			SentimentPipeline.sentimentStart();

			while (true) {
				String data = worm.recv();

				if (data == null) {
					break;
				}

				SentimentPipeline.sentimentProcess(data);
			}

		}

		worm.halt();
	}
}