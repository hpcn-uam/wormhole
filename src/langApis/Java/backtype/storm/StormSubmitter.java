package backtype.storm;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

import backtype.storm.generated.StormTopology;

import java.util.Map;
import java.io.PrintWriter;

public class StormSubmitter
{
	public StormSubmitter()
	{

	}

	private static int checkWH(StormTopology topology)
	{
		return 1;
	}

	public static void submitTopologyWithProgressBar(String name, Map stormConf, StormTopology topology) throws Exception
	{
		if (checkWH(StormTopology topology) != 0
			return;

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

		Einstein eins = new Einstein(configFileName, listenIp, listenPort, autoDeployWorms);
	}
}