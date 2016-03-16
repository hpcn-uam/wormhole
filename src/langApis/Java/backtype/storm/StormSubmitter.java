package backtype.storm;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

import java.util.Map;
import java.io.PrintWriter;

public class StormSubmitter
{
	public StormSubmitter()
	{

	}

	public static void submitTopologyWithProgressBar(String name, Map stormConf, StormTopology topology) throws Exception
	{
		String configFileName = "/tmp/stormEinstein";
		String listenIp = "150.244.58.114";
		int listenPort = 5000;
		boolean autoDeployWorms = true;

		PrintWriter writer = new PrintWriter(configFileName, "UTF-8");

		for (int i = 0; i < topology.getWHnum(); i++) {
			writer.println(topology.getWHconfig(i));
			writer.println("	" + topology.getWHdesc(i));
		}

		writer.println("");
		writer.close();

		Einstein eins = new Einstein(configFileName, listenIp, listenPort, autoDeployWorms);
	}
}