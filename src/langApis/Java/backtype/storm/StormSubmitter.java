package backtype.storm;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

import backtype.storm.generated.StormTopology;
import backtype.storm.topology.IRichBolt;
import backtype.storm.topology.IRichSpout;
import backtype.storm.task.OutputCollector;
import backtype.storm.spout.SpoutOutputCollector;
import backtype.storm.tuple.Tuple;

import java.util.Map;
import java.io.PrintWriter;
import java.util.ArrayList;

public class StormSubmitter
{
	public StormSubmitter()
	{

	}

	private static boolean  isEinsteinNode	= false;
	private static String   mainClass 		= null;
	private static String[] mainClassArgs 	= null;

	public static void setEinsteinNode(String mainclassPrm, String[] mainClassArgsPrm)
	{
		isEinsteinNode = true;
		mainClass = mainclassPrm;
		mainClassArgs = mainClassArgsPrm;
	}

	public static void submitTopologyWithProgressBar(String name, Map stormConf, StormTopology topology) throws Exception
	{
		if (isEinsteinNode) {   //Einstein things
			String configFileName = "/tmp/stormEinstein.conf";
			String listenIp = "150.244.58.114";
			int listenPort = 5000;
			boolean autoDeployWorms = true;

			PrintWriter writer = new PrintWriter(configFileName, "UTF-8");

			for (int i = 1; i <= topology.getWHnum(); i++) {
				writer.println(topology.getWHconfig(i));
				writer.println("	" + topology.getWHdesc(i));
			}

			writer.println("");
			writer.close();

			String[]  params = new String[mainClassArgs.length + 1];
			params[0] = mainClass;

			for (int i = 0; i < mainClassArgs.length; i++) {
				params[i + 1] = mainClassArgs[i];
			}

			Einstein eins = new Einstein(configFileName, listenIp, listenPort, autoDeployWorms, params);

		} else { //Worm things

			ArrayList<IRichSpout> spouts = topology.WHgetSpouts();
			ArrayList<IRichBolt>  bolts = topology.WHgetBolts();

			Worm worm = new Worm();
			int id = worm.getId();

			try {

				if (id <= spouts.size()) { // Is a spout!
					System.out.println("[" + id + "/" + spouts.size() + "] Im a spout! :)");
					IRichSpout me = spouts.get(id - 1);
					SpoutOutputCollector collector = new SpoutOutputCollector();
					me.open(null, null, collector);

					while (true) {
						me.nextTuple();
					}

				} else { // is a volt
					System.out.println("[" + id + "/" + bolts.size() + "] Im a bolt! :)");
					IRichBolt me = bolts.get((id - spouts.size()) - 1);
					OutputCollector collector = new OutputCollector();
					me.prepare(null, null, collector);

					Tuple t = new Tuple();
					String msg = new String();

					while (true) {
						worm.recv(msg);
						t.setWHString(msg);
						me.execute(t);
					}
				}

			} catch (Throwable err) {
				System.out.println(err.toString());
				System.out.println(err.getMessage());
				err.printStackTrace();

			} finally {
				worm.halt();
			}

			return;

		}
	}
}